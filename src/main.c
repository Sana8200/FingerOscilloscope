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


#define V_MIN           0.0f    // Minimum voltage on display
#define V_MAX           3.3f    // Maximum voltage on display
#define ERASE_WIDTH     3       // Pixels to clear ahead of waveform

#define DEFAULT_SAMPLE_RATE   150     // Samples per second
#define MIN_SAMPLE_RATE  50
#define MAX_SAMPLE_RATE 500

// Graph area boundaries
static int graph_left;
static int graph_right;
static int graph_top;
static int graph_bottom;

// Current and previous drawing positions
static int pos_x;
static int prev_x;
static int prev_y;

// Statistics
static float sweep_min;
static float sweep_max;
static int sweep_count;

static int current_gain = 1;
static int current_sample_rate = DEFAULT_SAMPLE_RATE;
static bool is_paused = false;
static int prev_switches = 0;   




int main(void) {
    display_string("\n");
    display_string("============================================= DE10-Lite Oscilloscope =============================================\n");
    display_string("Controls:");
    display_string("  Button  : Pause/Resume");
    display_string("  SW2     : Gain = 2");
    display_string("  SW3     : Gain = 4");
    display_string("  SW4     : Gain = 8");
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
    set_leds(0x001);
    display_7seg_voltage_gain(0.0f, current_gain);
    
    while (1) {
        if (get_btn()) {
            // Wait until button is RELEASED
            while (get_btn()) {
                delay_ms(10);
            }
            delay_ms(50); // Debounce
            // Toggle Pause State
            is_paused = !is_paused;

            if (is_paused) {
                vga_show_paused();
                set_leds(0x3FF); // All LEDs on
            } else {
                vga_hide_paused();
                set_leds(0x001);
            }
        }
        
        // 2. PAUSE LOGIC
        if (is_paused) {
            delay_ms(50); // Idle
            continue;     // Skip the rest of the loop
        }
        
        // Handle gain switches (SW2, SW3, SW4)
        int new_gain = read_gain_from_switches(get_sw());
        if(new_gain != current_gain){
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

        

        // Handle smaple rate switches (SW8, SW9)(SW9 increase, SW8 decrease)
        if (switch_pressed(get_sw(), prev_switches, 0x200)) {
            if (current_sample_rate < MAX_SAMPLE_RATE) {
                current_sample_rate += 50;
                timer_init(current_sample_rate);
                vga_update_settings(current_gain, current_sample_rate);
                set_leds(0x200);
            }
        }
        if (switch_pressed(get_sw(), prev_switches, 0x100)) {
            if (current_sample_rate > MIN_SAMPLE_RATE) {
                current_sample_rate -= 50;
                timer_init(current_sample_rate);
                vga_update_settings(current_gain, current_sample_rate);
                set_leds(0x100);
            }
        }
        prev_switches = get_sw();

      
        
        // Waiting for timer tick 
        while (!timer_check_tick()) {
            // Wait for next sample 
        }
        
        // Read ADC
        float voltage = ad7705_read_voltage(CHN_AIN1);

        // Update statistics
        if (voltage < sweep_min) sweep_min = voltage;
        if (voltage > sweep_max) sweep_max = voltage;
        
        display_7seg_voltage_gain(voltage, current_gain);

    
        
        
        // Erase ahead of waveform
        for (int i = 1; i <= ERASE_WIDTH; i++) {
            int erase_x = pos_x + i;
            if (erase_x > graph_right) {
                erase_x = graph_left + (erase_x - graph_right - 1);
            }
            vga_clear_column(erase_x);
        }
        
        
        // Draw waveform point
        int current_y = vga_voltage_to_y(voltage, V_MIN, V_MAX);
        if (pos_x > graph_left) {
            vga_draw_line(prev_x, prev_y, pos_x, current_y, COLOR_ORANGE);
        } else {
            vga_draw_pixel(pos_x, current_y, COLOR_ORANGE);
        }


        prev_x = pos_x;
        prev_y = current_y;
        pos_x++;
        // Checking if Sweep is complete 
        if (pos_x > graph_right) {
            // End of sweep
            pos_x = graph_left;
            prev_x = graph_left;
            sweep_count++;
            
            // Update header
            vga_draw_header(voltage, sweep_max, sweep_min, current_gain, current_sample_rate);
            
            // Reset stats
            sweep_min = V_MAX;
            sweep_max = V_MIN;
        }
    }
    
    return 0;
}