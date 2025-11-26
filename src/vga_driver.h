/**
 * vga_driver.h - Tektronix-Style Oscilloscope VGA Display
 * DE10-Lite, 320x240, 16-bit buffer (RGB332)
 */

#ifndef VGA_DRIVER_H
#define VGA_DRIVER_H

#include <stdint.h>

// Screen
#define SCREEN_WIDTH    320
#define SCREEN_HEIGHT   240

// VGA buffer
#define VGA_PIXEL_BUFFER_BASE   0x08000000
#define pVGA_PIXEL_BUFFER       ((volatile uint16_t *) VGA_PIXEL_BUFFER_BASE)

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


// Basic drawing
void vga_clear_screen(uint16_t color);
void vga_draw_pixel(int x, int y, uint16_t color);
void vga_draw_line(int x1, int y1, int x2, int y2, uint16_t color);
void vga_draw_filled_box(int x, int y, int w, int h, uint16_t color);
void vga_draw_box_outline(int x, int y, int w, int h, uint16_t color);

// Text
void vga_draw_char(int x, int y, char c, uint16_t color);
void vga_draw_string(int x, int y, const char *str, uint16_t color);
void vga_draw_int(int x, int y, int value, uint16_t color);

// Oscilloscope
void vga_draw_grid(void);
void vga_draw_header(void);
void vga_draw_footer(void);
void vga_scope_init(void);

void vga_clear_waveform_area(void);
void vga_draw_waveform_segment(int x1, uint16_t y1_adc, int x2, uint16_t y2_adc, uint16_t color);
void vga_erase_column(int x);
int vga_adc_to_screen_y(uint16_t adc_value);
void vga_get_waveform_bounds(int *top, int *bottom, int *left, int *right);

void vga_scope_update_info(uint8_t channel, float voltage, float v_per_div,
                           float time_per_div, float v_max, float v_min);
void vga_scope_set_trigger(uint16_t level);
void vga_scope_set_frequency(float freq);
void vga_scope_set_running(uint8_t running);
void vga_scope_set_channel(int ch, int enabled);

int abs(int n);

#endif