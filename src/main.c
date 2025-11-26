/**
 * main.c - DE10-Lite Oscilloscope with Full I/O Control
 * 
 * Uses:
 * - VGA display (320x240) for waveform
 * - AD7705 16-bit ADC for input
 * - LEDs for signal level indication
 * - Switches for V/div and time/div control
 * - 7-segment displays for voltage readout
 * - Push button for Run/Stop toggle
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
#include "vga_font.h"


#define VREF            3.3f

// V/div settings (selected by switches 0-2)
static const float VDIV_TABLE[] = {
    0.1f,   // 100mV/div
    0.2f,   // 200mV/div
    0.5f,   // 500mV/div
    1.0f,   // 1V/div
    2.0f,   // 2V/div
    3.3f,   // Full scale
    0.5f,   // default
    0.5f    // default
};

// Time/div settings (selected by switches 3-5)
static const float TDIV_TABLE[] = {
    1.0f,   // 1ms/div
    2.0f,   // 2ms/div
    5.0f,   // 5ms/div
    10.0f,  // 10ms/div
    20.0f,  // 20ms/div
    50.0f,  // 50ms/div
    100.0f, // 100ms/div
    5.0f    // default
};

// ============================================================================
// Global State
// ============================================================================
static uint16_t waveform_buf[320];
static int draw_x = 0;
static int grat_left, grat_right, grat_top, grat_bottom;

// Statistics for current sweep
static uint16_t adc_min = 65535;
static uint16_t adc_max = 0;
static uint32_t adc_sum = 0;
static uint32_t sample_count = 0;

// Run/Stop state
static int running = 1;
static int prev_btn = 0;

// Current settings
static float current_vdiv = 0.5f;
static float current_tdiv = 5.0f;

// ============================================================================
// Helpers
// ============================================================================

static float adc_to_voltage(uint16_t adc) {
    return (float)adc * VREF / 65535.0f;
}

static void reset_stats(void) {
    adc_min = 65535;
    adc_max = 0;
    adc_sum = 0;
    sample_count = 0;
}

static void update_stats(uint16_t adc) {
    if (adc < adc_min) adc_min = adc;
    if (adc > adc_max) adc_max = adc;
    adc_sum += adc;
    sample_count++;
}



// ============================================================================
// Read Settings from Switches
// ============================================================================

static void read_switch_settings(void) {
    int sw = get_sw();
    
    // Switches 0-2: V/div selection (8 options)
    int vdiv_sel = sw & 0x07;
    current_vdiv = VDIV_TABLE[vdiv_sel];
    
    // Switches 3-5: Time/div selection (8 options)
    int tdiv_sel = (sw >> 3) & 0x07;
    current_tdiv = TDIV_TABLE[tdiv_sel];
    
    // Switch 9: CH2 enable (for future dual channel)
    // int ch2_en = (sw >> 9) & 0x01;
}

// ============================================================================
// Button Handling - Toggle Run/Stop
// ============================================================================

static void handle_button(void) {
    int btn = get_btn();
    
    // Detect rising edge (button just pressed)
    if (btn && !prev_btn) {
        running = !running;
        vga_scope_set_running(running);
        
        // Visual feedback
        if (!running) {
            set_leds(0x3FF);  // All LEDs on when stopped
        }
    }
    
    prev_btn = btn;
}

// ============================================================================
// Interrupt Handler
// ============================================================================

void handle_interrupt(unsigned cause) {
    // Timer interrupt if needed
}



#include "vga_driver.h"
#include "hardware.h"
#include "delay.h"

/*
 * =============================================================================
 *  MINI OSCILLOSCOPE - Main Program
 *  DE10-Lite RISC-V + AD7705 16-bit ADC + VGA Display
 * =============================================================================
 *
 *  This example demonstrates:
 *  1. Initializing the oscilloscope display
 *  2. Drawing a test waveform (sine wave)
 *  3. Real-time scrolling display mode
 *
 *  Connect your AD7705 to the GPIO pins defined in hardware.h
 */

