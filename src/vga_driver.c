/**
 * vga_scope.c - Tektronix-Style Oscilloscope Display Implementation
 * 
 * A clean, well-organized VGA driver for oscilloscope display.
 * Matches the classic Tektronix look with:
 * - Dotted graticule grid
 * - Status bar with Run/Trig'd indicators
 * - Channel info bar at bottom
 * - Ground reference and trigger markers
 */

#include "vga_driver.h"

// =============================================================================
// Global State
// =============================================================================

ScopeState g_scope = {
    .running = true,
    .triggered = false,
    
    .ch1_enabled = true,
    .ch1_vdiv = 1.0f,
    .ch1_coupling = 0,      // DC
    .ch1_y_offset = 50,     // Upper half
    
    .ch2_enabled = true,
    .ch2_vdiv = 0.2f,
    .ch2_coupling = 0,
    .ch2_y_offset = -50,    // Lower half
    
    .time_div_ms = 0.4f,    // 400µs
    .time_is_us = true,
    
    .trig_channel = 1,
    .trig_level_mv = 80.0f,
    .trig_y_pos = 50,
    
    .ch1_vpp = 0.0f,
    .ch2_vpp = 0.0f,
    
    .horiz_pos = 50
};

// =============================================================================
// 5x7 Bitmap Font (ASCII 32-122)
// =============================================================================

