/**
 * vga_driver.c - Tektronix-Style Oscilloscope VGA Display
 * 
 * Clean, professional oscilloscope display for DE10-Lite
 * 320x240 resolution, 16-bit buffer (RGB332 in lower byte)
 * 
 * Layout:
 * - Top: Status bar (12px)
 * - Center: Waveform grid (200px) 
 * - Bottom: Channel info bar (28px)
 */

#include "vga_driver.h"
#include <stdint.h>

// ============================================================================
// Screen Layout
// ============================================================================
#define TOP_BAR_H       12
#define BOTTOM_BAR_H    28
#define GRID_X          0
#define GRID_Y          TOP_BAR_H
#define GRID_W          320
#define GRID_H          (240 - TOP_BAR_H - BOTTOM_BAR_H)  // 200px

// Grid divisions: 10 horizontal, 8 vertical (standard scope)
#define DIV_X           10
#define DIV_Y           8

// ============================================================================
// Colors - Tektronix Style (RGB332: RRRGGGBB)
// ============================================================================
#define C_BLACK         0x00
#define C_DARK_BLUE     0x01    // Very dark blue background
#define C_GRID          0x24    // Dim gray for grid
#define C_GRID_BRIGHT   0x49    // Brighter for major lines
#define C_YELLOW        0xFC    // Channel 1 waveform
#define C_CYAN          0x1F    // Channel 2 waveform  
#define C_WHITE         0xFF    // Bright text
#define C_GRAY          0x92    // Dim text
#define C_GREEN         0x1C    // Status indicators
#define C_RED           0xE0    // Trigger/alerts

