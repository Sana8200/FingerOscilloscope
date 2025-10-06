#include "framebuffer.h"

// --- Global Framebuffer Definition ---
// This allocates the 75 KB of memory needed for the 320x240 screen.
Color framebuffer[SCREEN_HEIGHT][SCREEN_WIDTH];

/**
 * @brief Sets a single pixel color in the framebuffer.
 */
void set_pixel(int x, int y, Color color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        framebuffer[y][x] = color;
    }
}

/**
 * @brief Clears the screen and draws a mock waveform.
 */
void draw_waveform() {
    // 1. Clear the screen to Black (Color 0)
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            framebuffer[y][x] = 0; // Black (R=0, G=0, B=0)
        }
    }

    // 2. Define colors
    Color green = 0b010; // Green trace
    Color red   = 0b100; // Red grid lines

    // 3. Draw a simple grid (Example: horizontal center line)
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        set_pixel(x, SCREEN_HEIGHT / 2, red);
        set_pixel(SCREEN_WIDTH / 2, x % SCREEN_HEIGHT, red); // Vertical line example
    }

    // 4. Draw a mock waveform (like an oscilloscope trace)
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        // Calculate Y value: scaled sine wave + offset for center
        float scale = 60.0f; 
        float offset = 120.0f; 
        float frequency = 30.0f; 
        
        int y_value = (int)(offset + scale * sin((float)x / frequency));
        
        set_pixel(x, y_value, green);
    }
}