static const uint8_t font_5x7[91][5] = {
    {0x00,0x00,0x00,0x00,0x00}, // 32 ' '
    {0x00,0x00,0x5F,0x00,0x00}, // 33 '!'
    {0x00,0x07,0x00,0x07,0x00}, // 34 '"'
    {0x14,0x7F,0x14,0x7F,0x14}, // 35 '#'
    {0x24,0x2A,0x7F,0x2A,0x12}, // 36 '$'
    {0x23,0x13,0x08,0x64,0x62}, // 37 '%'
    {0x36,0x49,0x55,0x22,0x50}, // 38 '&'
    {0x00,0x05,0x03,0x00,0x00}, // 39 '''
    {0x00,0x1C,0x22,0x41,0x00}, // 40 '('
    {0x00,0x41,0x22,0x1C,0x00}, // 41 ')'
    {0x14,0x08,0x3E,0x08,0x14}, // 42 '*'
    {0x08,0x08,0x3E,0x08,0x08}, // 43 '+'
    {0x00,0x50,0x30,0x00,0x00}, // 44 ','
    {0x08,0x08,0x08,0x08,0x08}, // 45 '-'
    {0x00,0x60,0x60,0x00,0x00}, // 46 '.'
    {0x20,0x10,0x08,0x04,0x02}, // 47 '/'
    {0x3E,0x51,0x49,0x45,0x3E}, // 48 '0'
    {0x00,0x42,0x7F,0x40,0x00}, // 49 '1'
    {0x42,0x61,0x51,0x49,0x46}, // 50 '2'
    {0x21,0x41,0x45,0x4B,0x31}, // 51 '3'
    {0x18,0x14,0x12,0x7F,0x10}, // 52 '4'
    {0x27,0x45,0x45,0x45,0x39}, // 53 '5'
    {0x3C,0x4A,0x49,0x49,0x30}, // 54 '6'
    {0x01,0x71,0x09,0x05,0x03}, // 55 '7'
    {0x36,0x49,0x49,0x49,0x36}, // 56 '8'
    {0x06,0x49,0x49,0x29,0x1E}, // 57 '9'
    {0x00,0x36,0x36,0x00,0x00}, // 58 ':'
    {0x00,0x56,0x36,0x00,0x00}, // 59 ';'
    {0x08,0x14,0x22,0x41,0x00}, // 60 '<'
    {0x14,0x14,0x14,0x14,0x14}, // 61 '='
    {0x00,0x41,0x22,0x14,0x08}, // 62 '>'
    {0x02,0x01,0x51,0x09,0x06}, // 63 '?'
    {0x32,0x49,0x79,0x41,0x3E}, // 64 '@'
    {0x7E,0x11,0x11,0x11,0x7E}, // 65 'A'
    {0x7F,0x49,0x49,0x49,0x36}, // 66 'B'
    {0x3E,0x41,0x41,0x41,0x22}, // 67 'C'
    {0x7F,0x41,0x41,0x22,0x1C}, // 68 'D'
    {0x7F,0x49,0x49,0x49,0x41}, // 69 'E'
    {0x7F,0x09,0x09,0x09,0x01}, // 70 'F'
    {0x3E,0x41,0x49,0x49,0x7A}, // 71 'G'
    {0x7F,0x08,0x08,0x08,0x7F}, // 72 'H'
    {0x00,0x41,0x7F,0x41,0x00}, // 73 'I'
    {0x20,0x40,0x41,0x3F,0x01}, // 74 'J'
    {0x7F,0x08,0x14,0x22,0x41}, // 75 'K'
    {0x7F,0x40,0x40,0x40,0x40}, // 76 'L'
    {0x7F,0x02,0x0C,0x02,0x7F}, // 77 'M'
    {0x7F,0x04,0x08,0x10,0x7F}, // 78 'N'
    {0x3E,0x41,0x41,0x41,0x3E}, // 79 'O'
    {0x7F,0x09,0x09,0x09,0x06}, // 80 'P'
    {0x3E,0x41,0x51,0x21,0x5E}, // 81 'Q'
    {0x7F,0x09,0x19,0x29,0x46}, // 82 'R'
    {0x46,0x49,0x49,0x49,0x31}, // 83 'S'
    {0x01,0x01,0x7F,0x01,0x01}, // 84 'T'
    {0x3F,0x40,0x40,0x40,0x3F}, // 85 'U'
    {0x1F,0x20,0x40,0x20,0x1F}, // 86 'V'
    {0x3F,0x40,0x38,0x40,0x3F}, // 87 'W'
    {0x63,0x14,0x08,0x14,0x63}, // 88 'X'
    {0x07,0x08,0x70,0x08,0x07}, // 89 'Y'
    {0x61,0x51,0x49,0x45,0x43}, // 90 'Z'
    {0x00,0x7F,0x41,0x41,0x00}, // 91 '['
    {0x02,0x04,0x08,0x10,0x20}, // 92 '\'
    {0x00,0x41,0x41,0x7F,0x00}, // 93 ']'
    {0x04,0x02,0x01,0x02,0x04}, // 94 '^'
    {0x40,0x40,0x40,0x40,0x40}, // 95 '_'
    {0x00,0x01,0x02,0x04,0x00}, // 96 '`'
    {0x20,0x54,0x54,0x54,0x78}, // 97 'a'
    {0x7F,0x48,0x44,0x44,0x38}, // 98 'b'
    {0x38,0x44,0x44,0x44,0x20}, // 99 'c'
    {0x38,0x44,0x44,0x48,0x7F}, // 100 'd'
    {0x38,0x54,0x54,0x54,0x18}, // 101 'e'
    {0x08,0x7E,0x09,0x01,0x02}, // 102 'f'
    {0x0C,0x52,0x52,0x52,0x3E}, // 103 'g'
    {0x7F,0x08,0x04,0x04,0x78}, // 104 'h'
    {0x00,0x44,0x7D,0x40,0x00}, // 105 'i'
    {0x20,0x40,0x44,0x3D,0x00}, // 106 'j'
    {0x7F,0x10,0x28,0x44,0x00}, // 107 'k'
    {0x00,0x41,0x7F,0x40,0x00}, // 108 'l'
    {0x7C,0x04,0x18,0x04,0x78}, // 109 'm'
    {0x7C,0x08,0x04,0x04,0x78}, // 110 'n'
    {0x38,0x44,0x44,0x44,0x38}, // 111 'o'
    {0x7C,0x14,0x14,0x14,0x08}, // 112 'p'
    {0x08,0x14,0x14,0x18,0x7C}, // 113 'q'
    {0x7C,0x08,0x04,0x04,0x08}, // 114 'r'
    {0x48,0x54,0x54,0x54,0x20}, // 115 's'
    {0x04,0x3F,0x44,0x40,0x20}, // 116 't'
    {0x3C,0x40,0x40,0x20,0x7C}, // 117 'u'
    {0x1C,0x20,0x40,0x20,0x1C}, // 118 'v'
    {0x3C,0x40,0x30,0x40,0x3C}, // 119 'w'
    {0x44,0x28,0x10,0x28,0x44}, // 120 'x'
    {0x0C,0x50,0x50,0x50,0x3C}, // 121 'y'
    {0x44,0x64,0x54,0x4C,0x44}, // 122 'z'
};

