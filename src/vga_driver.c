#include "vga_driver.h"
#include "hardware.h"   // To get the VGA buffer address pointer
#include <stdlib.h>     // For abs() used in the line drawing algorithm

/**
 * @brief Draws a single pixel on the screen.
 *
 * This is the fundamental drawing function. It calculates the memory address
 * for a given (x, y) coordinate and writes the 16-bit color value to it.
 * The hardware VGA controller reads this memory and displays the pixel.
 *
 * @param x The horizontal coordinate (0 to VGA_WIDTH - 1).
 * @param y The vertical coordinate (0 to VGA_HEIGHT - 1).
 * @param color The 16-bit color of the pixel.
 */
void vga_draw_pixel(int x, int y, uint16_t color) {
    // Check if the coordinates are within the screen boundaries.
    // If not, do nothing, to prevent writing to invalid memory.
    if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT) {
        // Calculate the memory offset for the pixel.
        // The screen is a grid, so the address is (y * width) + x.
        int offset = (y * VGA_WIDTH) + x;

        // Write the color to the calculated address in the pixel buffer.
        // The 'pVGA_PIXEL_BUFFER' is our direct pointer to the video memory.
        pVGA_PIXEL_BUFFER[offset] = color;
    }
}

/**
 * @brief Fills the entire screen with a single solid color.
 *
 * This is much faster than calling vga_draw_pixel() for every pixel,
 * as it loops through the memory buffer linearly.
 *
 * @param color The 16-bit color to fill the screen with.
 */
void vga_clear_screen(uint16_t color) {
    int total_pixels = VGA_WIDTH * VGA_HEIGHT;
    for (int i = 0; i < total_pixels; i++) {
        // Write the color to every single location in the buffer.
        pVGA_PIXEL_BUFFER[i] = color;
    }
}

/**
 * @brief Draws a line between two points using Bresenham's line algorithm.
 *
 * This is a classic, efficient algorithm that uses only integer arithmetic
 * to determine which pixels should be drawn to form a line.
 *
 * @param x1 The starting x-coordinate.
 * @param y1 The starting y-coordinate.
 * @param x2 The ending x-coordinate.
 * @param y2 The ending y-coordinate.
 * @param color The 16-bit color of the line.
 */
void vga_draw_line(int x1, int y1, int x2, int y2, uint16_t color) {
    // Calculate the differences in x and y
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);

    // Determine the direction of the line
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;

    // The core of Bresenham's algorithm
    int err = dx - dy;
    int e2;

    while (1) {
        // Draw the current pixel
        vga_draw_pixel(x1, y1, color);

        // Check if we've reached the end point
        if (x1 == x2 && y1 == y2) {
            break;
        }

        e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}


/**
 * @brief Draws a simple, non-filled rectangle (a box outline).
 *
 * @param x The top-left x-coordinate.
 * @param y The top-left y-coordinate.
 * @param width The width of the box.
 * @param height The height of the box.
 * @param color The 16-bit color of the box outline.
 */
void vga_draw_box_outline(int x, int y, int width, int height, uint16_t color) {
    int x2 = x + width - 1;
    int y2 = y + height - 1;

    // Draw the four lines that make up the rectangle
    vga_draw_line(x, y, x2, y, color);        // Top line
    vga_draw_line(x, y2, x2, y2, color);      // Bottom line
    vga_draw_line(x, y, x, y2, color);        // Left line
    vga_draw_line(x2, y, x2, y2, color);      // Right line
}