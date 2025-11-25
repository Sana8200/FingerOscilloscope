/**
 * main.c - Real-Time Oscilloscope using DE10-Lite RISC-V and AD7705 ADC
 * 
 * Features:
 * - Real-time waveform display on VGA (320x240)
 * - Professional HP-style oscilloscope UI
 * - Voltage measurements (current, Vpp, min, max)
 * - Scrolling waveform display
 */

#include <stdint.h>
#include <stdbool.h>
#include "hardware.h"
#include "spi_driver.h"
#include "ad7705_driver.h"
#include "vga_driver.h"
#include "timer.h"
#include "dtekv-lib.h"
#include "delay.h"

// ============================================================================
// Configuration
// ============================================================================
#define VREF                3.3f    // Reference voltage
#define VOLTS_PER_DIV       0.5f    // Voltage scale
#define TIME_PER_DIV_MS     10.0f   // Time scale

// ============================================================================
// Global State
// ============================================================================

// Waveform buffer
static uint16_t waveform_buffer[SCREEN_WIDTH];

// Display position
static int current_x = 0;
static int grat_left, grat_right, grat_top, grat_bottom;

// Statistics
static uint16_t adc_min = 65535;
static uint16_t adc_max = 0;

// ============================================================================
// Helper Functions
// ============================================================================

static float adc_to_voltage(uint16_t adc_value) {
    return (float)adc_value * VREF / 65535.0f;
}

static void reset_statistics(void) {
    adc_min = 65535;
    adc_max = 0;
}

static void update_statistics(uint16_t adc_value) {
    if (adc_value < adc_min) adc_min = adc_value;
    if (adc_value > adc_max) adc_max = adc_value;
}

// Scales 16-bit ADC value (0-65535) to Screen Y (239-0)
// 0V input -> Bottom of screen (Y=239), Max input -> Top of screen (Y=0)
uint8_t map_adc_to_screen_y(uint16_t adc_value) {
    // 65535 / 240 is approx 273. 
    // We divide by 274 to stay safely within 0-239 range.
    uint16_t scaled = adc_value / 274;
    
    if (scaled > 239) scaled = 239;
    
    // 0 is at the bottom
    return (uint8_t)(SCREEN_HEIGHT - 1 - scaled);
}

// ============================================================================
// Interrupt Handler
// ============================================================================
void handle_interrupt(unsigned cause) {
    // Timer interrupt if needed
}

// ============================================================================
// Main Program
// ============================================================================

int main(void) {
    // Initialize hardware
    display_string("\n=== DE10-Lite Oscilloscope ===\n\n");
    
    // Initialize peripherals
    display_string("Init timer...\n");
    timer_init(200);  // 200 Hz sample rate
    
    display_string("Init SPI...\n");
    spi_init();
    delay_ms(100);
    
    display_string("Init AD7705...\n");
    ad7705_init(CHN_AIN1);
    delay_ms(100);
    
    // Initialize VGA with oscilloscope display
    display_string("Init VGA...\n");
    vga_scope_init();
    
    // Get graticule bounds
    vga_get_waveform_bounds(&grat_top, &grat_bottom, &grat_left, &grat_right);
    current_x = grat_left;
    
    // Initialize buffer
    for (int i = 0; i < SCREEN_WIDTH; i++) {
        waveform_buffer[i] = 32768;  // Mid-scale
    }
    
    display_string("Ready!\n\n");
    
    // ========================================================================
    // Main Loop
    // ========================================================================
    
    uint32_t frame = 0;
    
    while (1) {
        // Read ADC
        uint16_t adc_raw = ad7705_read_data(CHN_AIN1);
        
        // Update stats
        update_statistics(adc_raw);
        
        // LED feedback (upper 8 bits)
        set_leds(adc_raw >> 8);
        
        // === Draw waveform ===
        
        // Erase old column and restore grid
        vga_erase_column(current_x);
        
        // Get previous sample
        int prev_x = (current_x == grat_left) ? grat_right : current_x - 1;
        uint16_t prev_adc = waveform_buffer[prev_x];
        
        // Draw segment
        if (current_x > grat_left) {
            vga_draw_waveform_segment(prev_x, prev_adc, current_x, adc_raw, COLOR_WAVEFORM);
        } else {
            // First point of new sweep
            int y = vga_adc_to_screen_y(adc_raw);
            vga_draw_pixel(current_x, y, COLOR_WAVEFORM);
        }
        
        // Store sample
        waveform_buffer[current_x] = adc_raw;
        
        // Advance position
        current_x++;
        if (current_x > grat_right) {
            current_x = grat_left;
            
            // Update header once per sweep
            float v_current = adc_to_voltage(adc_raw);
            float v_max = adc_to_voltage(adc_max);
            float v_min = adc_to_voltage(adc_min);
            
            vga_scope_update_info(1, v_current, VOLTS_PER_DIV, 
                                  TIME_PER_DIV_MS, v_max, v_min);
            
            // Debug output every 10 frames
            frame++;
            if (frame % 10 == 0) {
                print("Frame ");
                print_dec(frame);
                print(" ADC:");
                print_dec(adc_raw);
                print(" Vpp:");
                print_dec(adc_max - adc_min);
                print("\n");
            }
            
            reset_statistics();
        }
        
        // === Handle user input ===
        int sw = get_sw();
        if (sw & 0x01) {
            // Switch 0: Change time scale (placeholder)
        }
        if (sw & 0x02) {
            // Switch 1: Change voltage scale (placeholder)
        }
    }
    
    return 0;
}