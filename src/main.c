
#include "vga_driver.h"
#include "ad7705_driver.h"
#include "hardware.h"
#include <stdint.h>
#include <stdbool.h>




void handle_interrupt(unsigned cause) {

}



// This buffer stores all the y-coordinates for the waveform.
static uint16_t sample_buffer[SCREEN_WIDTH];


// Helper function to scale the 16-bit ADC value (0-65535) to a Y-coordinate (0-239)
int scale_adc_to_y(uint16_t adc_val) {
    // This scales 0 -> 239 (bottom) and 65535 -> 0 (top)
    // 65535 / 274 = 239.17
    return 239 - (adc_val / 274);
}



int main() {
    
    // --- 1. Initialization ---
    vga_clear_screen(COLOR_BLACK);
    //vga_draw_box_outline();
    ad7705_init(); // Initialize and calibrate the ADC

    // Fill the sample buffer with an initial "ground" reading
    uint16_t initial_val = ad7705_read_data();
    int initial_y = scale_adc_to_y(initial_val);
    for (int i = 0; i < SCREEN_WIDTH; i++) {
        sample_buffer[i] = initial_y;
    }

    int x_pos = 0; // Current horizontal position on the screen
    
    while(1) {
        
        // 3. Acquire ONE new sample ---
        // This call will pause until the ADC has new data (~20ms)
        uint16_t new_adc_val = ad7705_read_data();
        int new_y = scale_adc_to_y(new_adc_val);


        // 4. Erase Old Pixel and Redraw Grid ---
        vga_draw_line(x_pos, 0, x_pos, SCREEN_HEIGHT - 1, COLOR_BLACK);

        // Redraw the grid lines that we just erased
        if (x_pos % (SCREEN_WIDTH / 10) == 0) { // 10 = GRID_DIVISIONS_X
            vga_draw_line(x_pos, 0, x_pos, SCREEN_HEIGHT - 1, COLOR_DARK_GRAY); // Main axis
        } else {
            vga_draw_line(x_pos, 0, x_pos, SCREEN_HEIGHT - 1, COLOR_GRID_BLUE); // Minor axis
        }
        
        // Redraw the horizontal grid pixels that got erased
        for (int i = 1; i < 8; i++) { // 8 = GRID_DIVISIONS_Y
            vga_draw_pixel(x_pos, i * (SCREEN_HEIGHT / 8), COLOR_GRID_BLUE);
        }
        vga_draw_pixel(x_pos, SCREEN_HEIGHT / 2, COLOR_DARK_GRAY); // Horizontal main axis


        // 5. Draw New Pixel and Update Buffer ---
        vga_draw_pixel(x_pos, new_y, COLOR_YELLOW);
        
        // Store the new value in our buffer
        sample_buffer[x_pos] = new_y;

        // Move to the next horizontal position ---
        x_pos++;
        if (x_pos >= SCREEN_WIDTH) {
            x_pos = 0; // Wrap around to the beginning
        }
        
        // Flash LEDs to show the main loop is running
        // The LED pattern will "scroll" as x_pos increases
        *pLEDS = (1 << (x_pos % 8)); 
    }
    
    return 0;
}
