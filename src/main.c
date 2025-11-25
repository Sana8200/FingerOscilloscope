#include <stdint.h>
#include <stdbool.h>
#include "lib.h"
#include "hardware.h"
#include "spi_driver.h"
#include "ad7705_driver.h"
#include "vga_driver.h"
#include "timer.h"
#include "dtekv-lib.h"


#define SCOPE_SAMPLE_RATE_HZ 100     // Read ADC 100 times per second


void handle_interrupt(unsigned cause) {
}

void delay_msa(volatile uint32_t ms) {
    ms *= SYSTEM_CLOCK_FREQ / 1000;
    while(ms !=0) {
        __asm("nop"); // Prevents compiler optimization
        --ms;
    }
}

// Buffer to store the history of ADC readings (one per screen column)
uint8_t waveform_buffer[SCREEN_WIDTH]; 

// Current drawing position on the X-axis
int current_x = 0;

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




int main() {

    // Initialize Timer to tick 100 times per second
    timer_init(SCOPE_SAMPLE_RATE_HZ);

    display_string("start...");
//    delay_ns(10000000000);
//    display_string("stop...");
    //delay_ms(1000);
    spi_init();
    //delay_ms(1000);
    
    ad7705_init(CHN_AIN1);

    //VGA Setup 
    vga_clear_screen(COLOR_BLACK);
    vga_draw_grid(); 

    // Clear the buffer to "middle" of screen initially
    for (int i = 0; i < SCREEN_WIDTH; i++) {
        waveform_buffer[i] = SCREEN_HEIGHT / 2; 
    }

    // Main Loop 
    while (1) {

            uint16_t adc_raw = ad7705_read_data(CHN_AIN1);

            // adc value shown on leds changing 
            set_leds(adc_raw >> 8);

            // Calculate new Y coordinate
            uint8_t new_y = map_adc_to_screen_y(adc_raw);
            print_dec(adc_raw);
            
            // Erase the "Old" data at current X, We draw a vertical line of background color to clear the previous trace
            vga_draw_line(current_x, 0, current_x, SCREEN_HEIGHT-1, COLOR_BLACK);
            
            // Restore grid line if we erased it
            if (current_x % 32 == 0) {
                vga_draw_line(current_x, 0, current_x, SCREEN_HEIGHT-1, COLOR_CYAN); 
            }
            
            // Draw the "New" data
            // Connect previous point to current point for a continuous line
            int prev_x = (current_x == 0) ? 0 : current_x - 1;
            uint8_t prev_y = waveform_buffer[prev_x];

            if (current_x == 0) {
                // Start of screen, just draw a pixel
                vga_draw_pixel(current_x, new_y, COLOR_GREEN);
            } else {
                // Draw line from previous sample to this one
                vga_draw_line(prev_x, prev_y, current_x, new_y, COLOR_GREEN);
            }
            
            // Update buffer
            waveform_buffer[current_x] = new_y;

            // Increment X and wrap around
            current_x++;
            if (current_x >= SCREEN_WIDTH) {
                current_x = 0;
            }
        }
//    }
    return 0;
}