// ============================================================================
// 5x7 Font - Compact ASCII (32-122)
// ============================================================================
static const uint8_t font[91][5] = {
    {0x00,0x00,0x00,0x00,0x00}, // 32 space
    {0x00,0x00,0x5F,0x00,0x00}, // 33 !
    {0x00,0x07,0x00,0x07,0x00}, // 34 "
    {0x14,0x7F,0x14,0x7F,0x14}, // 35 #
    {0x24,0x2A,0x7F,0x2A,0x12}, // 36 $
    {0x23,0x13,0x08,0x64,0x62}, // 37 %
    {0x36,0x49,0x55,0x22,0x50}, // 38 &
    {0x00,0x05,0x03,0x00,0x00}, // 39 '
    {0x00,0x1C,0x22,0x41,0x00}, // 40 (
    {0x00,0x41,0x22,0x1C,0x00}, // 41 )
    {0x14,0x08,0x3E,0x08,0x14}, // 42 *
    {0x08,0x08,0x3E,0x08,0x08}, // 43 +
    {0x00,0x50,0x30,0x00,0x00}, // 44 ,
    {0x08,0x08,0x08,0x08,0x08}, // 45 -
    {0x00,0x60,0x60,0x00,0x00}, // 46 .
    {0x20,0x10,0x08,0x04,0x02}, // 47 /
    {0x3E,0x51,0x49,0x45,0x3E}, // 48 0
    {0x00,0x42,0x7F,0x40,0x00}, // 49 1
    {0x42,0x61,0x51,0x49,0x46}, // 50 2
    {0x21,0x41,0x45,0x4B,0x31}, // 51 3
    {0x18,0x14,0x12,0x7F,0x10}, // 52 4
    {0x27,0x45,0x45,0x45,0x39}, // 53 5
    {0x3C,0x4A,0x49,0x49,0x30}, // 54 6
    {0x01,0x71,0x09,0x05,0x03}, // 55 7
    {0x36,0x49,0x49,0x49,0x36}, // 56 8
    {0x06,0x49,0x49,0x29,0x1E}, // 57 9
    {0x00,0x36,0x36,0x00,0x00}, // 58 :
    {0x00,0x56,0x36,0x00,0x00}, // 59 ;
    {0x08,0x14,0x22,0x41,0x00}, // 60 <
    {0x14,0x14,0x14,0x14,0x14}, // 61 =
    {0x00,0x41,0x22,0x14,0x08}, // 62 >
    {0x02,0x01,0x51,0x09,0x06}, // 63 ?
    {0x32,0x49,0x79,0x41,0x3E}, // 64 @
    {0x7E,0x11,0x11,0x11,0x7E}, // 65 A
    {0x7F,0x49,0x49,0x49,0x36}, // 66 B
    {0x3E,0x41,0x41,0x41,0x22}, // 67 C
    {0x7F,0x41,0x41,0x22,0x1C}, // 68 D
    {0x7F,0x49,0x49,0x49,0x41}, // 69 E
    {0x7F,0x09,0x09,0x09,0x01}, // 70 F
    {0x3E,0x41,0x49,0x49,0x7A}, // 71 G
    {0x7F,0x08,0x08,0x08,0x7F}, // 72 H
    {0x00,0x41,0x7F,0x41,0x00}, // 73 I
    {0x20,0x40,0x41,0x3F,0x01}, // 74 J
    {0x7F,0x08,0x14,0x22,0x41}, // 75 K
    {0x7F,0x40,0x40,0x40,0x40}, // 76 L
    {0x7F,0x02,0x0C,0x02,0x7F}, // 77 M
    {0x7F,0x04,0x08,0x10,0x7F}, // 78 N
    {0x3E,0x41,0x41,0x41,0x3E}, // 79 O
    {0x7F,0x09,0x09,0x09,0x06}, // 80 P
    {0x3E,0x41,0x51,0x21,0x5E}, // 81 Q
    {0x7F,0x09,0x19,0x29,0x46}, // 82 R
    {0x46,0x49,0x49,0x49,0x31}, // 83 S
    {0x01,0x01,0x7F,0x01,0x01}, // 84 T
    {0x3F,0x40,0x40,0x40,0x3F}, // 85 U
    {0x1F,0x20,0x40,0x20,0x1F}, // 86 V
    {0x3F,0x40,0x38,0x40,0x3F}, // 87 W
    {0x63,0x14,0x08,0x14,0x63}, // 88 X
    {0x07,0x08,0x70,0x08,0x07}, // 89 Y
    {0x61,0x51,0x49,0x45,0x43}, // 90 Z
    {0x00,0x7F,0x41,0x41,0x00}, // 91 [
    {0x02,0x04,0x08,0x10,0x20}, // 92 backslash
    {0x00,0x41,0x41,0x7F,0x00}, // 93 ]
    {0x04,0x02,0x01,0x02,0x04}, // 94 ^
    {0x40,0x40,0x40,0x40,0x40}, // 95 _
    {0x00,0x01,0x02,0x04,0x00}, // 96 `
    {0x20,0x54,0x54,0x54,0x78}, // 97 a
    {0x7F,0x48,0x44,0x44,0x38}, // 98 b
    {0x38,0x44,0x44,0x44,0x20}, // 99 c
    {0x38,0x44,0x44,0x48,0x7F}, // 100 d
    {0x38,0x54,0x54,0x54,0x18}, // 101 e
    {0x08,0x7E,0x09,0x01,0x02}, // 102 f
    {0x0C,0x52,0x52,0x52,0x3E}, // 103 g
    {0x7F,0x08,0x04,0x04,0x78}, // 104 h
    {0x00,0x44,0x7D,0x40,0x00}, // 105 i
    {0x20,0x40,0x44,0x3D,0x00}, // 106 j
    {0x7F,0x10,0x28,0x44,0x00}, // 107 k
    {0x00,0x41,0x7F,0x40,0x00}, // 108 l
    {0x7C,0x04,0x18,0x04,0x78}, // 109 m
    {0x7C,0x08,0x04,0x04,0x78}, // 110 n
    {0x38,0x44,0x44,0x44,0x38}, // 111 o
    {0x7C,0x14,0x14,0x14,0x08}, // 112 p
    {0x08,0x14,0x14,0x18,0x7C}, // 113 q
    {0x7C,0x08,0x04,0x04,0x08}, // 114 r
    {0x48,0x54,0x54,0x54,0x20}, // 115 s
    {0x04,0x3F,0x44,0x40,0x20}, // 116 t
    {0x3C,0x40,0x40,0x20,0x7C}, // 117 u
    {0x1C,0x20,0x40,0x20,0x1C}, // 118 v
    {0x3C,0x40,0x30,0x40,0x3C}, // 119 w
    {0x44,0x28,0x10,0x28,0x44}, // 120 x
    {0x0C,0x50,0x50,0x50,0x3C}, // 121 y
    {0x44,0x64,0x54,0x4C,0x44}, // 122 z
};

