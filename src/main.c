/**
 * main.c - Real-Time Oscilloscope using DE10-Lite RISC-V and AD7705 ADC
 * 
 * Features:
 * - Real-time waveform display on VGA (320x240)
 * - Professional oscilloscope UI with header/footer
 * - Voltage measurements (current, Vpp, min, max)
 * - Scrolling waveform display
 * 
 * Hardware:
 * - DE10-Lite FPGA board with RISC-V soft processor
 * - AD7705 16-bit Sigma-Delta ADC
 * - VGA output (320x240)
 */

#include <stdint.h>
#include <stdbool.h>
#include "lib.h"
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
#define SAMPLE_RATE_HZ      200     // ADC sample rate (match AD7705 update rate)
#define VREF                3.3f    // Reference voltage

// Display settings
#define VOLTS_PER_DIV       0.5f    // Voltage scale: 0.5V per division
#define TIME_PER_DIV_MS     10.0f   // Time scale: 10ms per division

// ============================================================================
// Global State
// ============================================================================

// Waveform buffer - stores ADC values for each screen column
static uint16_t waveform_buffer[SCREEN_WIDTH];

// Current drawing position
static int current_x = 0;

// Statistics tracking
static uint16_t adc_min = 65535;
static uint16_t adc_max = 0;
static uint32_t adc_sum = 0;
static uint32_t sample_count = 0;

// Trigger detection
static uint16_t trigger_level = 32768;  // Mid-scale default
static uint16_t prev_sample = 32768;
static bool triggered = false;

// Zero-crossing detection for frequency measurement
static int zero_cross_count = 0;
static uint32_t samples_since_reset = 0;
static float measured_frequency = 0.0f;

// ============================================================================
// Interrupt Handler (if using timer interrupts)
// ============================================================================
void handle_interrupt(unsigned cause) {
    // Timer interrupt handling if needed
}

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * Update min/max/average statistics
 */
static void update_statistics(uint16_t adc_value) {
    if (adc_value < adc_min) adc_min = adc_value;
    if (adc_value > adc_max) adc_max = adc_value;
    adc_sum += adc_value;
    sample_count++;
    
    // Detect zero crossings (for frequency measurement)
    // Using mid-scale as "zero" reference
    samples_since_reset++;
    if ((prev_sample < trigger_level && adc_value >= trigger_level) ||
        (prev_sample > trigger_level && adc_value <= trigger_level)) {
        zero_cross_count++;
        
        // Calculate frequency every 10 zero crossings
        if (zero_cross_count >= 10) {
            // frequency = (crossings / 2) / time
            // time = samples / sample_rate
            float time_seconds = (float)samples_since_reset / SAMPLE_RATE_HZ;
            measured_frequency = (zero_cross_count / 2.0f) / time_seconds;
            
            // Reset counters
            zero_cross_count = 0;
            samples_since_reset = 0;
        }
    }
    prev_sample = adc_value;
}

/**
 * Reset statistics for new measurement period
 */
static void reset_statistics(void) {
    adc_min = 65535;
    adc_max = 0;
    adc_sum = 0;
    sample_count = 0;
}

/**
 * Convert ADC value to voltage
 */
static float adc_to_voltage(uint16_t adc_value) {
    return (float)adc_value * VREF / 65535.0f;
}

/**
 * Update the display header with current measurements
 */
static void update_display_info(uint16_t current_adc) {
    float v_current = adc_to_voltage(current_adc);
    float v_max = adc_to_voltage(adc_max);
    float v_min = adc_to_voltage(adc_min);
    
    vga_scope_update_info(
        1,                  // Channel 1
        v_current,          // Current voltage
        VOLTS_PER_DIV,      // V/div setting
        TIME_PER_DIV_MS,    // Time/div setting
        v_max,              // Max voltage
        v_min               // Min voltage
    );
    
    vga_scope_set_frequency(measured_frequency);
}

// ============================================================================
// Main Program
// ============================================================================

int main(void) {
    // ========================================================================
    // Initialization
    // ========================================================================
    
    display_string("\n========================================\n");
    display_string("  DE10-Lite Oscilloscope v1.0\n");
    display_string("========================================\n\n");
    
    // Initialize timer for sample timing
    display_string("Initializing timer...\n");
    timer_init(SAMPLE_RATE_HZ);
    
    // Initialize SPI
    display_string("Initializing SPI...\n");
    spi_init();
    delay_ms(100);
    
    // Initialize ADC
    display_string("Initializing AD7705...\n");
    ad7705_init(CHN_AIN1);
    delay_ms(100);
    
    // Initialize VGA display with oscilloscope UI
    display_string("Initializing VGA display...\n");
    vga_scope_init();
    
    // Set trigger level to mid-scale
    vga_scope_set_trigger(trigger_level);
    
    // Initialize waveform buffer to mid-scale
    for (int i = 0; i < SCREEN_WIDTH; i++) {
        waveform_buffer[i] = 32768;
    }
    
    display_string("\nOscilloscope ready!\n");
    display_string("========================================\n\n");
    
    // ========================================================================
    // Main Loop - Real-time waveform acquisition and display
    // ========================================================================
    
    uint32_t frame_counter = 0;
    
    while (1) {
        // Read ADC (blocking until data ready)
        uint16_t adc_raw = ad7705_read_data(CHN_AIN1);
        
        // Update statistics
        update_statistics(adc_raw);
        
        // Show current value on LEDs (upper 8 bits for visual feedback)
        set_leds(adc_raw >> 8);
        
        // === Waveform Display Update ===
        
        // Erase the old data at current X position and restore grid
        vga_erase_column(current_x);
        
        // Get previous sample for line drawing
        int prev_x = (current_x == 0) ? SCREEN_WIDTH - 1 : current_x - 1;
        uint16_t prev_adc = waveform_buffer[prev_x];
        
        // Draw waveform segment connecting to previous point
        if (current_x > 0) {
            vga_draw_waveform_segment(prev_x, prev_adc, current_x, adc_raw, COLOR_WAVEFORM);
        } else {
            // At start of screen, just draw a single point
            int y = vga_adc_to_screen_y(adc_raw);
            vga_draw_pixel(current_x, y, COLOR_WAVEFORM);
        }
        
        // Store current sample in buffer
        waveform_buffer[current_x] = adc_raw;
        
        // Advance X position and wrap around
        current_x++;
        if (current_x >= SCREEN_WIDTH) {
            current_x = 0;
            
            // Update header info once per screen sweep
            update_display_info(adc_raw);
            
            // Print debug info to UART
            frame_counter++;
            if (frame_counter % 10 == 0) {
                print("Frame: ");
                print_dec(frame_counter);
                print("  ADC: ");
                print_dec(adc_raw);
                print("  Vpp: ");
                print_dec(adc_max - adc_min);
                print("\n");
            }
            
            // Reset statistics for next sweep
            reset_statistics();
        }
        
        // === Handle User Input ===
        
        // Read switches for trigger level adjustment
        int sw = get_sw();
        if (sw != 0) {
            // Use switches to set trigger level (scaled to 16-bit range)
            trigger_level = (sw & 0x3FF) << 6;  // 10-bit switch -> 16-bit
            vga_scope_set_trigger(trigger_level);
        }
        
        // Button press could be used for run/stop
        // if (get_btn()) { ... }
    }
    
    return 0;
}