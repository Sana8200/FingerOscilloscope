/**
 * vga_driver.c - Professional Oscilloscope VGA Display Driver
 * 
 * Features:
 * - Information header with voltage, channel, time/div display
 * - Professional graticule grid with major/minor divisions
 * - Smooth waveform rendering with anti-aliased lines
 * - Axis labels and measurement markers
 * 
 * Screen Layout (320x240):
 * +------------------------------------------+
 * |  HEADER (Channel, Voltage, Settings)     | 24px
 * +------------------------------------------+
 * |                                          |
 * |           WAVEFORM AREA                  | 200px
 * |         (with graticule grid)            |
 * |                                          |
 * +------------------------------------------+
 * |  FOOTER (Time/div, Trigger info)         | 16px
 * +------------------------------------------+
 */

#include "vga_driver.h"
#include "hardware.h"
#include <stdint.h>

// ============================================================================
// Layout Constants
// ============================================================================
#define HEADER_HEIGHT       24
#define FOOTER_HEIGHT       16
#define WAVEFORM_TOP        HEADER_HEIGHT
#define WAVEFORM_HEIGHT     (SCREEN_HEIGHT - HEADER_HEIGHT - FOOTER_HEIGHT)
#define WAVEFORM_BOTTOM     (HEADER_HEIGHT + WAVEFORM_HEIGHT - 1)

// Grid configuration
#define GRID_DIVISIONS_X    10      // 10 horizontal divisions
#define GRID_DIVISIONS_Y    8       // 8 vertical divisions
#define MINOR_GRID_DIVS     5       // 5 minor divisions per major

// ============================================================================
// Color Palette - Oscilloscope Style
// ============================================================================
// RGB332 format: RRR GGG BB

// Background colors
#define COLOR_BACKGROUND    0x00    // Pure black
#define COLOR_HEADER_BG     0x24    // Dark blue-gray
#define COLOR_FOOTER_BG     0x24    // Dark blue-gray

// Grid colors
#define COLOR_GRID_MAJOR    0x24    // Visible but subtle gray
#define COLOR_GRID_MINOR    0x12    // Very dark, subtle dots
#define COLOR_AXIS_CENTER   0x49    // Brighter center lines

// Waveform colors
#define COLOR_WAVEFORM_1    0x1C    // Bright green (CH1)
#define COLOR_WAVEFORM_2    0xFC    // Yellow (CH2)
#define COLOR_TRIGGER       0xE0    // Red trigger line

// Text/UI colors
#define COLOR_TEXT_WHITE    0xFF    // White text
#define COLOR_TEXT_GREEN    0x1C    // Green text
#define COLOR_TEXT_YELLOW   0xFC    // Yellow text
#define COLOR_TEXT_CYAN     0x1F    // Cyan text
#define COLOR_TEXT_DIM      0x92    // Dim gray text

// Measurement colors
#define COLOR_CURSOR        0xE3    // Magenta cursors
#define COLOR_MEASURE       0xFC    // Yellow measurements