// ============================================================================
// Oscilloscope State
// ============================================================================
static struct {
    int running;           // 1=Run, 0=Stop
    int triggered;         // 1=Trig'd, 0=waiting
    float ch1_vdiv;        // V/div for CH1
    float ch2_vdiv;        // V/div for CH2
    float time_div;        // Time/div in ms
    float ch1_pkpk;        // CH1 peak-to-peak
    float ch2_pkpk;        // CH2 peak-to-peak
    int ch1_enabled;       // CH1 on/off
    int ch2_enabled;       // CH2 on/off
} scope = {
    .running = 1,
    .triggered = 0,
    .ch1_vdiv = 0.5f,
    .ch2_vdiv = 1.0f,
    .time_div = 5.0f,
    .ch1_pkpk = 0.0f,
    .ch2_pkpk = 0.0f,
    .ch1_enabled = 1,
    .ch2_enabled = 0
};

// ============================================================================
// Basic Drawing
// ============================================================================

void vga_clear_screen(uint16_t color) {
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        pVGA_PIXEL_BUFFER[i] = color;
    }
}

void vga_draw_pixel(int x, int y, uint16_t color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        pVGA_PIXEL_BUFFER[y * SCREEN_WIDTH + x] = color;
    }
}

static void hline(int x1, int x2, int y, uint16_t c) {
    if (y < 0 || y >= SCREEN_HEIGHT) return;
    if (x1 > x2) { int t = x1; x1 = x2; x2 = t; }
    if (x1 < 0) x1 = 0;
    if (x2 >= SCREEN_WIDTH) x2 = SCREEN_WIDTH - 1;
    for (int x = x1; x <= x2; x++) {
        pVGA_PIXEL_BUFFER[y * SCREEN_WIDTH + x] = c;
    }
}

static void vline(int x, int y1, int y2, uint16_t c) {
    if (x < 0 || x >= SCREEN_WIDTH) return;
    if (y1 > y2) { int t = y1; y1 = y2; y2 = t; }
    if (y1 < 0) y1 = 0;
    if (y2 >= SCREEN_HEIGHT) y2 = SCREEN_HEIGHT - 1;
    for (int y = y1; y <= y2; y++) {
        pVGA_PIXEL_BUFFER[y * SCREEN_WIDTH + x] = c;
    }
}

int abs(int n) { return n < 0 ? -n : n; }

void vga_draw_line(int x1, int y1, int x2, int y2, uint16_t color) {
    if (y1 == y2) { hline(x1, x2, y1, color); return; }
    if (x1 == x2) { vline(x1, y1, y2, color); return; }
    
    int dx = abs(x2 - x1), dy = abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1, sy = y1 < y2 ? 1 : -1;
    int err = dx - dy;
    
    while (1) {
        vga_draw_pixel(x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 < dx) { err += dx; y1 += sy; }
    }
}

void vga_draw_filled_box(int x, int y, int w, int h, uint16_t c) {
    for (int row = y; row < y + h && row < SCREEN_HEIGHT; row++) {
        if (row >= 0) hline(x, x + w - 1, row, c);
    }
}

void vga_draw_box_outline(int x, int y, int w, int h, uint16_t c) {
    hline(x, x + w - 1, y, c);
    hline(x, x + w - 1, y + h - 1, c);
    vline(x, y, y + h - 1, c);
    vline(x + w - 1, y, y + h - 1, c);
}

// ============================================================================
// Text Drawing
// ============================================================================