// =============================================================================
// Core Drawing Functions
// =============================================================================

void vga_put_pixel(int x, int y, uint8_t color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        VGA_BUFFER[y * SCREEN_WIDTH + x] = color;
    }
}

uint8_t vga_get_pixel(int x, int y) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        return (uint8_t)VGA_BUFFER[y * SCREEN_WIDTH + x];
    }
    return 0;
}

void vga_clear(uint8_t color) {
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        VGA_BUFFER[i] = color;
    }
}

// -----------------------------------------------------------------------------
// Line Drawing
// -----------------------------------------------------------------------------

void vga_hline(int x1, int x2, int y, uint8_t color) {
    if (y < 0 || y >= SCREEN_HEIGHT) return;
    
    // Ensure x1 <= x2
    if (x1 > x2) { int t = x1; x1 = x2; x2 = t; }
    
    // Clamp to screen
    if (x1 < 0) x1 = 0;
    if (x2 >= SCREEN_WIDTH) x2 = SCREEN_WIDTH - 1;
    
    for (int x = x1; x <= x2; x++) {
        VGA_BUFFER[y * SCREEN_WIDTH + x] = color;
    }
}

void vga_vline(int x, int y1, int y2, uint8_t color) {
    if (x < 0 || x >= SCREEN_WIDTH) return;
    
    if (y1 > y2) { int t = y1; y1 = y2; y2 = t; }
    if (y1 < 0) y1 = 0;
    if (y2 >= SCREEN_HEIGHT) y2 = SCREEN_HEIGHT - 1;
    
    for (int y = y1; y <= y2; y++) {
        VGA_BUFFER[y * SCREEN_WIDTH + x] = color;
    }
}

void vga_line(int x1, int y1, int x2, int y2, uint8_t color) {
    // Special cases for horizontal/vertical
    if (y1 == y2) { vga_hline(x1, x2, y1, color); return; }
    if (x1 == x2) { vga_vline(x1, y1, y2, color); return; }
    
    // Bresenham's algorithm
    int dx = (x2 > x1) ? (x2 - x1) : (x1 - x2);
    int dy = (y2 > y1) ? (y2 - y1) : (y1 - y2);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    
    while (1) {
        vga_put_pixel(x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 < dx)  { err += dx; y1 += sy; }
    }
}

// -----------------------------------------------------------------------------
// Rectangle Drawing
// -----------------------------------------------------------------------------

void vga_rect(int x, int y, int w, int h, uint8_t color) {
    vga_hline(x, x + w - 1, y, color);          // Top
    vga_hline(x, x + w - 1, y + h - 1, color);  // Bottom
    vga_vline(x, y, y + h - 1, color);          // Left
    vga_vline(x + w - 1, y, y + h - 1, color);  // Right
}

void vga_rect_fill(int x, int y, int w, int h, uint8_t color) {
    for (int row = y; row < y + h; row++) {
        if (row >= 0 && row < SCREEN_HEIGHT) {
            vga_hline(x, x + w - 1, row, color);
        }
    }
}

// =============================================================================
// Text Drawing
// =============================================================================

void vga_char(int x, int y, char c, uint8_t color) {
    if (c < 32 || c > 122) return;
    
    const uint8_t *glyph = font_5x7[c - 32];
    
    for (int col = 0; col < 5; col++) {
        uint8_t column_bits = glyph[col];
        for (int row = 0; row < 7; row++) {
            if (column_bits & (1 << row)) {
                vga_put_pixel(x + col, y + row, color);
            }
        }
    }
}

void vga_text(int x, int y, const char *str, uint8_t color) {
    while (*str) {
        vga_char(x, y, *str, color);
        x += 6;  // 5 pixels + 1 spacing
        str++;
    }
}

void vga_int(int x, int y, int value, uint8_t color) {
    char buf[12];
    int i = 0;
    int neg = 0;
    
    if (value < 0) {
        neg = 1;
        value = -value;
    }
    
    // Build digits in reverse
    if (value == 0) {
        buf[i++] = '0';
    } else {
        while (value > 0) {
            buf[i++] = '0' + (value % 10);
            value /= 10;
        }
    }
    
    if (neg) buf[i++] = '-';
    
    // Print in correct order
    while (i > 0) {
        vga_char(x, y, buf[--i], color);
        x += 6;
    }
}

