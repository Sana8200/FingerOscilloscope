/**
 * vga_driver.h - Professional Oscilloscope VGA Display Driver
 * 
 * HP-style oscilloscope display with green phosphor aesthetic
 * 320x240 resolution, 16-bit buffer (RGB332)
 */

#ifndef VGA_DRIVER_H
#define VGA_DRIVER_H

#include <stdint.h>

// ============================================================================
// Screen Configuration
// ============================================================================
#define SCREEN_WIDTH    320
#define SCREEN_HEIGHT   240

// ============================================================================
// VGA Memory-Mapped Interface
// ============================================================================
#define VGA_PIXEL_BUFFER_BASE   0x08000000
#define pVGA_PIXEL_BUFFER       ((volatile uint16_t *) VGA_PIXEL_BUFFER_BASE)

// ============================================================================
// Color Definitions (RGB332 format: RRR GGG BB)
// ============================================================================

// Basic colors
#define COLOR_BLACK         0x00
#define COLOR_WHITE         0xFF
#define COLOR_RED           0xE0
#define COLOR_GREEN         0x1C
#define COLOR_BLUE          0x03
#define COLOR_YELLOW        0xFC
#define COLOR_CYAN          0x1F
#define COLOR_MAGENTA       0xE3

// Grayscale
#define COLOR_DARK_GRAY     0x49
#define COLOR_LIGHT_GRAY    0x92

// Oscilloscope-specific colors (green phosphor)
#define COLOR_GRID_BLUE     0x08    // Subtle grid dots
#define COLOR_WAVEFORM      0x1C    // Bright green waveform
#define COLOR_TRIGGER       0xE0    // Red trigger line

// ============================================================================
// Basic Drawing Functions
// ============================================================================

void vga_clear_screen(uint16_t color);
void vga_draw_pixel(int x, int y, uint16_t color);
void vga_draw_line(int x1, int y1, int x2, int y2, uint16_t color);
void vga_draw_box_outline(int x, int y, int width, int height, uint16_t color);
void vga_draw_filled_box(int x, int y, int width, int height, uint16_t color);


// ============================================================================
// Oscilloscope Display Functions
// ============================================================================

/**
 * Draw the oscilloscope graticule grid
 * 10x8 divisions with center crosshairs and tick marks
 */
void vga_draw_grid(void);

/**
 * Draw the information header (channel, V/div, mode, RUN/STOP)
 */
void vga_draw_header(void);

/**
 * Draw the footer menu bar (HP scope style)
 */
void vga_draw_footer(void);

/**
 * Initialize complete oscilloscope display
 */
void vga_scope_init(void);

/**
 * Clear only the waveform area (preserves header/footer/grid)
 */
void vga_clear_waveform_area(void);

/**
 * Draw waveform segment between two ADC samples
 * @param x1, x2: Screen X coordinates
 * @param y1_adc, y2_adc: ADC values (0-65535)
 * @param color: Waveform color
 */
void vga_draw_waveform_segment(int x1, uint16_t y1_adc, int x2, uint16_t y2_adc, uint16_t color);

/**
 * Erase column and restore grid at that position
 */
void vga_erase_column(int x);

/**
 * Convert ADC value to screen Y coordinate
 */
int vga_adc_to_screen_y(uint16_t adc_value);

/**
 * Get graticule area boundaries
 */
void vga_get_waveform_bounds(int *top, int *bottom, int *left, int *right);

/**
 * Update oscilloscope info and redraw header
 */
void vga_scope_update_info(uint8_t channel, float voltage, float v_per_div,
                           float time_per_div, float v_max, float v_min);

/**
 * Set trigger level
 */
void vga_scope_set_trigger(uint16_t level);

/**
 * Set frequency display
 */
void vga_scope_set_frequency(float freq);

/**
 * Set RUN/STOP state
 */
void vga_scope_set_running(uint8_t running);

/**
 * Set display mode (XY, YT, etc.)
 */
void vga_scope_set_mode(const char *mode);

// ============================================================================
// Utility
// ============================================================================
int abs(int n);

#endif // VGA_DRIVER_H