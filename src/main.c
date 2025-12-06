/**
 * DE10-Lite oscilloscope using AD7705 ADC
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
#include "performance_counter.h"


#define V_MIN           0.0f    // Minimum voltage on display
#define V_MAX           3.3f    // Maximum voltage on display
#define ERASE_WIDTH     3       // Pixels to clear ahead of waveform

#define DEFAULT_SAMPLE_RATE   150     // Samples per second
#define MIN_SAMPLE_RATE  50
#define MAX_SAMPLE_RATE 500


// Number of samples to collect for performance measurement
#define PERF_TEST_SAMPLES  100

// Graph area boundaries
static int graph_left;
static int graph_right;
static int graph_top;
static int graph_bottom;

// Current and previous drawing positions
static int pos_x;   // where we are currently drawing 
static int prev_x;
static int prev_y;

// Statistics, using sweeping for not redrawing the whole screen every time 
static float sweep_min;
static float sweep_max;
static int sweep_count;

static int current_gain = 1;
static int current_sample_rate = DEFAULT_SAMPLE_RATE;
static bool is_paused = false;
static bool is_frozen = false;
static int prev_switches = 0;   
static bool perf_test_running = false;   



/**
 * Run performance test: collect PERF_TEST_SAMPLES and measure performance
 */
void run_performance_test(void) {
    perf_counters_t counters;
    float voltage;
    int y;
    
    perf_test_running = true;
    set_leds(0x155);  
    
    display_string("\n\n========================================  PERFORMANCE TEST STARTING  ========================================\n");
    
    print("Samples to collect: ");
    print_dec(PERF_TEST_SAMPLES);
    print("Sample rate: ");
    print_dec(current_sample_rate);
    print("Gain: ");
    print_dec(current_gain);
    
    // Clear counters before test
    perf_clear_counters();
    
    // === START OF MEASURED CODE ===
    for (int i = 0; i < PERF_TEST_SAMPLES; i++) {
        // Wait for timer tick
        while (!timer_check_tick()) {
            // busy wait
        }

        voltage = ad7705_read_voltage(CHN_AIN1);
        y = vga_voltage_to_y(voltage, V_MIN, V_MAX);
        
        // Draw pixel (simplified - no line drawing for cleaner measurement)
        vga_draw_pixel(graph_left + (i % (graph_right - graph_left)), y, COLOR_ORANGE);
    }
    // === END OF MEASURED CODE ===
    
    // Read counters after test
    perf_read_counters(&counters);
    
    // Print results
    perf_print_counters(&counters);
    perf_print_metrics(&counters);

    display_string("======================================== PERFORMANCE TEST COMPLETE ========================================");
    display_string("Toggle SW0 OFF to return to normal mode\n");
    
    perf_test_running = false;
}