void vga_float(int x, int y, float value, int decimals, uint8_t color) {
    // Handle negative
    if (value < 0) {
        vga_char(x, y, '-', color);
        x += 6;
        value = -value;
    }
    
    // Integer part
    int ipart = (int)value;
    float fpart = value - ipart;
    
    // Print integer part
    if (ipart == 0) {
        vga_char(x, y, '0', color);
        x += 6;
    } else {
        char buf[10];
        int i = 0;
        while (ipart > 0) {
            buf[i++] = '0' + (ipart % 10);
            ipart /= 10;
        }
        while (i > 0) {
            vga_char(x, y, buf[--i], color);
            x += 6;
        }
    }
    
    // Decimal point and fraction
    if (decimals > 0) {
        vga_char(x, y, '.', color);
        x += 6;
        
        for (int d = 0; d < decimals; d++) {
            fpart *= 10;
            int digit = (int)fpart;
            vga_char(x, y, '0' + digit, color);
            x += 6;
            fpart -= digit;
        }
    }
}

// =============================================================================
// Oscilloscope UI: Status Bar (Top)
// =============================================================================

void scope_draw_status_bar(void) {
    // Clear status bar area
    vga_rect_fill(0, STATUS_BAR_Y, SCREEN_WIDTH, STATUS_BAR_H, COL_BLACK);
    
    // "Tek" logo (left side)
    vga_text(2, 1, "Tek", COL_WHITE);
    
    // Run/Stop indicator
    if (g_scope.running) {
        vga_text(24, 1, "Run", COL_GREEN);
    } else {
        vga_text(24, 1, "Stop", COL_RED);
    }
    
    // Horizontal position indicator (center bracket with marker)
    // Draw position track
    int track_x = 120;
    int track_w = 80;
    vga_hline(track_x, track_x + track_w, 4, COL_GRAY);
    vga_vline(track_x, 2, 6, COL_GRAY);
    vga_vline(track_x + track_w, 2, 6, COL_GRAY);
    
    // Position marker (triangle/arrow)
    int marker_x = track_x + (g_scope.horiz_pos * track_w / 100);
    vga_vline(marker_x, 1, 7, COL_CYAN);
    vga_put_pixel(marker_x - 1, 2, COL_CYAN);
    vga_put_pixel(marker_x + 1, 2, COL_CYAN);
    
    // Trigger status (right side)
    if (g_scope.triggered) {
        vga_text(260, 1, "Trig'd", COL_GREEN);
    } else {
        vga_text(260, 1, "Ready", COL_GRAY);
    }
}

// =============================================================================
// Oscilloscope UI: Graticule (Grid)
// =============================================================================

void scope_draw_graticule(void) {
    // Clear graticule area
    vga_rect_fill(GRAT_X, GRAT_Y, GRAT_W, GRAT_H, COL_BLACK);
    
    int div_w = GRAT_W / DIV_X;  // 32 pixels per division
    int div_h = GRAT_H / DIV_Y;  // 25 pixels per division
    
    int center_x = GRAT_X + GRAT_W / 2;
    int center_y = GRAT_Y + GRAT_H / 2;
    
    // ----- Draw dotted vertical grid lines -----
    for (int i = 1; i < DIV_X; i++) {
        int x = GRAT_X + i * div_w;
        
        // Dotted line (every 5 pixels)
        for (int y = GRAT_Y; y < GRAT_Y + GRAT_H; y += 5) {
            vga_put_pixel(x, y, COL_GRID_DIM);
        }
    }
    
    // ----- Draw dotted horizontal grid lines -----
    for (int i = 1; i < DIV_Y; i++) {
        int y = GRAT_Y + i * div_h;
        
        for (int x = GRAT_X; x < GRAT_X + GRAT_W; x += 5) {
            vga_put_pixel(x, y, COL_GRID_DIM);
        }
    }
    
    // ----- Center crosshair (brighter, denser dots) -----
    // Horizontal center line
    for (int x = GRAT_X; x < GRAT_X + GRAT_W; x += 2) {
        vga_put_pixel(x, center_y, COL_GRID_BRIGHT);
    }
    
    // Vertical center line
    for (int y = GRAT_Y; y < GRAT_Y + GRAT_H; y += 2) {
        vga_put_pixel(center_x, y, COL_GRID_BRIGHT);
    }
    
    // ----- Tick marks on center lines -----
    // Major ticks (at each division)
    for (int i = 0; i <= DIV_X; i++) {
        int x = GRAT_X + i * div_w;
        vga_vline(x, center_y - 3, center_y + 3, COL_GRID_BRIGHT);
        
        // Minor ticks (5 per division)
        if (i < DIV_X) {
            int minor_step = div_w / 5;
            for (int m = 1; m < 5; m++) {
                int mx = x + m * minor_step;
                vga_vline(mx, center_y - 1, center_y + 1, COL_GRID_BRIGHT);
            }
        }
    }
    
    for (int i = 0; i <= DIV_Y; i++) {
        int y = GRAT_Y + i * div_h;
        vga_hline(center_x - 3, center_x + 3, y, COL_GRID_BRIGHT);
        
        if (i < DIV_Y) {
            int minor_step = div_h / 5;
            for (int m = 1; m < 5; m++) {
                int my = y + m * minor_step;
                vga_hline(center_x - 1, center_x + 1, my, COL_GRID_BRIGHT);
            }
        }
    }
    
    // ----- Border -----
    vga_rect(GRAT_X, GRAT_Y, GRAT_W, GRAT_H, COL_GRID_BRIGHT);
}