void vga_draw_char(int x, int y, char c, uint16_t color) {
    if (c < 32 || c > 122) return;
    const uint8_t *g = font[c - 32];
    for (int col = 0; col < 5; col++) {
        uint8_t bits = g[col];
        for (int row = 0; row < 7; row++) {
            if (bits & (1 << row)) {
                vga_draw_pixel(x + col, y + row, color);
            }
        }
    }
}

void vga_draw_string(int x, int y, const char *s, uint16_t color) {
    while (*s) {
        vga_draw_char(x, y, *s++, color);
        x += 6;
    }
}

void vga_draw_int(int x, int y, int val, uint16_t color) {
    char buf[12];
    int i = 0, neg = 0;
    if (val < 0) { neg = 1; val = -val; }
    if (val == 0) buf[i++] = '0';
    else while (val > 0) { buf[i++] = '0' + (val % 10); val /= 10; }
    if (neg) buf[i++] = '-';
    while (i > 0) { vga_draw_char(x, y, buf[--i], color); x += 6; }
}

// Draw float: handles 0-999 with 1-2 decimal places
static void draw_float(int x, int y, float val, int dec, uint16_t color) {
    if (val < 0) { vga_draw_char(x, y, '-', color); x += 6; val = -val; }
    
    int ipart = (int)val;
    float fpart = val - ipart;
    
    // Integer part
    if (ipart == 0) {
        vga_draw_char(x, y, '0', color);
        x += 6;
    } else {
        char buf[6];
        int i = 0;
        while (ipart > 0) { buf[i++] = '0' + (ipart % 10); ipart /= 10; }
        while (i > 0) { vga_draw_char(x, y, buf[--i], color); x += 6; }
    }
    
    // Decimal
    if (dec > 0) {
        vga_draw_char(x, y, '.', color);
        x += 6;
        for (int d = 0; d < dec; d++) {
            fpart *= 10;
            int digit = (int)fpart;
            vga_draw_char(x, y, '0' + digit, color);
            x += 6;
            fpart -= digit;
        }
    }
}

// ============================================================================
// Grid Drawing - Clean Tektronix style
// ============================================================================

void vga_draw_grid(void) {
    int div_w = GRID_W / DIV_X;  // 32 pixels
    int div_h = GRID_H / DIV_Y;  // 25 pixels
    int cx = GRID_X + GRID_W / 2;
    int cy = GRID_Y + GRID_H / 2;
    
    // Draw dotted grid lines
    // Vertical lines
    for (int i = 1; i < DIV_X; i++) {
        int x = GRID_X + i * div_w;
        for (int y = GRID_Y; y < GRID_Y + GRID_H; y += 5) {
            vga_draw_pixel(x, y, C_GRID);
        }
    }
    
    // Horizontal lines  
    for (int i = 1; i < DIV_Y; i++) {
        int y = GRID_Y + i * div_h;
        for (int x = GRID_X; x < GRID_X + GRID_W; x += 5) {
            vga_draw_pixel(x, y, C_GRID);
        }
    }
    
    // Center crosshair - denser dots
    for (int x = GRID_X; x < GRID_X + GRID_W; x += 3) {
        vga_draw_pixel(x, cy, C_GRID_BRIGHT);
    }
    for (int y = GRID_Y; y < GRID_Y + GRID_H; y += 3) {
        vga_draw_pixel(cx, y, C_GRID_BRIGHT);
    }
    
    // Tick marks on center lines
    for (int i = 0; i <= DIV_X; i++) {
        int x = GRID_X + i * div_w;
        // Major tick
        vline(x, cy - 3, cy + 3, C_GRID_BRIGHT);
        // Minor ticks (5 per div)
        if (i < DIV_X) {
            int step = div_w / 5;
            for (int m = 1; m < 5; m++) {
                vline(x + m * step, cy - 1, cy + 1, C_GRID_BRIGHT);
            }
        }
    }
    
    for (int i = 0; i <= DIV_Y; i++) {
        int y = GRID_Y + i * div_h;
        hline(cx - 3, cx + 3, y, C_GRID_BRIGHT);
        if (i < DIV_Y) {
            int step = div_h / 5;
            for (int m = 1; m < 5; m++) {
                hline(cx - 1, cx + 1, y + m * step, C_GRID_BRIGHT);
            }
        }
    }
    
    // Border
    vga_draw_box_outline(GRID_X, GRID_Y, GRID_W, GRID_H, C_GRID_BRIGHT);
}

