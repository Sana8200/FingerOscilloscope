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


#define V_MIN           0.0f    
#define V_MAX           3.3f   
#define ERASE_WIDTH     3       

#define DEFAULT_SAMPLE_RATE   150     
#define MIN_SAMPLE_RATE  50
#define MAX_SAMPLE_RATE 500


// Number of samples to for performance measurement
#define PERF_TEST_SAMPLES  500

// Perfromance Measuremnt
static int sample_count = 0;
static bool perf_done = false;
 

static int graph_left;
static int graph_right;
static int graph_top;
static int graph_bottom;

static int pos_x;  
static int prev_x;
static int prev_y;

static float sweep_min;
static float sweep_max;
static int sweep_count;

static int current_gain = 1;
static int current_sample_rate = DEFAULT_SAMPLE_RATE;
static bool is_paused = false;
static bool is_frozen = false;
static int prev_switches = 0;   

static inline uint32_t get_cycles(void) {
    uint32_t cycles;
    asm volatile ("csrr %0, mcycle" : "=r" (cycles));
    return cycles;
}

uint32_t time_idle_start, time_work_start, time_work_end;
uint32_t total_active_cycles = 0;
uint32_t total_idle_cycles = 0;


int main(void) {
    print("\n");
    print("============================================= DE10-Lite Oscilloscope =============================================\n");
    print("Controls:");
    print("  Button  : Pause/Resume");
    print("  SW0     : Freeze (examine waveform)");
    print("  SW2     : Gain = 2");
    print("  SW3     : Gain = 4");
    print("  SW4     : Gain = 8");
    print("  SW8     : Sample Rate -50");
    print("  SW9     : Sample Rate +50\n\n");

    print("Init Timer...");
    timer_init(current_sample_rate);
    
    print("Init SPI...");
    spi_init();
    delay_ms(50);
    
    print("Init ADC...");
    ad7705_init(CHN_AIN1);
    delay_ms(100);
    
    print("Init VGA...");
    vga_init_scope(current_gain, current_sample_rate);
    vga_get_graph_area(&graph_left, &graph_right, &graph_top, &graph_bottom);
    
    pos_x = graph_left;
    prev_x = graph_left;
    prev_y = (graph_top + graph_bottom) / 2;
    sweep_min = V_MAX;
    sweep_max = V_MIN;
    sweep_count = 0;
    
    print("\nReady!\n");
    set_leds(0x002);
    display_7seg_voltage_gain(0.0f, current_gain);

    // Clearning counters before main loop 
    clear_counters();

    
    
    while (1) {
        time_idle_start = get_cycles();
        
        // ================================================================
        // BUTTON PAUSE 
        // ================================================================
        if (get_btn()) {
            while (get_btn()) {
                delay_ms(10);
            }
            delay_ms(50); 
            is_paused = !is_paused;  

            if (is_paused) {
                vga_show_paused();
                set_leds(0x3FF); 
            } else {
                vga_hide_paused();
                set_leds(0x002);
            }
        }
        if (is_paused) {
            delay_ms(50); 
            continue;     
        }

        // ================================================================
        // SW0 FREEZE (examine waveform without overlay)
        // ================================================================
        int current_switches = get_sw();
        bool sw0_on = (current_switches & SW0_FREEZE) != 0;
        
        if (sw0_on != is_frozen) {   
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
            continue;  
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
        
        float voltage = ad7705_read_voltage(CHN_AIN1);   

        if (voltage < sweep_min) sweep_min = voltage;
        if (voltage > sweep_max) sweep_max = voltage;
        
        display_7seg_voltage_gain(voltage, current_gain);

        // ================================================================
        // ERASE AHEAD OF WAVEFORM
        // ================================================================
        for (int i = 1; i <= ERASE_WIDTH; i++) {
            int erase_x = pos_x + i;  
            if (erase_x > graph_right) {  
                erase_x = graph_left + (erase_x - graph_right - 1);
            }
            vga_clear_column(erase_x);  
        }
        
        
        // ================================================================
        // DRAW WAVEFORM
        // ================================================================
        int current_y = vga_voltage_to_y(voltage, V_MIN, V_MAX);
        if (pos_x > graph_left) {
            vga_draw_line(prev_x, prev_y, pos_x, current_y, COLOR_ORANGE, false);
        } else {
            vga_draw_pixel(pos_x, current_y, COLOR_ORANGE);
        }
        prev_x = pos_x;
        prev_y = current_y;
        pos_x++;  

        // ================================================================
        // CHECK SWEEP COMPLETE
        // ================================================================ 
        if (pos_x > graph_right) {
            pos_x = graph_left; 
            prev_x = graph_left;
            sweep_count++;
            
            vga_draw_display_info(voltage, sweep_max, sweep_min, current_gain, current_sample_rate);  // update 
            
            // Reset min/max for the next sweep 
            sweep_min = V_MAX;
            sweep_max = V_MIN;
        }

        sample_count++;
        if(sample_count == PERF_TEST_SAMPLES && !perf_done){
            print_counters();
            perf_done = true;
            print("Oscilloscope continues running...\n");
        }
    }
    return 0;
}