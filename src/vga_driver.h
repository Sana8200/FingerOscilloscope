// VGA driver 320x240 pixels, 8-bit color (RGB332)
#ifndef VGA_DRIVER_H
#define VGA_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

#define SCREEN_WIDTH    320
#define SCREEN_HEIGHT   240

// VGA buffer memory address (where pixels stored)  
#define VGA_BUFFER_BASE     0x08000000
// pixel buffer = 1D array :  [SCREEN_WIDTH * SCREEN_HEIGHT]
#define pVGA_PIXEL_BUFFER   ((volatile uint8_t *) VGA_BUFFER_BASE)   

// Osilliscope layout where to draw the grid and waveform
// Margins (empty spaces) around the the graph 
#define LEFT_MARGIN     30
#define RIGHT_MARGIN    1
#define TOP_MARGIN      20
#define BOTTOM_MARGIN   15
// Graph area (calculated from margins)(the box where waveform is drawn)
#define GRAPH_X     LEFT_MARGIN    
#define GRAPH_Y     TOP_MARGIN
#define GRAPH_W     (SCREEN_WIDTH - LEFT_MARGIN - RIGHT_MARGIN)   // width = 285 pixels
#define GRAPH_H     (SCREEN_HEIGHT - TOP_MARGIN - BOTTOM_MARGIN)  // height = 205 pixels 


#define GRID_LINE_SIZE  ((GRAPH_W / (GRAPH_W / GRID_DIVS_X)) + (GRAPH_H / (GRAPH_H / GRID_DIVS_Y)))


// Grid Divisions (lines inside the box)
#define GRID_DIV_X     20
#define GRID_DIV_Y     15


// COLORS (RGB332 format: RRRGGGBB) 
// Red:   bits 7-5 (3 bits, values 0-7)
// Green: bits 4-2 (3 bits, values 0-7)
// Blue:  bits 1-0 (2 bits, values 0-3)
#define COLOR_BLACK      0x00    // 000 000 00
#define COLOR_WHITE      0xFF    // 111 111 11
#define COLOR_RED        0xE0    // 111 000 00
#define COLOR_GREEN      0x1C    // 000 111 00
#define COLOR_BLUE       0x03    // 000 000 11
#define COLOR_YELLOW     0xFC    // 111 111 00
#define COLOR_CYAN       0x1F    // 000 111 11
#define COLOR_ORANGE     0xEC    // 111 011 00 
#define COLOR_GRID       0x49    // 010 010 01 
#define COLOR_DARK_RED   0x40
#define COLOR_DARK_GREEN 0x08


// Basic draawing functions 
void vga_clear_screen(uint8_t color);
void vga_draw_pixel(int x, int y, uint8_t color);
void vga_draw_line(int x0, int y0, int x1, int y1, uint8_t color, bool dashed);

// Text drawing functions 
void vga_draw_char(int x, int y, char c, uint8_t color);
void vga_draw_string(int x, int y, const char *str, uint8_t color);
void vga_draw_int(int x, int y, int value, uint8_t color);
void vga_draw_float(int x, int y, float value, uint8_t color);

// Initial setup
void vga_init_scope(int gain, int sample_rate);

// Drawing the oscilloscope display
void vga_draw_grid(void);

void vga_draw_display_info(float voltage, float v_max, float v_min, int gain, int sample_rate);

// Updating the display
void vga_update_settings(int gain, int sample_rate);
void vga_clear_column(int x);


// Pause functionality display box 
void vga_show_paused(void);
void vga_hide_paused(void);

// freeze functionality display indicatro
void vga_show_freeze_indicator(void);
void vga_hide_freeze_indicator(void);

// Helper functions
int vga_voltage_to_y(float voltage, float v_min, float v_max);
void vga_get_graph_area(int *left, int *right, int *top, int *bottom);

// Drawing border rectangular and filled one
void vga_draw_rect(int x, int y, int w, int h, uint8_t color);
void vga_draw_filled_rect(int x, int y, int w, int h, uint8_t color);

#endif // VGA_DRIVER_H