// =============================================================================
// Oscilloscope UI: Info Bar (Bottom)
// =============================================================================

void scope_draw_info_bar(void) {
    // Clear info bar
    vga_rect_fill(0, INFO_BAR_Y, SCREEN_WIDTH, INFO_BAR_H, COL_BLACK);
    
    // Separator line at top
    vga_hline(0, SCREEN_WIDTH - 1, INFO_BAR_Y, COL_GRID_DIM);
    
    // ----- Row 1: Channel and timebase info -----
    int row1_y = INFO_BAR_Y + 3;
    
    // CH1 info (yellow)
    vga_text(4, row1_y, "Ch1", COL_CH1);
    vga_float(30, row1_y, g_scope.ch1_vdiv, 2, COL_CH1);
    vga_text(66, row1_y, "V", COL_CH1);
    
    // CH2 info (magenta) 
    if (g_scope.ch2_enabled) {
        vga_text(85, row1_y, "Ch2", COL_CH2);
        vga_float(111, row1_y, g_scope.ch2_vdiv, 1, COL_CH2);
        vga_text(141, row1_y, "V", COL_CH2);
    }
    
    // Timebase (center) - "M 400us" style
    vga_text(160, row1_y, "M", COL_WHITE);
    if (g_scope.time_is_us) {
        vga_float(172, row1_y, g_scope.time_div_ms * 1000, 0, COL_WHITE);
        vga_text(202, row1_y, "us", COL_WHITE);
    } else {
        vga_float(172, row1_y, g_scope.time_div_ms, 1, COL_WHITE);
        vga_text(202, row1_y, "ms", COL_WHITE);
    }
    
    // Trigger info (right) - "A Ch1 f 80.0mV"
    vga_text(230, row1_y, "A", COL_WHITE);
    vga_text(242, row1_y, "Ch1", COL_CH1);
    vga_char(266, row1_y, 'f', COL_WHITE);  // Trigger slope symbol
    vga_float(278, row1_y, g_scope.trig_level_mv, 1, COL_WHITE);
    vga_text(308, row1_y, "mV", COL_WHITE);
    
    // ----- Row 2: Status/measurements -----
    int row2_y = INFO_BAR_Y + 14;
    
    // DC coupling indicators
    vga_text(4, row2_y, "DC", COL_GRAY);
    
    // Measurements (if available)
    if (g_scope.ch1_vpp > 0) {
        vga_text(30, row2_y, "Vpp:", COL_GRAY);
        vga_float(60, row2_y, g_scope.ch1_vpp, 2, COL_CH1);
        vga_text(96, row2_y, "V", COL_CH1);
    }
    
    // ADC resolution indicator
    vga_text(260, row2_y, "16bit", COL_GRAY);
}

// =============================================================================
// Oscilloscope UI: Ground Reference Markers
// =============================================================================