// ============================================================================
// Header Bar (Top)
// ============================================================================

void vga_draw_header(void) {
    // Clear header
    vga_draw_filled_box(0, 0, SCREEN_WIDTH, TOP_BAR_H, C_BLACK);
    
    // "Tek" logo style
    vga_draw_string(4, 2, "Tek", C_WHITE);
    
    // Run/Stop status
    if (scope.running) {
        vga_draw_string(30, 2, "Run", C_GREEN);
    } else {
        vga_draw_string(30, 2, "Stop", C_RED);
    }
    
    // Trigger status
    if (scope.triggered) {
        vga_draw_string(240, 2, "Trig'd", C_GREEN);
    } else {
        vga_draw_string(240, 2, "Ready", C_GRAY);
    }
    
    // Separator line
    hline(0, SCREEN_WIDTH - 1, TOP_BAR_H - 1, C_GRID);
}

// ============================================================================
// Footer Bar (Bottom) - Channel info like Tek scope
// ============================================================================

void vga_draw_footer(void) {
    int y = SCREEN_HEIGHT - BOTTOM_BAR_H;
    
    // Clear footer
    vga_draw_filled_box(0, y, SCREEN_WIDTH, BOTTOM_BAR_H, C_BLACK);
    
    // Separator line
    hline(0, SCREEN_WIDTH - 1, y, C_GRID);
    
    // Row 1: Channel settings
    int row1 = y + 4;
    
    // CH1 indicator and V/div
    vga_draw_string(4, row1, "Ch1", C_YELLOW);
    draw_float(30, row1, scope.ch1_vdiv, 2, C_YELLOW);
    vga_draw_string(66, row1, "V", C_YELLOW);
    
    // CH2 indicator and V/div  
    vga_draw_string(90, row1, "Ch2", C_CYAN);
    draw_float(116, row1, scope.ch2_vdiv, 2, C_CYAN);
    vga_draw_string(152, row1, "V", C_CYAN);
    
    // Time/div
    vga_draw_string(175, row1, "M", C_WHITE);
    draw_float(188, row1, scope.time_div, 1, C_WHITE);
    vga_draw_string(224, row1, "ms", C_WHITE);
    
    // Row 2: Measurements
    int row2 = y + 15;
    
    // CH1 Pk-Pk
    vga_draw_string(4, row2, "Pk:", C_GRAY);
    draw_float(28, row2, scope.ch1_pkpk, 2, C_YELLOW);
    vga_draw_string(70, row2, "V", C_YELLOW);
    
    // CH2 Pk-Pk
    if (scope.ch2_enabled) {
        vga_draw_string(90, row2, "Pk:", C_GRAY);
        draw_float(114, row2, scope.ch2_pkpk, 2, C_CYAN);
        vga_draw_string(156, row2, "V", C_CYAN);
    }
    
    // DC coupling indicator
    vga_draw_string(260, row1, "DC", C_WHITE);
    
    // AD7705 indicator
    vga_draw_string(260, row2, "16bit", C_GRAY);
}

// ============================================================================
// Waveform Drawing
// ============================================================================

void vga_clear_waveform_area(void) {
    vga_draw_filled_box(GRID_X + 1, GRID_Y + 1, GRID_W - 2, GRID_H - 2, C_BLACK);
}

void vga_draw_waveform_segment(int x1, uint16_t y1_adc, int x2, uint16_t y2_adc, uint16_t color) {
    // Map ADC (0-65535) to screen Y
    int sy1 = GRID_Y + GRID_H - 1 - (int)((uint32_t)y1_adc * (GRID_H - 2) / 65535);
    int sy2 = GRID_Y + GRID_H - 1 - (int)((uint32_t)y2_adc * (GRID_H - 2) / 65535);
    
    // Clamp
    if (sy1 < GRID_Y + 1) sy1 = GRID_Y + 1;
    if (sy1 > GRID_Y + GRID_H - 2) sy1 = GRID_Y + GRID_H - 2;
    if (sy2 < GRID_Y + 1) sy2 = GRID_Y + 1;
    if (sy2 > GRID_Y + GRID_H - 2) sy2 = GRID_Y + GRID_H - 2;
    
    vga_draw_line(x1, sy1, x2, sy2, color);
}

