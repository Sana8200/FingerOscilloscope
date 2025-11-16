
#include "vga_driver.h"
#include "ad7705_driver.h"
#include "hardware.h"
#include <stdint.h>
#include <stdbool.h>
#include "dtekv-lib.h"




void handle_interrupt(unsigned cause) {

}



// This buffer stores all the y-coordinates for the waveform.
// static uint16_t sample_buffer[SCREEN_WIDTH];


// Helper function to scale the 16-bit ADC value (0-65535) to a Y-coordinate (0-239)
static int scale_adc_to_y(uint16_t adc_val) {
    // This is a simple linear scaling.
    // 65535 (max ADC) / 239 (max Y) = ~274
    // We invert the result (239 - ...) so that 0V is at the bottom (y=239)
    // and max voltage is at the top (y=0).
    int y = 239 - (adc_val / 274);

    // Clamp the value just in case of rounding
    if (y < 0) y = 0;
    if (y > 239) y = 239;
    return y;
}

// Redraws the small vertical segment of the grid at column x.
// This is used to "patch" the grid after erasing a pixel.
static void redraw_grid_column(int x) {
    // Check if x is on a vertical grid line
    if ((x % (SCREEN_WIDTH / 10)) == 0) {
        vga_draw_line(x, 0, x, SCREEN_HEIGHT - 1, COLOR_GRID_BLUE);
    }
    // Check if x is on the vertical axis
    if (x == (SCREEN_WIDTH / 2)) {
        vga_draw_line(x, 0, x, SCREEN_HEIGHT - 1, COLOR_DARK_GRAY);
    }
}


int main() {
    
    // --- 1. Initialization ---
    vga_clear_screen(COLOR_BLACK);
    vga_draw_grid();
    
    // This will initialize SPI and set the default gain to 1
    ad7705_init(); 

    // --- 2. State Variables ---
    int x_pos = 0;         // Current horizontal position (0-319)
    uint16_t new_adc_val;
    int new_y;

    // Use switches 0, 1, and 2 to select gain
    //uint8_t current_gain_code = *pSWITCHES & 0x7; 
    
    // Buffer to store old Y-values for flicker-free erasing
    int old_y_values[SCREEN_WIDTH];
    for (int i = 0; i < SCREEN_WIDTH; i++) {
        old_y_values[i] = SCREEN_HEIGHT / 2; // Start with a flat line
    }

    // Main Oscilloscope Loop 
    while(1) {

        // Data 
        new_adc_val = ad7705_read_data();
        new_y = scale_adc_to_y(new_adc_val);


        print("ADC Value: ");
        print_dec(new_adc_val);
    
        
        // Erase Old Pixel 
        // Erase the old pixel by redrawing the background
        if ((old_y_values[x_pos] % (SCREEN_HEIGHT / 8)) == 0) {
            vga_draw_pixel(x_pos, old_y_values[x_pos], COLOR_RED);
        } else if (old_y_values[x_pos] == (SCREEN_HEIGHT / 2)) {
            vga_draw_pixel(x_pos, old_y_values[x_pos], COLOR_DARK_GRAY);
        } else {
            vga_draw_pixel(x_pos, old_y_values[x_pos], COLOR_BLACK);
        }
        
        // Redraw the vertical grid line that was erased
        redraw_grid_column(x_pos);

        // Draw New Pixel 
        vga_draw_pixel(x_pos, new_y, COLOR_YELLOW);
        
        // Update State
        old_y_values[x_pos] = new_y; 
        
        x_pos++; // Move to the next horizontal pixel
        if (x_pos >= SCREEN_WIDTH) {
            x_pos = 0; 
        }

        // Flash the LEDs
       // *pLEDS = (current_gain_code << 4) | (x_pos >> 5); 
    }
    
    return 0; 
}