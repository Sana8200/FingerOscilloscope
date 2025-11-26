/**
 * vga_scope.h - Tektronix-Style Oscilloscope Display
 * 
 * Clean, professional oscilloscope UI for DE10-Lite
 * Resolution: 320x240, 8-bit color (RGB332)
 * 
 * Layout matches classic Tek scopes:
 * - Top status bar with Run/Stop and Trigger status
 * - 10x8 division graticule with dotted grid
 * - Bottom info bar with channel/timebase settings
 * - Ground reference markers and trigger level indicator
 */

#ifndef VGA_SCOPE_H
#define VGA_SCOPE_H

#include <stdint.h>
#include <stdbool.h>

// =============================================================================
// Display Configuration
// =============================================================================

#define SCREEN_WIDTH    320
#define SCREEN_HEIGHT   240

// Memory-mapped VGA buffer
#define VGA_BASE        0x08000000
#define VGA_BUFFER      ((volatile uint16_t *)VGA_BASE)

// =============================================================================
// Color Palette (RGB332: RRR GGG BB)
// =============================================================================

#define COL_BLACK       0x00
#define COL_WHITE       0xFF
#define COL_GRAY        0x92    // Medium gray for dim text
#define COL_DARK_GRAY   0x49    // Dark gray for grid

// Waveform colors (matching Tek)
#define COL_CH1         0xFC    // Yellow (Ch1 waveform)
#define COL_CH2         0xE3    // Magenta/Red (Ch2 waveform)

// UI colors
#define COL_GREEN       0x1C    // Run indicator, Trig'd
#define COL_RED         0xE0    // Stop indicator
#define COL_CYAN        0x1F    // Secondary info

// Grid colors
#define COL_GRID_DIM    0x24    // Dotted grid lines
#define COL_GRID_BRIGHT 0x49    // Major lines, center cross

// =============================================================================
// Screen Layout Constants
// =============================================================================

// Status bar (top)
#define STATUS_BAR_Y    0
#define STATUS_BAR_H    10

// Graticule (waveform area)
#define GRAT_X          0
#define GRAT_Y          10
#define GRAT_W          320
#define GRAT_H          200

// Divisions (standard 10x8 scope grid)
#define DIV_X           10
#define DIV_Y           8

// Info bar (bottom)
#define INFO_BAR_Y      210
#define INFO_BAR_H      30

// =============================================================================
// Scope State Structure
// =============================================================================

typedef struct {
    // Run state
    bool running;           // true = Run, false = Stop
    bool triggered;         // true = Trig'd, false = Ready
    
    // Channel 1 settings
    bool ch1_enabled;
    float ch1_vdiv;         // Volts per division
    int ch1_coupling;       // 0=DC, 1=AC, 2=GND
    int ch1_y_offset;       // Vertical position (pixels from center)
    
    // Channel 2 settings  
    bool ch2_enabled;
    float ch2_vdiv;
    int ch2_coupling;
    int ch2_y_offset;
    
    // Timebase
    float time_div_ms;      // Time per division in ms (or µs)
    bool time_is_us;        // true if time_div is in µs
    
    // Trigger
    int trig_channel;       // 1 or 2
    float trig_level_mv;    // Trigger level in mV
    int trig_y_pos;         // Trigger marker Y position
    
    // Measurements
    float ch1_vpp;          // Peak-to-peak voltage
    float ch2_vpp;
    
    // Horizontal position (0-100, 50 = center)
    int horiz_pos;
    
} ScopeState;

// Global scope state
extern ScopeState g_scope;

// =============================================================================
// Core Drawing Functions
// =============================================================================

// Pixel operations
void vga_put_pixel(int x, int y, uint8_t color);
uint8_t vga_get_pixel(int x, int y);
void vga_clear(uint8_t color);

// Primitives
void vga_hline(int x1, int x2, int y, uint8_t color);
void vga_vline(int x, int y1, int y2, uint8_t color);
void vga_line(int x1, int y1, int x2, int y2, uint8_t color);
void vga_rect(int x, int y, int w, int h, uint8_t color);
void vga_rect_fill(int x, int y, int w, int h, uint8_t color);

// Text (5x7 font)
void vga_char(int x, int y, char c, uint8_t color);
void vga_text(int x, int y, const char *str, uint8_t color);
void vga_int(int x, int y, int value, uint8_t color);
void vga_float(int x, int y, float value, int decimals, uint8_t color);

// =============================================================================
// Oscilloscope UI Functions
// =============================================================================

// Full screen setup
void scope_init(void);
void scope_redraw_all(void);

// Individual UI elements
void scope_draw_status_bar(void);
void scope_draw_graticule(void);
void scope_draw_info_bar(void);
void scope_draw_ground_markers(void);
void scope_draw_trigger_marker(void);

// Waveform drawing
void scope_clear_waveform(void);
void scope_draw_point(int x, uint16_t adc_value, uint8_t channel);
void scope_draw_segment(int x1, uint16_t adc1, int x2, uint16_t adc2, uint8_t channel);
void scope_erase_column(int x);

// =============================================================================
// Coordinate Helpers
// =============================================================================

// Convert ADC value (0-65535) to screen Y coordinate
int scope_adc_to_y(uint16_t adc_value, uint8_t channel);

// Get waveform drawing bounds
int scope_get_left(void);
int scope_get_right(void);
int scope_get_top(void);
int scope_get_bottom(void);

// =============================================================================
// State Update Functions
// =============================================================================

void scope_set_running(bool running);
void scope_set_triggered(bool triggered);
void scope_set_ch1_vdiv(float vdiv);
void scope_set_ch2_vdiv(float vdiv);
void scope_set_timebase(float time_div, bool is_microseconds);
void scope_set_trigger_level(float level_mv);
void scope_set_measurements(float ch1_vpp, float ch2_vpp);

#endif // VGA_SCOPE_H