// ============================================================================
// 5x7 Font Data - Compact bitmap font for oscilloscope display
// Each character is 5 pixels wide, 7 pixels tall
// Stored as 5 bytes per character (one byte per column)
// ============================================================================
static const uint8_t font_5x7[][5] = {
    // Space (32)
    {0x00, 0x00, 0x00, 0x00, 0x00},
    // ! (33)
    {0x00, 0x00, 0x5F, 0x00, 0x00},
    // " (34)
    {0x00, 0x07, 0x00, 0x07, 0x00},
    // # (35)
    {0x14, 0x7F, 0x14, 0x7F, 0x14},
    // $ (36)
    {0x24, 0x2A, 0x7F, 0x2A, 0x12},
    // % (37)
    {0x23, 0x13, 0x08, 0x64, 0x62},
    // & (38)
    {0x36, 0x49, 0x55, 0x22, 0x50},
    // ' (39)
    {0x00, 0x05, 0x03, 0x00, 0x00},
    // ( (40)
    {0x00, 0x1C, 0x22, 0x41, 0x00},
    // ) (41)
    {0x00, 0x41, 0x22, 0x1C, 0x00},
    // * (42)
    {0x14, 0x08, 0x3E, 0x08, 0x14},
    // + (43)
    {0x08, 0x08, 0x3E, 0x08, 0x08},
    // , (44)
    {0x00, 0x50, 0x30, 0x00, 0x00},
    // - (45)
    {0x08, 0x08, 0x08, 0x08, 0x08},
    // . (46)
    {0x00, 0x60, 0x60, 0x00, 0x00},
    // / (47)
    {0x20, 0x10, 0x08, 0x04, 0x02},
    // 0 (48)
    {0x3E, 0x51, 0x49, 0x45, 0x3E},
    // 1 (49)
    {0x00, 0x42, 0x7F, 0x40, 0x00},
    // 2 (50)
    {0x42, 0x61, 0x51, 0x49, 0x46},
    // 3 (51)
    {0x21, 0x41, 0x45, 0x4B, 0x31},
    // 4 (52)
    {0x18, 0x14, 0x12, 0x7F, 0x10},
    // 5 (53)
    {0x27, 0x45, 0x45, 0x45, 0x39},
    // 6 (54)
    {0x3C, 0x4A, 0x49, 0x49, 0x30},
    // 7 (55)
    {0x01, 0x71, 0x09, 0x05, 0x03},
    // 8 (56)
    {0x36, 0x49, 0x49, 0x49, 0x36},
    // 9 (57)
    {0x06, 0x49, 0x49, 0x29, 0x1E},
    // : (58)
    {0x00, 0x36, 0x36, 0x00, 0x00},
    // ; (59)
    {0x00, 0x56, 0x36, 0x00, 0x00},
    // < (60)
    {0x08, 0x14, 0x22, 0x41, 0x00},
    // = (61)
    {0x14, 0x14, 0x14, 0x14, 0x14},
    // > (62)
    {0x00, 0x41, 0x22, 0x14, 0x08},
    // ? (63)
    {0x02, 0x01, 0x51, 0x09, 0x06},
    // @ (64)
    {0x32, 0x49, 0x79, 0x41, 0x3E},
    // A (65)
    {0x7E, 0x11, 0x11, 0x11, 0x7E},
    // B (66)
    {0x7F, 0x49, 0x49, 0x49, 0x36},
    // C (67)
    {0x3E, 0x41, 0x41, 0x41, 0x22},
    // D (68)
    {0x7F, 0x41, 0x41, 0x22, 0x1C},
    // E (69)
    {0x7F, 0x49, 0x49, 0x49, 0x41},
    // F (70)
    {0x7F, 0x09, 0x09, 0x09, 0x01},
    // G (71)
    {0x3E, 0x41, 0x49, 0x49, 0x7A},
    // H (72)
    {0x7F, 0x08, 0x08, 0x08, 0x7F},
    // I (73)
    {0x00, 0x41, 0x7F, 0x41, 0x00},
    // J (74)
    {0x20, 0x40, 0x41, 0x3F, 0x01},
    // K (75)
    {0x7F, 0x08, 0x14, 0x22, 0x41},
    // L (76)
    {0x7F, 0x40, 0x40, 0x40, 0x40},
    // M (77)
    {0x7F, 0x02, 0x0C, 0x02, 0x7F},
    // N (78)
    {0x7F, 0x04, 0x08, 0x10, 0x7F},
    // O (79)
    {0x3E, 0x41, 0x41, 0x41, 0x3E},
    // P (80)
    {0x7F, 0x09, 0x09, 0x09, 0x06},
    // Q (81)
    {0x3E, 0x41, 0x51, 0x21, 0x5E},
    // R (82)
    {0x7F, 0x09, 0x19, 0x29, 0x46},
    // S (83)
    {0x46, 0x49, 0x49, 0x49, 0x31},
    // T (84)
    {0x01, 0x01, 0x7F, 0x01, 0x01},
    // U (85)
    {0x3F, 0x40, 0x40, 0x40, 0x3F},
    // V (86)
    {0x1F, 0x20, 0x40, 0x20, 0x1F},
    // W (87)
    {0x3F, 0x40, 0x38, 0x40, 0x3F},
    // X (88)
    {0x63, 0x14, 0x08, 0x14, 0x63},
    // Y (89)
    {0x07, 0x08, 0x70, 0x08, 0x07},
    // Z (90)
    {0x61, 0x51, 0x49, 0x45, 0x43},
    // [ (91)
    {0x00, 0x7F, 0x41, 0x41, 0x00},
    // \ (92)
    {0x02, 0x04, 0x08, 0x10, 0x20},
    // ] (93)
    {0x00, 0x41, 0x41, 0x7F, 0x00},
    // ^ (94)
    {0x04, 0x02, 0x01, 0x02, 0x04},
    // _ (95)
    {0x40, 0x40, 0x40, 0x40, 0x40},
    // ` (96)
    {0x00, 0x01, 0x02, 0x04, 0x00},
    // a (97)
    {0x20, 0x54, 0x54, 0x54, 0x78},
    // b (98)
    {0x7F, 0x48, 0x44, 0x44, 0x38},
    // c (99)
    {0x38, 0x44, 0x44, 0x44, 0x20},
    // d (100)
    {0x38, 0x44, 0x44, 0x48, 0x7F},
    // e (101)
    {0x38, 0x54, 0x54, 0x54, 0x18},
    // f (102)
    {0x08, 0x7E, 0x09, 0x01, 0x02},
    // g (103)
    {0x0C, 0x52, 0x52, 0x52, 0x3E},
    // h (104)
    {0x7F, 0x08, 0x04, 0x04, 0x78},
    // i (105)
    {0x00, 0x44, 0x7D, 0x40, 0x00},
    // j (106)
    {0x20, 0x40, 0x44, 0x3D, 0x00},
    // k (107)
    {0x7F, 0x10, 0x28, 0x44, 0x00},
    // l (108)
    {0x00, 0x41, 0x7F, 0x40, 0x00},
    // m (109)
    {0x7C, 0x04, 0x18, 0x04, 0x78},
    // n (110)
    {0x7C, 0x08, 0x04, 0x04, 0x78},
    // o (111)
    {0x38, 0x44, 0x44, 0x44, 0x38},
    // p (112)
    {0x7C, 0x14, 0x14, 0x14, 0x08},
    // q (113)
    {0x08, 0x14, 0x14, 0x18, 0x7C},
    // r (114)
    {0x7C, 0x08, 0x04, 0x04, 0x08},
    // s (115)
    {0x48, 0x54, 0x54, 0x54, 0x20},
    // t (116)
    {0x04, 0x3F, 0x44, 0x40, 0x20},
    // u (117)
    {0x3C, 0x40, 0x40, 0x20, 0x7C},
    // v (118)
    {0x1C, 0x20, 0x40, 0x20, 0x1C},
    // w (119)
    {0x3C, 0x40, 0x30, 0x40, 0x3C},
    // x (120)
    {0x44, 0x28, 0x10, 0x28, 0x44},
    // y (121)
    {0x0C, 0x50, 0x50, 0x50, 0x3C},
    // z (122)
    {0x44, 0x64, 0x54, 0x4C, 0x44},
};

