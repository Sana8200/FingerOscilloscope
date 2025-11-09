#include "hardware.h"
#include "vga_driver.h"
#include "ad7705_driver.h"




void handle_interrupt(unsigned cause) {

}



// Global to track the last plotted point
int last_y = SCREEN_HEIGHT / 2;
int current_x = 0;

/**
 * @brief Scales a 16-bit ADC value (0-65535) to a Y-coordinate (0-239).
 */
int scale_adc_to_y(uint16_t adc_val) {
    // 65535 / 239 = 274.18
    // We can approximate this with integer division.
    // This scales the 0-65535 range to fit the 0-239 pixel screen height.
    // We subtract from 239 to flip it, since 0V=0 and 5V=65535 (bottom to top)
    return 239 - (adc_val / 274);
}

int main() {
    
    // --- Static Setup ---
    vga_clear_screen(COLOR_BLACK);
    // Draw a horizontal line in the middle of the screen
    vga_draw_line(0, SCREEN_HEIGHT / 2, SCREEN_WIDTH - 1, SCREEN_HEIGHT / 2, COLOR_GRID_BLUE);

    // --- Initialize Hardware ---
    ad7705_init(); // This will reset and calibrate the ADC

    // --- Main Loop ---
    while(1) {
        
        // 1. ERASE the old pixel
        //    (This also erases the grid line, so we redraw it)
        vga_draw_pixel(current_x, last_y, COLOR_BLACK);
        vga_draw_pixel(current_x, SCREEN_HEIGHT / 2, COLOR_GRID_BLUE); // Redraw grid

        // 2. READ a new value from the ADC
        uint16_t adc_val = ad7705_read_data();
        
        // 3. SCALE the value to a Y-coordinate
        int current_y = scale_adc_to_y(adc_val);
        
        // 4. DRAW the new pixel
        vga_draw_pixel(current_x, current_y, COLOR_YELLOW);
        
        // 5. UPDATE state for next loop
        last_y = current_y;
        current_x++;
        
        // 6. WRAP screen
        if (current_x >= SCREEN_WIDTH) {
            current_x = 0;
            // Clear the screen when we wrap
            vga_clear_screen(COLOR_BLACK);
            vga_draw_line(0, SCREEN_HEIGHT / 2, SCREEN_WIDTH - 1, SCREEN_HEIGHT / 2, COLOR_GRID_BLUE);
        }
    }
    
    return 0;
}

