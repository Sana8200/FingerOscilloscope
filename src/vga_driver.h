#ifndef VGA_DRIVER_H
#define VGA_DRIVER_H
#include <stdint.h> // For uint16_t, the type for our pixel color


// Screen dimensions in pixels for the standard 320x240 VGA controller mode
#define VGA_WIDTH  320
#define VGA_HEIGHT 240


// VGA Color Definitions (16-bit RGB 5-6-5 format)
// A 16-bit color value is packed into a uint16_t as follows:
// Bits 15-11: Red   (5 bits)
// Bits 10-5:  Green (6 bits)
// Bits 4-0:   Blue  (5 bits)
// This helper macro creates a 16-bit color value from 8-bit R, G, B components.
// It correctly shifts and masks the values.
#define RGB_TO_16BIT(r,g,b) ( (((r) >> 3) << 11) | (((g) >> 2) << 5) | ((b) >> 3) )

// Pre-defined basic colors in 16-bit format
#define COLOR_BLACK         0x0000
#define COLOR_WHITE         0xFFFF
#define COLOR_RED           0xF800
#define COLOR_GREEN         0x07E0
#define COLOR_BLUE          0x001F
#define COLOR_YELLOW        (COLOR_RED | COLOR_GREEN)
#define COLOR_CYAN          (COLOR_GREEN | COLOR_BLUE)
#define COLOR_MAGENTA       (COLOR_RED | COLOR_BLUE)
#define COLOR_DARK_GRAY     0x3186
#define COLOR_GRID_BLUE     0x0210  // A dark blue for the oscilloscope grid


//=============================================================================
//                     VGA Driver Function Prototypes
//=============================================================================
// These are the functions we will implement in vga_driver.c to draw things.

/**
 * @brief Draws a single pixel on the screen.
 * @param x The horizontal coordinate (0 to VGA_WIDTH - 1).
 * @param y The vertical coordinate (0 to VGA_HEIGHT - 1).
 * @param color The 16-bit color of the pixel.
 */
void vga_draw_pixel(int x, int y, uint16_t color);

/**
 * @brief Fills the entire screen with a single solid color.
 * @param color The 16-bit color to fill the screen with.
 */
void vga_clear_screen(uint16_t color);

/**
 * @brief Draws a line between two points on the screen.
 * @param x1 The starting x-coordinate.
 * @param y1 The starting y-coordinate.
 * @param x2 The ending x-coordinate.
 * @param y2 The ending y-coordinate.
 * @param color The 16-bit color of the line.
 */
void vga_draw_line(int x1, int y1, int x2, int y2, uint16_t color);

/**
 * @brief Draws a simple, non-filled rectangle (a box).
 * @param x The top-left x-coordinate.
 * @param y The top-left y-coordinate.
 * @param width The width of the box.
 * @param height The height of the box.
 * @param color The 16-bit color of the box outline.
 */
void vga_draw_box_outline(int x, int y, int width, int height, uint16_t color);


#endif // VGA_DRIVER_H