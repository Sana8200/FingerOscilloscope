#ifndef VGA_DRIVER_H
#define VGA_DRIVER_H
#include <stdint.h> // For uint16_t, the type for our pixel color


// Screen dimensions in pixels for the standard 320x240 VGA controller mode
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240


// --- VGA Controller ---
// The start address of the 320x240 pixel buffer in memory.
#define VGA_PIXEL_BUFFER_BASE   0x08000000
#define pVGA_PIXEL_BUFFER      ((volatile uint16_t *) VGA_PIXEL_BUFFER_BASE)


// --- VGA Color Definitions (8-bit RGB 3-3-2 format) ---
/* * Format: RRR GGG BB
 * RRR = 3 bits for Red
 * GGG = 3 bits for Green
 * BB  = 2 bits for Blue
 */
#define COLOR_BLACK         0x00 // 0b000 000 00
#define COLOR_WHITE         0xFF // 0b111 111 11
#define COLOR_RED           0xE0 // 0b111 000 00
#define COLOR_GREEN         0x1C // 0b000 111 00
#define COLOR_BLUE          0x03 // 0b000 000 11
#define COLOR_YELLOW        0xFC // 0b111 111 00 (Red + Green)
#define COLOR_CYAN          0x1F // 0b000 111 11 (Green + Blue)
#define COLOR_MAGENTA       0xE3 // 0b111 000 11 (Red + Blue)
#define COLOR_DARK_GRAY     0x92 // 0b100 100 10
#define COLOR_GRID_BLUE     0x05 // 0b000 001 01 (A very dark blue)




void vga_draw_pixel(int x, int y, uint16_t color);
void vga_clear_screen(uint16_t color);
void vga_draw_line(int x1, int y1, int x2, int y2, uint16_t color);
void vga_draw_box_outline(int x, int y, int width, int height, uint16_t color);
void vga_draw_filled_box(int x, int y, int width, int height, uint16_t color);

#endif // VGA_DRIVER_H