int main(void) {
    display_string("\n");
    display_string("============================================= DE10-Lite Oscilloscope =============================================\n");
    display_string("Controls:");
    display_string("  Button  : Pause/Resume");
    display_string("  SW0     : Freeze (examine waveform)");
    display_string("  SW2     : Gain = 2");
    display_string("  SW3     : Gain = 4");
    display_string("  SW4     : Gain = 8");
    display_string("  SW6     : Run Performance Analysis test");
    display_string("  SW8     : Sample Rate -50");
    display_string("  SW9     : Sample Rate +50\n");

    display_string("Init Timer...");
    timer_init(current_sample_rate);
    
    display_string("Init SPI...");
    spi_init();
    delay_ms(50);
    
    display_string("Init ADC...");
    ad7705_init(CHN_AIN1);
    delay_ms(100);
    
    display_string("Init VGA...");
    vga_init_scope(current_gain, current_sample_rate);
    vga_get_graph_area(&graph_left, &graph_right, &graph_top, &graph_bottom);
    
    // Initialize variables 
    pos_x = graph_left;
    prev_x = graph_left;
    prev_y = (graph_top + graph_bottom) / 2;
    sweep_min = V_MAX;
    sweep_max = V_MIN;
    sweep_count = 0;
    
    display_string("\nReady!\n");
    set_leds(0x002);
    display_7seg_voltage_gain(0.0f, current_gain);
    
    while (1) {
        // ================================================================
        // BUTTON PAUSE 
        // ================================================================
        if (get_btn()) {
            // Wait until button is RELEASED
            while (get_btn()) {
                delay_ms(10);
            }
            delay_ms(50); // Debounce
            is_paused = !is_paused;  // Toggle Pause State

            if (is_paused) {
                vga_show_paused();
                set_leds(0x3FF); // All LEDs on
            } else {
                vga_hide_paused();
                set_leds(0x002);
            }
        }
        if (is_paused) {
            delay_ms(50); 
            continue;     // Skip the rest of the loop
        }

        // ================================================================
        // SW0 FREEZE (examine waveform without overlay)
        // ================================================================
        int current_switches = get_sw();
        bool sw0_on = (current_switches & SW0_FREEZE) != 0;
        
        if (sw0_on != is_frozen) {   // detecting state change 
            is_frozen = sw0_on;
            if (is_frozen) {
                vga_show_freeze_indicator();
                set_leds(0x001);
            } else {
                vga_hide_freeze_indicator();
            }
        }
        if (is_frozen) {
            delay_ms(50);
            continue;  // Skip waveform updates - screen stays frozen
        }

        // ================================================================
        // SW6: Performance Test (rising edge detection)
        // ================================================================
        if (switch_pressed(current_switches, prev_switches, SW6_PERF_TEST)) {
            run_performance_test();
            // Wait for SW6 to be turned off before continuing
            while (get_sw() & SW6_PERF_TEST) {
                delay_ms(50);
            }     
            vga_init_scope(current_gain, current_sample_rate); // Restart display
            pos_x = graph_left;
            prev_x = graph_left;
            set_leds(0x001);
        }
        
        // ================================================================
        // GAIN SWITCHES (SW2, SW3, SW4)
        // ================================================================
        int new_gain = read_gain_from_switches(current_switches);
        if (new_gain != current_gain) {
            current_gain = new_gain;
            ad7705_set_gain(CHN_AIN1, current_gain);
            vga_update_settings(current_gain, current_sample_rate);

            if (current_gain == 8) {
                set_leds(0x10);
            } else if (current_gain == 4) {
                set_leds(0x08);
            } else if (current_gain == 2) {
                set_leds(0x04);
            } else {
                set_leds(0x01);
            }
        }

        // ================================================================
        // SAMPLE_RATE CHANGE SWITCHES (SW8, SW9)
        // ================================================================
        if (switch_pressed(current_switches, prev_switches, 0x200)) {
            if (current_sample_rate < MAX_SAMPLE_RATE) {
                current_sample_rate += 50;
                timer_init(current_sample_rate);
                vga_update_settings(current_gain, current_sample_rate);
                set_leds(0x200);
            }
        }
        if (switch_pressed(current_switches, prev_switches, 0x100)) {
            if (current_sample_rate > MIN_SAMPLE_RATE) {
                current_sample_rate -= 50;
                timer_init(current_sample_rate);
                vga_update_settings(current_gain, current_sample_rate);
                set_leds(0x100);
            }
        }
        prev_switches = current_switches;

      
        
        // ================================================================
        // WAIT FOR TIMER AND READ ADC
        // ================================================================ 
        while (!timer_check_tick()) {}
        
        float voltage = ad7705_read_voltage(CHN_AIN1);   // Read ADC

        // Update statistics
        if (voltage < sweep_min) sweep_min = voltage;
        if (voltage > sweep_max) sweep_max = voltage;
        
        display_7seg_voltage_gain(voltage, current_gain);

        // ================================================================
        // ERASE AHEAD OF WAVEFORM
        // ================================================================
        for (int i = 1; i <= ERASE_WIDTH; i++) {
            int erase_x = pos_x + i;  // look i pixels ahead of current position
            // If looking ahead goes offf screen, wrap to start 
            if (erase_x > graph_right) {  
                erase_x = graph_left + (erase_x - graph_right - 1);
            }
            vga_clear_column(erase_x);  // clearing just this column but keeping grid lines 
        }
        
        
        // ================================================================
        // DRAW WAVEFORM
        // ================================================================
        int current_y = vga_voltage_to_y(voltage, V_MIN, V_MAX);
        if (pos_x > graph_left) {
            // draws a line from previous point to the current point
            vga_draw_line(prev_x, prev_y, pos_x, current_y, COLOR_ORANGE, false);
        } else {
            // if we are at the very start(left edge), just drawing a dot
            vga_draw_pixel(pos_x, current_y, COLOR_ORANGE);
        }

        // Saving current positions as prevous for the next loopiteration 
        prev_x = pos_x;
        prev_y = current_y;
        pos_x++;  // Move the cursor one step right 

        // ================================================================
        // CHECK SWEEP COMPLETE
        // ================================================================ 
        if (pos_x > graph_right) {
            pos_x = graph_left; // going back to the left edge 
            prev_x = graph_left;
            sweep_count++;
            
            // Updating the numbers only once per full sweep 
            vga_draw_display_info(voltage, sweep_max, sweep_min, current_gain, current_sample_rate);  // update 
            
            // Reset min/max for the next sweep 
            sweep_min = V_MAX;
            sweep_max = V_MIN;
        }
    }
    return 0;
}