void scope_draw_ground_markers(void) {
    int center_y = GRAT_Y + GRAT_H / 2;
    
    // CH1 ground marker (yellow "1" with arrow on left edge)
    if (g_scope.ch1_enabled) {
        int ch1_y = center_y - g_scope.ch1_y_offset;
        
        // Clamp to graticule
        if (ch1_y < GRAT_Y + 5) ch1_y = GRAT_Y + 5;
        if (ch1_y > GRAT_Y + GRAT_H - 5) ch1_y = GRAT_Y + GRAT_H - 5;
        
        // Draw arrow pointing right: >
        vga_put_pixel(1, ch1_y, COL_CH1);
        vga_put_pixel(2, ch1_y - 1, COL_CH1);
        vga_put_pixel(2, ch1_y + 1, COL_CH1);
        vga_put_pixel(3, ch1_y - 2, COL_CH1);
        vga_put_pixel(3, ch1_y + 2, COL_CH1);
        
        // "1" label
        vga_char(6, ch1_y - 3, '1', COL_CH1);
    }
    
    // CH2 ground marker (magenta "2")
    if (g_scope.ch2_enabled) {
        int ch2_y = center_y - g_scope.ch2_y_offset;
        
        if (ch2_y < GRAT_Y + 5) ch2_y = GRAT_Y + 5;
        if (ch2_y > GRAT_Y + GRAT_H - 5) ch2_y = GRAT_Y + GRAT_H - 5;
        
        vga_put_pixel(1, ch2_y, COL_CH2);
        vga_put_pixel(2, ch2_y - 1, COL_CH2);
        vga_put_pixel(2, ch2_y + 1, COL_CH2);
        vga_put_pixel(3, ch2_y - 2, COL_CH2);
        vga_put_pixel(3, ch2_y + 2, COL_CH2);
        
        vga_char(6, ch2_y - 3, '2', COL_CH2);
    }
}

// =============================================================================
// Oscilloscope UI: Trigger Level Marker
// =============================================================================

void scope_draw_trigger_marker(void) {
    int center_y = GRAT_Y + GRAT_H / 2;
    int trig_y = center_y - g_scope.trig_y_pos;
    
    // Clamp
    if (trig_y < GRAT_Y + 3) trig_y = GRAT_Y + 3;
    if (trig_y > GRAT_Y + GRAT_H - 3) trig_y = GRAT_Y + GRAT_H - 3;
    
    // Draw arrow on right edge pointing left: <
    int x = SCREEN_WIDTH - 5;
    uint8_t color = (g_scope.trig_channel == 1) ? COL_CH1 : COL_CH2;
    
    vga_put_pixel(x + 3, trig_y, color);
    vga_put_pixel(x + 2, trig_y - 1, color);
    vga_put_pixel(x + 2, trig_y + 1, color);
    vga_put_pixel(x + 1, trig_y - 2, color);
    vga_put_pixel(x + 1, trig_y + 2, color);
    vga_put_pixel(x, trig_y - 3, color);
    vga_put_pixel(x, trig_y + 3, color);
}

// =============================================================================
// Waveform Drawing
// =============================================================================

int scope_adc_to_y(uint16_t adc_value, uint8_t channel) {
    // Map ADC (0-65535) to graticule Y coordinate
    // 0 = bottom, 65535 = top
    
    int offset = (channel == 1) ? g_scope.ch1_y_offset : g_scope.ch2_y_offset;
    int center_y = GRAT_Y + GRAT_H / 2 - offset;
    
    // Scale: full ADC range maps to graticule height
    // Centered around the channel's offset position
    int y = center_y + (GRAT_H / 2) - ((int)adc_value * (GRAT_H - 4) / 65535);
    
    // Clamp to graticule bounds
    if (y < GRAT_Y + 1) y = GRAT_Y + 1;
    if (y > GRAT_Y + GRAT_H - 2) y = GRAT_Y + GRAT_H - 2;
    
    return y;
}

void scope_draw_point(int x, uint16_t adc_value, uint8_t channel) {
    if (x < GRAT_X + 1 || x > GRAT_X + GRAT_W - 2) return;
    
    int y = scope_adc_to_y(adc_value, channel);
    uint8_t color = (channel == 1) ? COL_CH1 : COL_CH2;
    
    vga_put_pixel(x, y, color);
}

void scope_draw_segment(int x1, uint16_t adc1, int x2, uint16_t adc2, uint8_t channel) {
    int y1 = scope_adc_to_y(adc1, channel);
    int y2 = scope_adc_to_y(adc2, channel);
    uint8_t color = (channel == 1) ? COL_CH1 : COL_CH2;
    
    vga_line(x1, y1, x2, y2, color);
}