// =============================================================================
// SAMPLE BUFFER
// =============================================================================

#define MAX_SAMPLES 320  // One sample per horizontal pixel

static uint16_t sample_buffer[MAX_SAMPLES];
static int sample_index = 0;

// =============================================================================
// SINE WAVE LOOKUP TABLE (for testing without ADC)
// =============================================================================
// 64-point sine wave, scaled to 16-bit (0-65535, centered at 32768)

static const uint16_t sine_table[64] = {
    32768, 35980, 39160, 42280, 45307, 48214, 50972, 53555,
    55938, 58097, 60013, 61666, 63041, 64124, 64905, 65378,
    65535, 65378, 64905, 64124, 63041, 61666, 60013, 58097,
    55938, 53555, 50972, 48214, 45307, 42280, 39160, 35980,
    32768, 29556, 26376, 23256, 20229, 17322, 14564, 11981,
     9598,  7439,  5523,  3870,  2495,  1412,   631,   158,
        0,   158,   631,  1412,  2495,  3870,  5523,  7439,
     9598, 11981, 14564, 17322, 20229, 23256, 26376, 29556
};

// =============================================================================
// DISPLAY UPDATE FUNCTIONS
// =============================================================================

// Update the voltage reading display in the header
void update_voltage_display(uint16_t adc_value) {
    // Convert 16-bit ADC value to millivolts
    // Assuming 2.5V reference: 2500mV / 65535 = 0.0381 mV per count
    // For display, we'll show it as a rough voltage
    
    int millivolts = (adc_value * 2500) / 65535;
    int volts = millivolts / 1000;
    int mv_remainder = millivolts % 1000;
    
    // Clear the voltage display area
    vga_draw_filled_box(260, 16, 60, 10, COLOR_HEADER_BG);
    
    // Draw voltage value (simplified: just show millivolts)
    vga_draw_number(260, 16, millivolts, COLOR_YELLOW);
    vga_draw_string(296, 16, "MV", COLOR_YELLOW);
}

// Update frequency display in status bar
void update_frequency_display(int frequency_hz) {
    // Clear the frequency area
    vga_draw_filled_box(210, STATUS_BAR_TOP + 6, 80, 10, COLOR_HEADER_BG);
    
    vga_draw_string(210, STATUS_BAR_TOP + 6, "F:", COLOR_TEXT_DIM);
    vga_draw_number(224, STATUS_BAR_TOP + 6, frequency_hz, COLOR_YELLOW);
    vga_draw_string(270, STATUS_BAR_TOP + 6, "HZ", COLOR_YELLOW);
}

// =============================================================================
// TEST PATTERN GENERATOR
// =============================================================================

void generate_test_sine_wave(void) {
    // Fill the sample buffer with a sine wave
    // Creates ~5 cycles across the screen width
    
    for (int i = 0; i < MAX_SAMPLES; i++) {
        int table_index = (i * 5) % 64;  // 5 cycles
        sample_buffer[i] = sine_table[table_index];
    }
}

void generate_test_square_wave(void) {
    // Generate a square wave for testing
    int period = 64;  // Pixels per cycle
    
    for (int i = 0; i < MAX_SAMPLES; i++) {
        if ((i / (period / 2)) % 2 == 0) {
            sample_buffer[i] = 55000;  // High level
        } else {
            sample_buffer[i] = 10000;  // Low level
        }
    }
}

void generate_test_triangle_wave(void) {
    // Generate a triangle wave for testing
    int period = 80;
    
    for (int i = 0; i < MAX_SAMPLES; i++) {
        int phase = i % period;
        if (phase < period / 2) {
            // Rising edge
            sample_buffer[i] = 5000 + (phase * 55000) / (period / 2);
        } else {
            // Falling edge
            sample_buffer[i] = 60000 - ((phase - period / 2) * 55000) / (period / 2);
        }
    }
}