// ============================================================================
// Oscilloscope State (for display updates)
// ============================================================================
static struct {
    uint8_t channel;            // Current channel (1 or 2)
    float voltage;              // Current voltage reading
    float v_per_div;            // Volts per division
    float time_per_div;         // Time per division (ms)
    uint16_t trigger_level;     // Trigger level (0-65535)
    float v_max;                // Max voltage seen
    float v_min;                // Min voltage seen
    float v_pp;                 // Peak-to-peak voltage
    float frequency;            // Estimated frequency
} scope_state = {
    .channel = 1,
    .voltage = 0.0f,
    .v_per_div = 0.5f,
    .time_per_div = 10.0f,
    .trigger_level = 32768,
    .v_max = 0.0f,
    .v_min = 3.3f,
    .v_pp = 0.0f,
    .frequency = 0.0f
};

// ============================================================================
// Utility Functions
// ============================================================================

int abs(int n) {
    return (n < 0) ? -n : n;
}

// ============================================================================
// Basic Drawing Functions
// ============================================================================

void vga_clear_screen(uint16_t color) {
    int total_pixels = SCREEN_WIDTH * SCREEN_HEIGHT;
    for (int i = 0; i < total_pixels; i++) {
        pVGA_PIXEL_BUFFER[i] = color;
    }
}

void vga_draw_pixel(int x, int y, uint16_t color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        pVGA_PIXEL_BUFFER[y * SCREEN_WIDTH + x] = color;
    }
}