void scope_clear_waveform(void) {
    // Clear just the inner graticule area (preserve border)
    vga_rect_fill(GRAT_X + 1, GRAT_Y + 1, GRAT_W - 2, GRAT_H - 2, COL_BLACK);
    
    // Redraw the grid (it got erased)
    scope_draw_graticule();
}

void scope_erase_column(int x) {
    if (x < GRAT_X + 1 || x > GRAT_X + GRAT_W - 2) return;
    
    // Clear the column
    vga_vline(x, GRAT_Y + 1, GRAT_Y + GRAT_H - 2, COL_BLACK);
    
    // Restore grid elements at this column
    int div_w = GRAT_W / DIV_X;
    int div_h = GRAT_H / DIV_Y;
    int center_x = GRAT_X + GRAT_W / 2;
    int center_y = GRAT_Y + GRAT_H / 2;
    int col = x - GRAT_X;
    
    // Vertical grid line?
    if (col > 0 && col % div_w == 0 && x != center_x) {
        for (int y = GRAT_Y + 1; y < GRAT_Y + GRAT_H - 1; y += 5) {
            vga_put_pixel(x, y, COL_GRID_DIM);
        }
    }
    
    // Horizontal grid lines
    for (int i = 1; i < DIV_Y; i++) {
        int gy = GRAT_Y + i * div_h;
        if ((x - GRAT_X) % 5 == 0) {
            vga_put_pixel(x, gy, COL_GRID_DIM);
        }
    }
    
    // Center horizontal line
    if ((x - GRAT_X) % 2 == 0) {
        vga_put_pixel(x, center_y, COL_GRID_BRIGHT);
    }
    
    // Center vertical line
    if (x == center_x) {
        for (int y = GRAT_Y + 1; y < GRAT_Y + GRAT_H - 1; y += 2) {
            vga_put_pixel(x, y, COL_GRID_BRIGHT);
        }
    }
    
    // Tick marks on horizontal center
    int minor = div_w / 5;
    if (col % div_w == 0) {
        vga_vline(x, center_y - 3, center_y + 3, COL_GRID_BRIGHT);
    } else if (col % minor == 0 && col > 0) {
        vga_vline(x, center_y - 1, center_y + 1, COL_GRID_BRIGHT);
    }
}

// =============================================================================
// Coordinate Helpers
// =============================================================================

int scope_get_left(void)   { return GRAT_X + 1; }
int scope_get_right(void)  { return GRAT_X + GRAT_W - 2; }
int scope_get_top(void)    { return GRAT_Y + 1; }
int scope_get_bottom(void) { return GRAT_Y + GRAT_H - 2; }

// =============================================================================
// High-Level Init & Redraw
// =============================================================================

void scope_init(void) {
    vga_clear(COL_BLACK);
    scope_redraw_all();
}

void scope_redraw_all(void) {
    scope_draw_status_bar();
    scope_draw_graticule();
    scope_draw_info_bar();
    scope_draw_ground_markers();
    scope_draw_trigger_marker();
}

// =============================================================================
// State Update Functions
// =============================================================================

void scope_set_running(bool running) {
    g_scope.running = running;
    scope_draw_status_bar();
}

void scope_set_triggered(bool triggered) {
    g_scope.triggered = triggered;
    scope_draw_status_bar();
}

void scope_set_ch1_vdiv(float vdiv) {
    g_scope.ch1_vdiv = vdiv;
    scope_draw_info_bar();
}

void scope_set_ch2_vdiv(float vdiv) {
    g_scope.ch2_vdiv = vdiv;
    scope_draw_info_bar();
}

void scope_set_timebase(float time_div, bool is_microseconds) {
    g_scope.time_div_ms = time_div;
    g_scope.time_is_us = is_microseconds;
    scope_draw_info_bar();
}

void scope_set_trigger_level(float level_mv) {
    g_scope.trig_level_mv = level_mv;
    scope_draw_info_bar();
    scope_draw_trigger_marker();
}

void scope_set_measurements(float ch1_vpp, float ch2_vpp) {
    g_scope.ch1_vpp = ch1_vpp;
    g_scope.ch2_vpp = ch2_vpp;
    scope_draw_info_bar();
}