// =============================================================================
// SCROLLING DISPLAY MODE
// =============================================================================
// This mode draws samples one at a time, scrolling left when full

static int scroll_x = 0;
static uint16_t scroll_buffer[MAX_SAMPLES];

void scroll_add_sample(uint16_t sample) {
    if (scroll_x < MAX_SAMPLES) {
        // Still filling the buffer
        scroll_buffer[scroll_x] = sample;
        scroll_x++;
    } else {
        // Buffer full - shift left and add new sample at the end
        for (int i = 0; i < MAX_SAMPLES - 1; i++) {
            scroll_buffer[i] = scroll_buffer[i + 1];
        }
        scroll_buffer[MAX_SAMPLES - 1] = sample;
    }
}

void scroll_redraw_waveform(void) {
    // Clear waveform area and redraw grid
    scope_clear_waveform_area();
    scope_draw_grid();
    
    // Draw the waveform
    scope_draw_waveform(scroll_buffer, scroll_x, COLOR_TRACE_GREEN);
}

// =============================================================================
// MAIN PROGRAM
// =============================================================================

int main(void) {
    // -------------------------
    // Initialize the display
    // -------------------------
    scope_init_display();
    
    // -------------------------
    // Demo Mode Selection
    // -------------------------
    // Use switches to select which demo to show:
    // SW0 = 0: Sine wave
    // SW0 = 1: Square wave
    // SW1 = 1: Triangle wave
    
    uint32_t switches = get_sw();
    
    if (switches & 0x02) {
        generate_test_triangle_wave();
    } else if (switches & 0x01) {
        generate_test_square_wave();
    } else {
        generate_test_sine_wave();
    }
    
    // -------------------------
    // Draw the test waveform
    // -------------------------
    scope_draw_waveform(sample_buffer, MAX_SAMPLES, COLOR_TRACE_GREEN);
    
    // Draw trigger level at center
    scope_draw_trigger_level(32768);
    
    // Update displays
    update_voltage_display(32768);   // Center voltage
    update_frequency_display(50);    // Placeholder frequency
    
    // -------------------------
    // Main loop
    // -------------------------
    // In a real application, you would:
    // 1. Read samples from AD7705
    // 2. Store in buffer
    // 3. Redraw waveform
    // 4. Update measurements
    
    int frame_count = 0;
    
    while (1) {
        // Check for button press to change waveform
        uint32_t buttons = get_btn();
        
        if (buttons & 0x01) {
            // Button 0: Cycle through test patterns
            static int pattern = 0;
            pattern = (pattern + 1) % 3;
            
            switch (pattern) {
                case 0: generate_test_sine_wave(); break;
                case 1: generate_test_square_wave(); break;
                case 2: generate_test_triangle_wave(); break;
            }
            
            // Redraw
            scope_clear_waveform_area();
            scope_draw_grid();
            scope_draw_waveform(sample_buffer, MAX_SAMPLES, COLOR_TRACE_GREEN);
            scope_draw_trigger_level(32768);
            
            // Simple debounce
            delay_us(200000);  // 200ms delay
        }
        
        if (buttons & 0x02) {
            // Button 1: Demonstrate scrolling mode
            // Reset scroll buffer
            scroll_x = 0;
            
            // Simulate real-time sampling
            for (int i = 0; i < 500; i++) {
                // Generate a sample (in real use, read from AD7705)
                int table_index = (i * 3) % 64;
                uint16_t sample = sine_table[table_index];
                
                scroll_add_sample(sample);
                
                // Redraw every few samples for animation effect
                if (i % 5 == 0) {
                    scroll_redraw_waveform();
                    delay_us(20000);  // 20ms delay for visible animation
                }
            }
            
            // Debounce
            delay_us(200000);
        }
        
        // Update LED with frame counter (shows we're running)
        frame_count++;
        set_leds(frame_count >> 10);
        
        // Small delay to prevent busy loop
        delay_us(1000);
    }
    
    return 0;
}