// Fast horizontal line (optimized)
static void vga_hline(int x1, int x2, int y, uint16_t color) {
    if (y < 0 || y >= SCREEN_HEIGHT) return;
    if (x1 > x2) { int t = x1; x1 = x2; x2 = t; }
    if (x1 < 0) x1 = 0;
    if (x2 >= SCREEN_WIDTH) x2 = SCREEN_WIDTH - 1;
    
    volatile uint16_t *ptr = &pVGA_PIXEL_BUFFER[y * SCREEN_WIDTH + x1];
    for (int x = x1; x <= x2; x++) {
        *ptr++ = color;
    }
}

// Fast vertical line (optimized)
static void vga_vline(int x, int y1, int y2, uint16_t color) {
    if (x < 0 || x >= SCREEN_WIDTH) return;
    if (y1 > y2) { int t = y1; y1 = y2; y2 = t; }
    if (y1 < 0) y1 = 0;
    if (y2 >= SCREEN_HEIGHT) y2 = SCREEN_HEIGHT - 1;
    
    volatile uint16_t *ptr = &pVGA_PIXEL_BUFFER[y1 * SCREEN_WIDTH + x];
    for (int y = y1; y <= y2; y++) {
        *ptr = color;
        ptr += SCREEN_WIDTH;
    }
}

void vga_draw_line(int x1, int y1, int x2, int y2, uint16_t color) {
    // Optimized cases for horizontal and vertical lines
    if (y1 == y2) {
        vga_hline(x1, x2, y1, color);
        return;
    }
    if (x1 == x2) {
        vga_vline(x1, y1, y2, color);
        return;
    }
    
    // Bresenham's algorithm for diagonal lines
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        vga_draw_pixel(x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 < dx)  { err += dx; y1 += sy; }
    }
}

void vga_draw_box_outline(int x, int y, int width, int height, uint16_t color) {
    vga_hline(x, x + width - 1, y, color);              // Top
    vga_hline(x, x + width - 1, y + height - 1, color); // Bottom
    vga_vline(x, y, y + height - 1, color);             // Left
    vga_vline(x + width - 1, y, y + height - 1, color); // Right
}

void vga_draw_filled_box(int x, int y, int width, int height, uint16_t color) {
    for (int row = y; row < y + height; row++) {
        vga_hline(x, x + width - 1, row, color);
    }
}

// ============================================================================
// Text Drawing Functions
// ============================================================================

/**
 * Draw a single character at position (x, y)
 * @param x: X position (left edge)
 * @param y: Y position (top edge)
 * @param c: Character to draw
 * @param color: Text color
 */
void vga_draw_char(int x, int y, char c, uint16_t color) {
    // Handle printable ASCII range (32-122)
    if (c < 32 || c > 122) c = '?';
    int index = c - 32;
    
    const uint8_t *glyph = font_5x7[index];
    
    for (int col = 0; col < 5; col++) {
        uint8_t column_data = glyph[col];
        for (int row = 0; row < 7; row++) {
            if (column_data & (1 << row)) {
                vga_draw_pixel(x + col, y + row, color);
            }
        }
    }
}

/**
 * Draw a string at position (x, y)
 * @param x: X position
 * @param y: Y position  
 * @param str: Null-terminated string
 * @param color: Text color
 */
void vga_draw_string(int x, int y, const char *str, uint16_t color) {
    while (*str) {
        vga_draw_char(x, y, *str, color);
        x += 6;  // 5 pixels + 1 pixel spacing
        str++;
    }
}

/**
 * Draw an integer value
 */
void vga_draw_int(int x, int y, int value, uint16_t color) {
    char buf[12];
    int i = 0;
    int negative = 0;
    
    if (value < 0) {
        negative = 1;
        value = -value;
    }
    
    if (value == 0) {
        buf[i++] = '0';
    } else {
        while (value > 0) {
            buf[i++] = '0' + (value % 10);
            value /= 10;
        }
    }
    
    if (negative) buf[i++] = '-';
    
    // Reverse and draw
    while (i > 0) {
        vga_draw_char(x, y, buf[--i], color);
        x += 6;
    }
}

/**
 * Draw a float value with specified decimal places
 */