void vga_erase_column(int x) {
    if (x < GRID_X + 1 || x > GRID_X + GRID_W - 2) return;
    
    // Clear column
    vline(x, GRID_Y + 1, GRID_Y + GRID_H - 2, C_BLACK);
    
    // Restore grid
    int div_w = GRID_W / DIV_X;
    int div_h = GRID_H / DIV_Y;
    int cx = GRID_X + GRID_W / 2;
    int cy = GRID_Y + GRID_H / 2;
    int col = x - GRID_X;
    
    // Vertical grid line?
    if (col > 0 && col % div_w == 0 && x != cx) {
        for (int y = GRID_Y; y < GRID_Y + GRID_H; y += 5) {
            vga_draw_pixel(x, y, C_GRID);
        }
    }
    
    // Horizontal grid lines
    for (int i = 1; i < DIV_Y; i++) {
        int gy = GRID_Y + i * div_h;
        if ((x - GRID_X) % 5 == 0) {
            vga_draw_pixel(x, gy, C_GRID);
        }
    }
    
    // Center horizontal line
    if ((x - GRID_X) % 3 == 0) {
        vga_draw_pixel(x, cy, C_GRID_BRIGHT);
    }
    
    // Center vertical line
    if (x == cx) {
        for (int y = GRID_Y; y < GRID_Y + GRID_H; y += 3) {
            vga_draw_pixel(x, y, C_GRID_BRIGHT);
        }
    }
    
    // Ticks on center line
    int minor = div_w / 5;
    if (col % div_w == 0) {
        vline(x, cy - 3, cy + 3, C_GRID_BRIGHT);
    } else if (col % minor == 0) {
        vline(x, cy - 1, cy + 1, C_GRID_BRIGHT);
    }
}

int vga_adc_to_screen_y(uint16_t adc_value) {
    int y = GRID_Y + GRID_H - 1 - (int)((uint32_t)adc_value * (GRID_H - 2) / 65535);
    if (y < GRID_Y + 1) y = GRID_Y + 1;
    if (y > GRID_Y + GRID_H - 2) y = GRID_Y + GRID_H - 2;
    return y;
}

void vga_get_waveform_bounds(int *top, int *bottom, int *left, int *right) {
    if (top) *top = GRID_Y + 1;
    if (bottom) *bottom = GRID_Y + GRID_H - 2;
    if (left) *left = GRID_X + 1;
    if (right) *right = GRID_X + GRID_W - 2;
}

// ============================================================================
// High-Level API
// ============================================================================

void vga_scope_init(void) {
    vga_clear_screen(C_BLACK);
    vga_draw_header();
    vga_draw_grid();
    vga_draw_footer();
}

void vga_scope_update_info(uint8_t channel, float voltage, float v_per_div,
                           float time_per_div, float v_max, float v_min) {
    if (channel == 1) {
        scope.ch1_vdiv = v_per_div;
        scope.ch1_pkpk = v_max - v_min;
    } else {
        scope.ch2_vdiv = v_per_div;
        scope.ch2_pkpk = v_max - v_min;
    }
    scope.time_div = time_per_div;
    
    // Redraw footer with new values
    vga_draw_footer();
}

void vga_scope_set_trigger(uint16_t level) {
    scope.triggered = 1;
}

void vga_scope_set_frequency(float freq) {
    // Could display frequency
}

void vga_scope_set_running(uint8_t running) {
    scope.running = running;
    vga_draw_header();
}

void vga_scope_set_channel(int ch, int enabled) {
    if (ch == 1) scope.ch1_enabled = enabled;
    else scope.ch2_enabled = enabled;
}