/**
 * vga_driver.h - Professional Oscilloscope VGA Display Driver
 * 
 * Provides a complete oscilloscope-style display with:
 * - Information header (channel, voltage, V/div)
 * - Graticule grid with major/minor divisions
 * - Waveform rendering
 * - Footer with time base and trigger info
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

// Oscilloscope-specific colors
#define COLOR_GRID_BLUE     0x05    // Subtle grid color
#define COLOR_WAVEFORM      0x1C    // Bright green waveform
#define COLOR_TRIGGER       0xE0    // Red trigger line

// ============================================================================
// Basic Drawing Functions
// ============================================================================

/**
 * Clear entire screen with a color
 */
void vga_clear_screen(uint16_t color);

/**
 * Draw a single pixel
 */
void vga_draw_pixel(int x, int y, uint16_t color);

/**
 * Draw a line using Bresenham's algorithm
 */
void vga_draw_line(int x1, int y1, int x2, int y2, uint16_t color);

/**
 * Draw a rectangle outline
 */
void vga_draw_box_outline(int x, int y, int width, int height, uint16_t color);

/**
 * Draw a filled rectangle
 */
void vga_draw_filled_box(int x, int y, int width, int height, uint16_t color);

// ============================================================================
// Text Drawing Functions
// ============================================================================

/**
 * Draw a single character (5x7 font)
 */
void vga_draw_char(int x, int y, char c, uint16_t color);

/**
 * Draw a null-terminated string
 */
void vga_draw_string(int x, int y, const char *str, uint16_t color);

/**
 * Draw an integer value
 */
void vga_draw_int(int x, int y, int value, uint16_t color);

/**
 * Draw a floating point value
 * @param decimals: Number of decimal places to show
 */
void vga_draw_float(int x, int y, float value, int decimals, uint16_t color);

// ============================================================================
// Oscilloscope Grid Functions
// ============================================================================

/**
 * Draw the oscilloscope graticule grid
 * Includes major divisions, minor dots, and center crosshairs
 */
void vga_draw_grid(void);

/**
 * Draw the information header
 */
void vga_draw_header(void);

/**
 * Draw the footer with time base info
 */
void vga_draw_footer(void);

/**
 * Draw the trigger level indicator
 */
void vga_draw_trigger_line(void);

// ============================================================================
// Waveform Drawing Functions
// ============================================================================

/**
 * Clear only the waveform display area (keeps header/footer)
 */
void vga_clear_waveform_area(void);

/**
 * Draw a line segment between two waveform samples
 * @param prev_x: Previous X screen coordinate
 * @param prev_y: Previous ADC value (0-65535)
 * @param curr_x: Current X screen coordinate
 * @param curr_y: Current ADC value (0-65535)
 * @param color: Waveform color
 */
void vga_draw_waveform_segment(int prev_x, uint16_t prev_y, int curr_x, uint16_t curr_y, uint16_t color);

/**
 * Erase a vertical column and redraw grid at that position
 * Used for scrolling waveform display
 */
void vga_erase_column(int x);

// ============================================================================
// High-Level Oscilloscope Functions
// ============================================================================

/**
 * Initialize the complete oscilloscope display
 * Draws header, grid, footer, and trigger line
 */
void vga_scope_init(void);

/**
 * Update oscilloscope info and redraw header
 * @param channel: Current channel (1 or 2)
 * @param voltage: Current voltage reading
 * @param v_per_div: Volts per division setting
 * @param time_per_div: Time per division (ms)
 * @param v_max: Maximum voltage seen
 * @param v_min: Minimum voltage seen
 */
void vga_scope_update_info(uint8_t channel, float voltage, float v_per_div,
                           float time_per_div, float v_max, float v_min);

/**
 * Set the trigger level
 * @param level: Trigger level in ADC units (0-65535)
 */
void vga_scope_set_trigger(uint16_t level);

/**
 * Set the estimated frequency display
 * @param freq: Frequency in Hz
 */
void vga_scope_set_frequency(float freq);

/**
 * Convert ADC value to screen Y coordinate
 * @param adc_value: 16-bit ADC reading
 * @return: Screen Y coordinate in waveform area
 */
int vga_adc_to_screen_y(uint16_t adc_value);

/**
 * Get waveform area boundaries
 * @param top: Pointer to store top Y coordinate
 * @param bottom: Pointer to store bottom Y coordinate
 * @param left: Pointer to store left X coordinate
 * @param right: Pointer to store right X coordinate
 */
void vga_get_waveform_bounds(int *top, int *bottom, int *left, int *right);

// ============================================================================
// Utility
// ============================================================================
int abs(int n);

#endif // VGA_DRIVER_H