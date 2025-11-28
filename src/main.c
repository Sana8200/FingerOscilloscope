/**
 * main.c - Real-Time Oscilloscope
 * 
 * DE10-Lite RISC-V + AD7705 16-bit ADC + VGA Display
 * 
 * Features:
 * - Real-time scrolling waveform display
 * - Tektronix-style oscilloscope UI
 * - Voltage measurements (Vpp, min, max)
 * - Professional grid display
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
#include "lib.h"


#define VOLTS_PER_DIV   0.5f        // Default voltage scale
#define TIME_PER_DIV_US 400.0f      // Default time scale (µs)

// =============================================================================
// Waveform State
// =============================================================================

static uint16_t waveform_buffer[SCREEN_WIDTH];
static int current_x;
static int grat_left, grat_right;

// Statistics
static uint16_t adc_min = 65535;
static uint16_t adc_max = 0;



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

static void reset_stats(void) {
    adc_min = 65535;
    adc_max = 0;
}

static void update_stats(uint16_t value) {
    if (value < adc_min) adc_min = value;
    if (value > adc_max) adc_max = value;
}

// =============================================================================
// Interrupt Handler (if needed)
// =============================================================================

void handle_interrupt(unsigned cause) {
    // Timer interrupt handling if needed
}

// =============================================================================
// Main Program
// =============================================================================

int main(void) {
    display_string("\n");
    display_string("================================DE10-Lite RISC-V Oscilloscope, AD7705 16-bit ADC================================\n\n ");

    
    display_string("Initializing...\n");
    // Timer (200 Hz sample rate)
    display_string("  Timer...");
    timer_init(200);
    display_string(" OK\n");
    
    // SPI interface
    display_string("  SPI...");
    spi_init();
    delay_ms(50);
    display_string(" OK\n");
    
    // AD7705 ADC
    display_string("  AD7705 ADC...\n");
    ad7705_init(CHN_AIN1);
    delay_ms(100);
    
    // VGA Display
    display_string("  VGA display...");
    scope_init();
    display_string(" OK\n");
    
    // ----- Configure Scope State -----
    g_scope.ch1_vdiv = VOLTS_PER_DIV;
    g_scope.time_div_ms = TIME_PER_DIV_US / 1000.0f;
    g_scope.time_is_us = true;
    g_scope.ch1_enabled = true;
    g_scope.ch2_enabled = false;
    g_scope.running = true;
    g_scope.trig_level_mv = 80.0f;
    g_scope.ch1_y_offset = 0;  // Center channel 1
    
    // Update display with initial settings
    scope_redraw_all();
    
    // ----- Get Drawing Bounds -----
    grat_left = scope_get_left();
    grat_right = scope_get_right();
    current_x = grat_left;
    
    // Initialize waveform buffer to mid-scale
    for (int i = 0; i < SCREEN_WIDTH; i++) {
        waveform_buffer[i] = 32768;
    }
    
    display_string("\nReady! Starting acquisition...\n\n");
    
    // ==========================================================================
    // Main Loop - Continuous Acquisition
    // ==========================================================================
    
    uint32_t frame_count = 0;
    
    while (1) {
        // ----- Read ADC Sample -----
        uint16_t adc_raw = ad7705_read_data(CHN_AIN1);
        
        // Update statistics
        update_stats(adc_raw);
        
        // LED feedback (show upper bits of ADC value)
        set_leds(adc_raw >> 8);
        
        // ----- Draw Waveform -----
        
        // Erase old data at current position and restore grid
        scope_erase_column(current_x);
        
        // Get previous sample for line drawing
        int prev_x = (current_x == grat_left) ? grat_right : current_x - 1;
        uint16_t prev_adc = waveform_buffer[prev_x];
        
        // Draw line segment connecting samples
        if (current_x > grat_left) {
            scope_draw_segment(prev_x, prev_adc, current_x, adc_raw, 1);
        } else {
            // First point of sweep - just draw a dot
            scope_draw_point(current_x, adc_raw, 1);
        }
        
        // Store sample in buffer
        waveform_buffer[current_x] = adc_raw;
        
        // ----- Advance Position -----
        current_x++;
        
        if (current_x > grat_right) {
            // End of sweep - wrap around
            current_x = grat_left;
            
            // Update measurements once per sweep
            float v_max = map_adc_to_screen_y(adc_max);
            float v_min = map_adc_to_screen_y(adc_min);
            float v_pp = v_max - v_min;
            
            g_scope.ch1_vpp = v_pp;
            g_scope.triggered = true;
            
            // Update display info bar
            scope_draw_info_bar();
            scope_draw_status_bar();
            
            // Debug output every 10 frames
            frame_count++;
            if (frame_count % 10 == 0) {
                print("Frame ");
                print_dec(frame_count);
                print("  ADC: ");
                print_dec(adc_raw);
                print("  Vpp: ");
                print_dec((uint32_t)(v_pp * 1000));  // mV
                print(" mV\n");
            }
            
            // Reset statistics for next sweep
            reset_stats();
        }
        
        // ----- Handle User Input -----
        int switches = get_sw();
        
        // Switch 0: Toggle CH2
        static int prev_sw0 = 0;
        int sw0 = switches & 0x01;
        if (sw0 && !prev_sw0) {
            g_scope.ch2_enabled = !g_scope.ch2_enabled;
            scope_draw_info_bar();
            scope_draw_ground_markers();
        }
        prev_sw0 = sw0;
        
        // Switch 1: Change voltage scale
        static int prev_sw1 = 0;
        int sw1 = (switches >> 1) & 0x01;
        if (sw1 && !prev_sw1) {
            // Cycle through: 0.5V, 1.0V, 2.0V
            if (g_scope.ch1_vdiv < 0.7f) {
                g_scope.ch1_vdiv = 1.0f;
            } else if (g_scope.ch1_vdiv < 1.5f) {
                g_scope.ch1_vdiv = 2.0f;
            } else {
                g_scope.ch1_vdiv = 0.5f;
            }
            scope_draw_info_bar();
        }
        prev_sw1 = sw1;
        
        // Switch 9: Run/Stop toggle
        static int prev_sw9 = 0;
        int sw9 = (switches >> 9) & 0x01;
        if (sw9 && !prev_sw9) {
            g_scope.running = !g_scope.running;
            scope_draw_status_bar();
        }
        prev_sw9 = sw9;
        
        // If stopped, skip acquisition
        if (!g_scope.running) {
            delay_ms(10);
        }
    }
    
    return 0;
}