void vga_draw_float(int x, int y, float value, int decimals, uint16_t color) {
    char buf[16];
    int idx = 0;
    
    if (value < 0) {
        buf[idx++] = '-';
        value = -value;
    }
    
    int integer_part = (int)value;
    float frac_part = value - integer_part;
    
    // Integer part
    char int_buf[8];
    int int_idx = 0;
    if (integer_part == 0) {
        int_buf[int_idx++] = '0';
    } else {
        while (integer_part > 0) {
            int_buf[int_idx++] = '0' + (integer_part % 10);
            integer_part /= 10;
        }
    }
    while (int_idx > 0) {
        buf[idx++] = int_buf[--int_idx];
    }
    
    // Decimal point and fraction
    if (decimals > 0) {
        buf[idx++] = '.';
        for (int d = 0; d < decimals; d++) {
            frac_part *= 10;
            int digit = (int)frac_part;
            buf[idx++] = '0' + digit;
            frac_part -= digit;
        }
    }
    
    buf[idx] = '\0';
    vga_draw_string(x, y, buf, color);
}

// ============================================================================
// Oscilloscope Grid Drawing
// ============================================================================

/**
 * Draw the professional oscilloscope graticule grid
 * Includes major divisions, minor tick marks, and center axis highlights
 */
void vga_draw_grid(void) {
    int grid_left = 0;
    int grid_right = SCREEN_WIDTH - 1;
    int grid_top = WAVEFORM_TOP;
    int grid_bottom = WAVEFORM_BOTTOM;
    int grid_width = SCREEN_WIDTH;
    int grid_height = WAVEFORM_HEIGHT;
    
    int div_width = grid_width / GRID_DIVISIONS_X;
    int div_height = grid_height / GRID_DIVISIONS_Y;
    int minor_width = div_width / MINOR_GRID_DIVS;
    int minor_height = div_height / MINOR_GRID_DIVS;
    
    // Draw minor grid (dotted pattern for subtle effect)
    for (int x = grid_left; x <= grid_right; x += minor_width) {
        for (int y = grid_top; y <= grid_bottom; y += minor_height) {
            // Skip if this is a major grid position
            if ((x - grid_left) % div_width != 0 || (y - grid_top) % div_height != 0) {
                vga_draw_pixel(x, y, COLOR_GRID_MINOR);
            }
        }
    }
    
    // Draw major grid lines (dashed style)
    for (int i = 0; i <= GRID_DIVISIONS_X; i++) {
        int x = grid_left + i * div_width;
        if (x > grid_right) x = grid_right;
        
        // Draw dashed vertical line
        for (int y = grid_top; y <= grid_bottom; y += 4) {
            vga_draw_pixel(x, y, COLOR_GRID_MAJOR);
            if (y + 1 <= grid_bottom) vga_draw_pixel(x, y + 1, COLOR_GRID_MAJOR);
        }
    }
    
    for (int i = 0; i <= GRID_DIVISIONS_Y; i++) {
        int y = grid_top + i * div_height;
        if (y > grid_bottom) y = grid_bottom;
        
        // Draw dashed horizontal line
        for (int x = grid_left; x <= grid_right; x += 4) {
            vga_draw_pixel(x, y, COLOR_GRID_MAJOR);
            if (x + 1 <= grid_right) vga_draw_pixel(x + 1, y, COLOR_GRID_MAJOR);
        }
    }
    
    // Draw center crosshairs (brighter)
    int center_x = grid_left + grid_width / 2;
    int center_y = grid_top + grid_height / 2;
    
    // Horizontal center line with tick marks
    for (int x = grid_left; x <= grid_right; x++) {
        if (x % 2 == 0) {
            vga_draw_pixel(x, center_y, COLOR_AXIS_CENTER);
        }
    }
    
    // Vertical center line with tick marks  
    for (int y = grid_top; y <= grid_bottom; y++) {
        if (y % 2 == 0) {
            vga_draw_pixel(center_x, y, COLOR_AXIS_CENTER);
        }
    }
    
    // Draw tick marks on center lines
    for (int i = 0; i <= GRID_DIVISIONS_X; i++) {
        int x = grid_left + i * div_width;
        vga_draw_pixel(x, center_y - 2, COLOR_TEXT_WHITE);
        vga_draw_pixel(x, center_y - 1, COLOR_TEXT_WHITE);
        vga_draw_pixel(x, center_y + 1, COLOR_TEXT_WHITE);
        vga_draw_pixel(x, center_y + 2, COLOR_TEXT_WHITE);
    }
    
    for (int i = 0; i <= GRID_DIVISIONS_Y; i++) {
        int y = grid_top + i * div_height;
        vga_draw_pixel(center_x - 2, y, COLOR_TEXT_WHITE);
        vga_draw_pixel(center_x - 1, y, COLOR_TEXT_WHITE);
        vga_draw_pixel(center_x + 1, y, COLOR_TEXT_WHITE);
        vga_draw_pixel(center_x + 2, y, COLOR_TEXT_WHITE);
    }
    
    // Draw border around waveform area
    vga_draw_box_outline(grid_left, grid_top, grid_width, grid_height, COLOR_GRID_MAJOR);
}

