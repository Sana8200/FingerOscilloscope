#ifndef VGA_DRIVER_H
#define VGA_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

// ==========================================
// CONFIGURATION
// ==========================================
#define SCREEN_WIDTH        320
#define SCREEN_HEIGHT       240
#define VGA_BUFFER_BASE     0x08000000

// Pointer to the pixel buffer (8-bit color depth)
#define pVGA_PIXEL_BUFFER   ((volatile uint8_t *) VGA_BUFFER_BASE)

// ==========================================
// PRO_STYLE COLOR PALETTE (RRRGGGBB 8-bit)
// ==========================================
#define COLOR_BLACK         0x00
#define COLOR_WHITE         0xFF
#define COLOR_RED           0xE0 
#define COLOR_GREEN         0x1C 
#define COLOR_BLUE          0x03 
#define COLOR_YELLOW        0xFC 
#define COLOR_CYAN          0x1F 
#define COLOR_MAGENTA       0xE3 
#define COLOR_GRID_GRAY     0x49 // Dim gray for the grid lines
#define COLOR_ORANGE        0xEC // A nice orange for the waveform

// ==========================================
// FUNCTION PROTOTYPES
// ==========================================

void vga_clear_screen(uint8_t color);
void vga_draw_pixel(int x, int y, uint8_t color);
void vga_draw_line(int x0, int y0, int x1, int y1, uint8_t color);
void vga_draw_dashed_line(int x0, int y0, int x1, int y1, uint8_t color);
void vga_draw_rect(int x, int y, int w, int h, uint8_t color);
void vga_draw_filled_rect(int x, int y, int w, int h, uint8_t color);

// Text Functions
void vga_draw_char(int x, int y, char c, uint8_t color);
void vga_draw_string(int x, int y, const char *str, uint8_t color);

// Scientific UI Functions
void vga_draw_grid_axis(void);

// Helper
int vga_map_voltage(float voltage, float min_v, float max_v);

#endif // VGA_DRIVER_H