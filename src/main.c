#include "hardware.h"
#include "vga_driver.h"
#include "ad7705_driver.h"
#include <stdint.h>


/**
 * @brief Provides the 'abs' function, which is not available when compiling
 * with the -nostdlib flag. Returns the absolute value of an integer.
 */
int abs(int n) {
    if (n < 0) {
        return -n;
    }
    return n;
}

/**
 * @brief A stub function to handle hardware interrupts. The system's startup
 * code requires this function to exist. For now, it does nothing.
 * @param cause The cause of the interrupt (unused for now).
 */
void handle_interrupt(unsigned cause) {
    // This function is called by the hardware interrupt handler in boot.S.
    // We will add code here later for the timer.
}



// We'll create a buffer to hold one full screen of ADC samples.
// The size matches the screen width for a 1-to-1 mapping of sample to pixel column.
#define WAVEFORM_BUFFER_SIZE VGA_WIDTH
uint16_t waveform_buffer[WAVEFORM_BUFFER_SIZE];

// Define the area on the screen where the waveform will be plotted.
// We leave a small margin at the top and bottom.
#define PLOT_Y_TOP    20
#define PLOT_Y_BOTTOM 220
#define PLOT_HEIGHT   (PLOT_Y_BOTTOM - PLOT_Y_TOP)

/**
 * @brief Draws the static user interface for the oscilloscope (the grid/graticule).
 */
void draw_ui(void) {
    // 1. Clear the entire screen to a black background.
    vga_clear_screen(COLOR_BLACK);

    // 2. Draw the vertical grid lines
    for (int x = 0; x < VGA_WIDTH; x += 32) { // A line every 32 pixels
        vga_draw_line(x, PLOT_Y_TOP, x, PLOT_Y_BOTTOM, COLOR_GRID_BLUE);
    }

    // 3. Draw the horizontal grid lines
    for (int y = PLOT_Y_TOP; y <= PLOT_Y_BOTTOM; y += 25) { // A line every 25 pixels
        vga_draw_line(0, y, VGA_WIDTH - 1, y, COLOR_GRID_BLUE);
    }

    // 4. Draw a white box around the plotting area
    vga_draw_box_outline(0, PLOT_Y_TOP, VGA_WIDTH - 1, PLOT_HEIGHT + 1, COLOR_WHITE);
}

/**
 * @brief Scales a 16-bit ADC value to a Y-coordinate on the screen.
 * @param adc_value The raw 16-bit value from the ADC (0-65535).
 * @return The corresponding Y-coordinate within the plotting area.
 */
int scale_adc_to_y(uint16_t adc_value) {
    // This is the core math of the oscilloscope. It maps the ADC's 16-bit range
    // to the pixel height of our plotting area.
    // We use a 32-bit intermediate value to prevent overflow during multiplication.
    uint32_t scaled_value = ((uint32_t)adc_value * PLOT_HEIGHT) / 65535;

    // Screen coordinates start at the top (y=0), so we subtract from the bottom
    // to make larger ADC values appear higher on the screen.
    return PLOT_Y_BOTTOM - (int)scaled_value;
}

/**
 * @brief Main program entry point.
 */
int main() {
    // 1. Initialize the AD7705. This performs the reset, configuration,
    //    and initial self-calibration.
    AD7705_init();

    // 2. Draw the oscilloscope grid once at the start.
    draw_ui();

    // These will hold the Y-coordinates for the previous and current points to draw a line
    int y_prev, y_curr;

    // 3. Enter the main, infinite loop of the oscilloscope.
    while (1) {
        // --- Step A: Acquire one full waveform ---
        // This loop fills our buffer with 320 consecutive samples from the ADC.
        for (int i = 0; i < WAVEFORM_BUFFER_SIZE; i++) {
            waveform_buffer[i] = AD7705_read_data();
        }

        // --- Step B: Draw the new waveform ---
        // To keep the code simple, we will redraw the grid to erase the old
        // waveform. Note: This will cause some flicker. A more advanced version
        // would only erase the pixels of the old line.
        draw_ui();

        // Get the initial point for the start of the line.
        y_prev = scale_adc_to_y(waveform_buffer[0]);

        // Draw the waveform by connecting each sample point to the next with a line.
        for (int x = 1; x < WAVEFORM_BUFFER_SIZE; x++) {
            // Get the Y-coordinate for the current data point
            y_curr = scale_adc_to_y(waveform_buffer[x]);

            // Draw a short line segment from the previous point to the current one
            vga_draw_line(x - 1, y_prev, x, y_curr, COLOR_GREEN);

            // The current point becomes the previous point for the next segment
            y_prev = y_curr;
        }
    }

    return 0; // This line is never reached
}