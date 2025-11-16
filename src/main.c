
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


// linear scale the 16-bit ADC value (0-65535) to a Y-coordinate (0-239)
static int scale_adc_to_y(uint16_t adc_val) {
    // 65535 (max ADC) / 239 (max Y) = ~274
    int y = 239 - (adc_val / 274);

    // in case of rounding
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
    // Initialize Hardware
    spi_init();
    ad7705_init();
    
    // Clear VGA 
    vga_clear_screen(COLOR_BLACK);
    vga_draw_filled_box(10, 10, 50, 50, COLOR_RED);

    uint16_t adc_value = 0;

    while (1) {
        // Read ADC
        adc_value = ad7705_read_data();

        // 3. Display on LEDs (MSB 10 bits)
        // AD7705 is 16-bit. We shift right by 6 to fit on 10 LEDs.
        set_leds(adc_value >> 6);
        
        // Simple delay so we don't flood the serial bus
        for(volatile int i=0; i<50000; i++);
    }
    return 0;
}