// ============================================================================
// Header and Footer Drawing
// ============================================================================

/**
 * Draw the oscilloscope header with channel info and voltage reading
 */
void vga_draw_header(void) {
    // Clear header area
    vga_draw_filled_box(0, 0, SCREEN_WIDTH, HEADER_HEIGHT, COLOR_HEADER_BG);
    
    // Channel indicator (left side)
    vga_draw_string(4, 4, "CH", COLOR_TEXT_DIM);
    vga_draw_char(16, 4, '0' + scope_state.channel, COLOR_WAVEFORM_1);
    
    // Separator
    vga_draw_char(26, 4, '|', COLOR_TEXT_DIM);
    
    // Current voltage reading
    vga_draw_float(34, 4, scope_state.voltage, 2, COLOR_TEXT_GREEN);
    vga_draw_string(70, 4, "V", COLOR_TEXT_GREEN);
    
    // Separator
    vga_draw_char(82, 4, '|', COLOR_TEXT_DIM);
    
    // Volts per division
    vga_draw_float(90, 4, scope_state.v_per_div, 1, COLOR_TEXT_YELLOW);
    vga_draw_string(120, 4, "V/d", COLOR_TEXT_YELLOW);
    
    // Second row - measurements
    vga_draw_string(4, 14, "Vpp:", COLOR_TEXT_DIM);
    vga_draw_float(30, 14, scope_state.v_pp, 2, COLOR_TEXT_CYAN);
    vga_draw_string(66, 14, "V", COLOR_TEXT_CYAN);
    
    // Max/Min
    vga_draw_string(80, 14, "Max:", COLOR_TEXT_DIM);
    vga_draw_float(110, 14, scope_state.v_max, 1, COLOR_TEXT_WHITE);
    
    vga_draw_string(150, 14, "Min:", COLOR_TEXT_DIM);
    vga_draw_float(180, 14, scope_state.v_min, 1, COLOR_TEXT_WHITE);
    
    // Frequency (if calculated)
    if (scope_state.frequency > 0) {
        vga_draw_string(220, 4, "f:", COLOR_TEXT_DIM);
        if (scope_state.frequency >= 1000) {
            vga_draw_float(234, 4, scope_state.frequency / 1000.0f, 1, COLOR_TEXT_CYAN);
            vga_draw_string(276, 4, "kHz", COLOR_TEXT_CYAN);
        } else {
            vga_draw_float(234, 4, scope_state.frequency, 0, COLOR_TEXT_CYAN);
            vga_draw_string(270, 4, "Hz", COLOR_TEXT_CYAN);
        }
    }
    
    // Status indicators (right side)
    vga_draw_string(280, 4, "RUN", COLOR_TEXT_GREEN);
}

/**
 * Draw the oscilloscope footer with time base info
 */
void vga_draw_footer(void) {
    int footer_y = SCREEN_HEIGHT - FOOTER_HEIGHT;
    
    // Clear footer area
    vga_draw_filled_box(0, footer_y, SCREEN_WIDTH, FOOTER_HEIGHT, COLOR_FOOTER_BG);
    
    // Time per division
    vga_draw_string(4, footer_y + 4, "Time:", COLOR_TEXT_DIM);
    vga_draw_float(40, footer_y + 4, scope_state.time_per_div, 1, COLOR_TEXT_YELLOW);
    vga_draw_string(76, footer_y + 4, "ms/d", COLOR_TEXT_YELLOW);
    
    // Trigger level
    float trig_voltage = (scope_state.trigger_level / 65535.0f) * 3.3f;
    vga_draw_string(120, footer_y + 4, "Trig:", COLOR_TEXT_DIM);
    vga_draw_float(156, footer_y + 4, trig_voltage, 2, COLOR_TRIGGER);
    vga_draw_string(192, footer_y + 4, "V", COLOR_TRIGGER);
    
    // Input info
    vga_draw_string(220, footer_y + 4, "AIN1", COLOR_TEXT_GREEN);
    vga_draw_string(250, footer_y + 4, "DC", COLOR_TEXT_CYAN);
    
    // AD7705 indicator
    vga_draw_string(275, footer_y + 4, "AD7705", COLOR_TEXT_DIM);
}

