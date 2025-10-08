#include "framebuffer.h"
#include <math.h>
#include <stdint.h>

Color framebuffer[SCREEN_HEIGHT][SCREEN_WIDTH];

// Forward declaration for buttons
extern void handle_buttons(void);
extern volatile int current_state;

// --- Set a single pixel ---
void set_pixel(int x, int y, Color color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        framebuffer[y][x] = color;
    }
}

// --- Clear screen ---
void clear_screen(Color color) {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            framebuffer[y][x] = color;
        }
    }
}

// --- Draw waveform with state machine ---
void draw_waveform() {
    handle_buttons();  // check buttons every frame

    clear_screen(COLOR_BLACK);

    Color red = COLOR_RED;
    Color green = COLOR_GREEN;

    // Draw grid
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        set_pixel(x, SCREEN_HEIGHT / 2, red);
        set_pixel(SCREEN_WIDTH / 2, x % SCREEN_HEIGHT, red);
    }

    // Draw sine wave for LIVE_VIEW or PAUSED
    if (current_state == 0 || current_state == 1) { // 0=LIVE_VIEW, 1=PAUSED
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            float scale = 60.0f;
            float offset = 120.0f;
            float frequency = 30.0f;
            int y_value = (int)(offset + scale * sinf((float)x / frequency));
            set_pixel(x, y_value, green);
        }
    }

    // Menu placeholder
    if (current_state == 2) { // MENU
        for (int y = 100; y < 140; y++) {
            for (int x = 140; x < 180; x++) {
                set_pixel(x, y, COLOR_WHITE);
            }
        }
    }
}
