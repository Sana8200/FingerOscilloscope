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



int main() {
    
    int frame_counter = 0;

    // --- Static Setup ---
    // 1. Clear the entire screen to black first
    vga_clear_screen(COLOR_BLACK);
    
    // 2. Draw the static grid
    vga_draw_grid();

    // --- Main Loop ---
    // This loop simulates the continuous redrawing
    while(1) {
        
        // --- Redraw dynamic elements ---
        
        // 1. Redraw the grid (to "erase" the old wave)
        //    *This is slow!* We will optimize this later.
        //    For now, it's the easiest way to clear the old signal.
        vga_draw_grid();

        // 2. Draw the new, animated sine wave
        vga_draw_test_wave(frame_counter);

        // 3. Increment the counter to animate the phase
        frame_counter++;
    }
    
    return 0;
}