// ============================================================================
// Waveform Drawing
// ============================================================================

/**
 * Draw the trigger level indicator line
 */
void vga_draw_trigger_line(void) {
    // Convert trigger level to Y coordinate
    float trig_normalized = scope_state.trigger_level / 65535.0f;
    int trig_y = WAVEFORM_BOTTOM - (int)(trig_normalized * WAVEFORM_HEIGHT);
    
    // Clamp to waveform area
    if (trig_y < WAVEFORM_TOP) trig_y = WAVEFORM_TOP;
    if (trig_y > WAVEFORM_BOTTOM) trig_y = WAVEFORM_BOTTOM;
    
    // Draw dashed trigger line
    for (int x = 0; x < SCREEN_WIDTH; x += 8) {
        vga_draw_pixel(x, trig_y, COLOR_TRIGGER);
        vga_draw_pixel(x + 1, trig_y, COLOR_TRIGGER);
        vga_draw_pixel(x + 2, trig_y, COLOR_TRIGGER);
    }
    
    // Draw trigger arrow on left edge
    vga_draw_pixel(0, trig_y - 2, COLOR_TRIGGER);
    vga_draw_pixel(0, trig_y - 1, COLOR_TRIGGER);
    vga_draw_pixel(0, trig_y, COLOR_TRIGGER);
    vga_draw_pixel(0, trig_y + 1, COLOR_TRIGGER);
    vga_draw_pixel(0, trig_y + 2, COLOR_TRIGGER);
    vga_draw_pixel(1, trig_y - 1, COLOR_TRIGGER);
    vga_draw_pixel(1, trig_y, COLOR_TRIGGER);
    vga_draw_pixel(1, trig_y + 1, COLOR_TRIGGER);
    vga_draw_pixel(2, trig_y, COLOR_TRIGGER);
}

/**
 * Clear only the waveform area (preserving header/footer)
 */
void vga_clear_waveform_area(void) {
    vga_draw_filled_box(0, WAVEFORM_TOP, SCREEN_WIDTH, WAVEFORM_HEIGHT, COLOR_BACKGROUND);
}

/**
 * Draw a single waveform sample connected to previous sample
 * @param prev_x: Previous X coordinate
 * @param prev_y: Previous Y value (in ADC units 0-65535)
 * @param curr_x: Current X coordinate  
 * @param curr_y: Current Y value (in ADC units 0-65535)
 * @param color: Waveform color
 */
void vga_draw_waveform_segment(int prev_x, uint16_t prev_y, int curr_x, uint16_t curr_y, uint16_t color) {
    // Convert ADC values to screen Y coordinates
    // ADC 0 = bottom of waveform area, ADC 65535 = top
    int y1 = WAVEFORM_BOTTOM - ((prev_y * WAVEFORM_HEIGHT) / 65536);
    int y2 = WAVEFORM_BOTTOM - ((curr_y * WAVEFORM_HEIGHT) / 65536);
    
    // Clamp to waveform area
    if (y1 < WAVEFORM_TOP) y1 = WAVEFORM_TOP;
    if (y1 > WAVEFORM_BOTTOM) y1 = WAVEFORM_BOTTOM;
    if (y2 < WAVEFORM_TOP) y2 = WAVEFORM_TOP;
    if (y2 > WAVEFORM_BOTTOM) y2 = WAVEFORM_BOTTOM;
    
    // Draw connected line
    vga_draw_line(prev_x, y1, curr_x, y2, color);
}

/**
 * Erase a vertical column in the waveform area (for scrolling display)
 * and redraw the grid at that column
 */
