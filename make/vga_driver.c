#include "vga_driver.h"
#include "hardware.h" // For pVGA_PIXEL_BUFFER, SCREEN_WIDTH, etc.


#define GRID_DIVISIONS_X 10 
#define GRID_DIVISIONS_Y 8



// Provides the 'abs' function, required since we don't use the standard library.
int abs(int n)
{
    if (n < 0)
    {
        return -n;
    }
    return n;
}

// Fills the entire screen with a single color.
// This is fast as it loops through the memory buffer linearly.
void vga_clear_screen(uint16_t color){
    int total_pixels = SCREEN_WIDTH * SCREEN_HEIGHT;
    for (int i = 0; i < total_pixels; i++) {
        pVGA_PIXEL_BUFFER[i] = color;
    }
}


// Draws a single pixel on the screen at (x, y).
void vga_draw_pixel(int x, int y, uint16_t color) {
    // Check for out-of-bounds coordinates
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        int offset = (y * SCREEN_WIDTH) + x;
        pVGA_PIXEL_BUFFER[offset] = color;
    }
}



// Draws a line between (x1, y1) and (x2, y2) using Bresenham's algorithm.
void vga_draw_line(int x1, int y1, int x2, int y2, uint16_t color) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    int e2;

    while (1) {
        vga_draw_pixel(x1, y1, color);

        if (x1 == x2 && y1 == y2) {
            break;
        }

        e2 = 2 * err;
        // Use >= and <= to prevent stalls on straight lines
        if (e2 >= -dy) { 
            err -= dy;
            x1 += sx;
        }
        if (e2 <= dx) { 
            err += dx;
            y1 += sy;
        }
    }
}

// Draws a non-filled rectangle (a box outline).
void vga_draw_box_outline(int x, int y, int width, int height, uint16_t color) {
    int x2 = x + width - 1;
    int y2 = y + height - 1;

    // Draw the four lines that make up the rectangle
    vga_draw_line(x, y, x2, y, color);        // Top line
    vga_draw_line(x, y2, x2, y2, color);      // Bottom line
    vga_draw_line(x, y, x, y2, color);        // Left line
    vga_draw_line(x2, y, x2, y2, color);      // Right line
}




// Draws a solid, filled rectangle.
void vga_draw_filled_box(int x, int y, int width, int height, uint16_t color) {
    for (int current_y = y; current_y < (y + height); current_y++) {
        for (int current_x = x; current_x < (x + width); current_x++) {
            // vga_draw_pixel already handles boundary checks
            vga_draw_pixel(current_x, current_y, color);
        }
    }
}




void vga_draw_grid() {
    int i;
    
    // 1. Calculate Spacing
    int x_spacing = SCREEN_WIDTH / GRID_DIVISIONS_X; // 320 / 10 = 32 pixels
    int y_spacing = SCREEN_HEIGHT / GRID_DIVISIONS_Y; // 240 / 8 = 30 pixels

    // 2. Draw Vertical Grid Lines
    for (i = 1; i < GRID_DIVISIONS_X; i++) {
        vga_draw_line(i * x_spacing, 0, i * x_spacing, SCREEN_HEIGHT - 1, COLOR_GRID_BLUE);
    }

    // 3. Draw Horizontal Grid Lines
    for (i = 1; i < GRID_DIVISIONS_Y; i++) {
        vga_draw_line(0, i * y_spacing, SCREEN_WIDTH - 1, i * y_spacing, COLOR_GRID_BLUE);
    }

    // 4. Draw Main Axes
    vga_draw_line(0, SCREEN_HEIGHT / 2, SCREEN_WIDTH - 1, SCREEN_HEIGHT / 2, COLOR_DARK_GRAY); // X-axis
    vga_draw_line(SCREEN_WIDTH / 2, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT - 1, COLOR_DARK_GRAY); // Y-axis
}