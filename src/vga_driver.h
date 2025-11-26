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

// Colors (RGB332)
#define COLOR_BLACK         0x00
#define COLOR_WHITE         0xFF
#define COLOR_RED           0xE0
#define COLOR_GREEN         0x1C
#define COLOR_BLUE          0x03
#define COLOR_YELLOW        0xFC
#define COLOR_CYAN          0x1F
#define COLOR_WAVEFORM      0xFC    // Yellow for CH1

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