void vga_erase_column(int x) {
    if (x < 0 || x >= SCREEN_WIDTH) return;
    
    // Clear the column
    vga_vline(x, WAVEFORM_TOP, WAVEFORM_BOTTOM, COLOR_BACKGROUND);
    
    // Redraw grid elements at this column
    int div_width = SCREEN_WIDTH / GRID_DIVISIONS_X;
    int div_height = WAVEFORM_HEIGHT / GRID_DIVISIONS_Y;
    int minor_width = div_width / MINOR_GRID_DIVS;
    int minor_height = div_height / MINOR_GRID_DIVS;
    int center_y = WAVEFORM_TOP + WAVEFORM_HEIGHT / 2;
    int center_x = SCREEN_WIDTH / 2;
    
    // Minor grid dots
    if (x % minor_width == 0) {
        for (int y = WAVEFORM_TOP; y <= WAVEFORM_BOTTOM; y += minor_height) {
            if ((y - WAVEFORM_TOP) % div_height != 0) {
                vga_draw_pixel(x, y, COLOR_GRID_MINOR);
            }
        }
    }
    
    // Major grid line
    if (x % div_width == 0) {
        for (int y = WAVEFORM_TOP; y <= WAVEFORM_BOTTOM; y += 4) {
            vga_draw_pixel(x, y, COLOR_GRID_MAJOR);
            if (y + 1 <= WAVEFORM_BOTTOM) vga_draw_pixel(x, y + 1, COLOR_GRID_MAJOR);
        }
    }
    
    // Center horizontal line
    if (x % 2 == 0) {
        vga_draw_pixel(x, center_y, COLOR_AXIS_CENTER);
    }
    
    // Center vertical tick marks
    if (x == center_x) {
        for (int i = 0; i <= GRID_DIVISIONS_Y; i++) {
            int y = WAVEFORM_TOP + i * div_height;
            vga_draw_pixel(x - 2, y, COLOR_TEXT_WHITE);
            vga_draw_pixel(x - 1, y, COLOR_TEXT_WHITE);
            vga_draw_pixel(x + 1, y, COLOR_TEXT_WHITE);
            vga_draw_pixel(x + 2, y, COLOR_TEXT_WHITE);
        }
    }
    
    // Horizontal tick marks at center Y
    if (x % div_width == 0) {
        vga_draw_pixel(x, center_y - 2, COLOR_TEXT_WHITE);
        vga_draw_pixel(x, center_y - 1, COLOR_TEXT_WHITE);
        vga_draw_pixel(x, center_y + 1, COLOR_TEXT_WHITE);
        vga_draw_pixel(x, center_y + 2, COLOR_TEXT_WHITE);
    }
}

// ============================================================================
// High-Level Oscilloscope Functions
// ============================================================================

/**
 * Initialize the complete oscilloscope display
 */
void vga_scope_init(void) {
    vga_clear_screen(COLOR_BACKGROUND);
    vga_draw_header();
    vga_draw_grid();
    vga_draw_footer();
    vga_draw_trigger_line();
}

/**
 * Update the oscilloscope state and redraw header
 */
void vga_scope_update_info(uint8_t channel, float voltage, float v_per_div, 
                           float time_per_div, float v_max, float v_min) {
    scope_state.channel = channel;
    scope_state.voltage = voltage;
    scope_state.v_per_div = v_per_div;
    scope_state.time_per_div = time_per_div;
    scope_state.v_max = v_max;
    scope_state.v_min = v_min;
    scope_state.v_pp = v_max - v_min;
    
    // Redraw header with new info
    vga_draw_header();
}

/**
 * Set the trigger level
 */
void vga_scope_set_trigger(uint16_t level) {
    scope_state.trigger_level = level;
}

/**
 * Set estimated frequency
 */
void vga_scope_set_frequency(float freq) {
    scope_state.frequency = freq;
}

/**
 * Convert ADC value to screen Y coordinate (for external use)
 */
int vga_adc_to_screen_y(uint16_t adc_value) {
    int y = WAVEFORM_BOTTOM - ((adc_value * WAVEFORM_HEIGHT) / 65536);
    if (y < WAVEFORM_TOP) y = WAVEFORM_TOP;
    if (y > WAVEFORM_BOTTOM) y = WAVEFORM_BOTTOM;
    return y;
}

/**
 * Get waveform area boundaries (for external use)
 */
void vga_get_waveform_bounds(int *top, int *bottom, int *left, int *right) {
    if (top) *top = WAVEFORM_TOP;
    if (bottom) *bottom = WAVEFORM_BOTTOM;
    if (left) *left = 0;
    if (right) *right = SCREEN_WIDTH - 1;
}