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
#include "vga_font.h"
#include <stdint.h>


// Screen Layout
#define TOP_BAR_H       12
#define BOTTOM_BAR_H    28
#define GRID_X          0
#define GRID_Y          TOP_BAR_H
#define GRID_W          320
#define GRID_H          (240 - TOP_BAR_H - BOTTOM_BAR_H)  // 200px

// Grid divisions: 10 horizontal, 8 vertical (standard scope)
#define DIV_X           20
#define DIV_Y           16
#define GRID_DIVISIONS_X 10 
#define GRID_DIVISIONS_Y 8
#define HEADER_H 20
#define FOOTER_H 20
#define GRID_TOP HEADER_H
#define GRID_BOTTOM (SCREEN_HEIGHT - FOOTER_H)

// Oscilloscope State
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




// Fills the entire screen with a single color.
// This is fast as it loops through the memory buffer linearly.
void vga_clear_screen(uint16_t color){
    int total_pixels = SCREEN_WIDTH * SCREEN_HEIGHT;
    for (int i = 0; i < total_pixels; i++) {
        pVGA_PIXEL_BUFFER[i] = color;
    }
}

// Draws a single pixel on the screen at (x, y).
void vga_draw_pixel(int x, int y, uint16_t color) {
    // Check for out-of-bounds coordinates
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        int offset = (y * SCREEN_WIDTH) + x;
        pVGA_PIXEL_BUFFER[offset] = color;
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

// Draws a line between (x1, y1) and (x2, y2) using Bresenham's algorithm.
void vga_draw_line(int x1, int y1, int x2, int y2, uint16_t color) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    int e2;

    while (1) {
        vga_draw_pixel(x1, y1, color);

        if (x1 == x2 && y1 == y2) {
            break;
        }

        e2 = 2 * err;
        // Use >= and <= to prevent stalls on straight lines
        if (e2 >= -dy) { 
            err -= dy;
            x1 += sx;
        }
        if (e2 <= dx) { 
            err += dx;
            y1 += sy;
        }
    }
}

void vga_draw_filled_box(int x, int y, int w, int h, uint16_t c) {
    for (int row = y; row < y + h && row < SCREEN_HEIGHT; row++) {
        if (row >= 0) hline(x, x + w - 1, row, c);
    }
}

// Draws a non-filled rectangle (a box outline).
void vga_draw_box_outline(int x, int y, int width, int height, uint16_t color) {
    int x2 = x + width - 1;
    int y2 = y + height - 1;

    // Draw the four lines that make up the rectangle
    vga_draw_line(x, y, x2, y, color);        // Top line
    vga_draw_line(x, y2, x2, y2, color);      // Bottom line
    vga_draw_line(x, y, x, y2, color);        // Left line
    vga_draw_line(x2, y, x2, y2, color);      // Right line
}



// ============================================================================
// Grid Drawing - Clean Tektronix style
// ============================================================================

// Draws a grid on the screen, for x and y axes
void vga_draw_grid() {
    int i;
    int x_spacing = SCREEN_WIDTH / GRID_DIVISIONS_X; // 32
    int y_spacing = SCREEN_HEIGHT / GRID_DIVISIONS_Y; // 30

    // Draw vertical grid lines (darker)
    for (i = 1; i < GRID_DIVISIONS_X; i++) {
        vga_draw_line(i * x_spacing, 0, i * x_spacing, SCREEN_HEIGHT - 1, COLOR_GRID_BLUE);
    }

    // Draw horizontal grid lines (darker)
    for (i = 1; i < GRID_DIVISIONS_Y; i++) {
        vga_draw_line(0, i * y_spacing, SCREEN_WIDTH - 1, i * y_spacing, COLOR_GRID_BLUE);
    }

    // Draw main axes 
    vga_draw_line(0, SCREEN_HEIGHT / 2, SCREEN_WIDTH - 1, SCREEN_HEIGHT / 2, COLOR_DARK_GRAY); // X-axis
    vga_draw_line(SCREEN_WIDTH / 2, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT - 1, COLOR_DARK_GRAY); // Y-axis
}

// ============================================================================
// Header Bar (Top)
// ============================================================================

void vga_draw_header(void) {
    // Clear header
    vga_draw_filled_box(0, 0, SCREEN_WIDTH, TOP_BAR_H, COLOR_BLACK);
    
    // "Tek" logo style
    vga_draw_string(4, 2, "Tek", COLOR_WHITE);
    
    // Run/Stop status
    if (scope.running) {
        vga_draw_string(30, 2, "Run", COLOR_GREEN);
    } else {
        vga_draw_string(30, 2, "Stop", COLOR_RED);
    }
    
    // Trigger status
    if (scope.triggered) {
        vga_draw_string(240, 2, "Trig'd", COLOR_GREEN);
    } else {
        vga_draw_string(240, 2, "Ready", COLOR_DARK_GRAY);
    }
    
    // Separator line
    hline(0, SCREEN_WIDTH - 1, TOP_BAR_H - 1, COLOR_GRID_BLUE);
}

// ============================================================================
// Footer Bar (Bottom) - Channel info like Tek scope
// ============================================================================

void vga_draw_footer(void) {
    int y = SCREEN_HEIGHT - BOTTOM_BAR_H;
    
    // Clear footer
    vga_draw_filled_box(0, y, SCREEN_WIDTH, BOTTOM_BAR_H, COLOR_BLACK);
    
    // Separator line
    hline(0, SCREEN_WIDTH - 1, y, COLOR_GRID_BLUE);
    
    // Row 1: Channel settings
    int row1 = y + 4;
    
    // CH1 indicator and V/div
    vga_draw_string(4, row1, "Ch1", COLOR_YELLOW);
    draw_float(30, row1, scope.ch1_vdiv, 2, COLOR_YELLOW);
    vga_draw_string(66, row1, "V", COLOR_YELLOW);
    
    // CH2 indicator and V/div  
    vga_draw_string(90, row1, "Ch2", COLOR_CYAN);
    draw_float(116, row1, scope.ch2_vdiv, 2, COLOR_CYAN);
    vga_draw_string(152, row1, "V", COLOR_CYAN);
    
    // Time/div
    vga_draw_string(175, row1, "M", COLOR_WHITE);
    draw_float(188, row1, scope.time_div, 1, COLOR_WHITE);
    vga_draw_string(224, row1, "ms", COLOR_WHITE);
    
    // Row 2: Measurements
    int row2 = y + 15;
    
    // CH1 Pk-Pk
    vga_draw_string(4, row2, "Pk:", COLOR_DARK_GRAY);
    draw_float(28, row2, scope.ch1_pkpk, 2, COLOR_YELLOW);
    vga_draw_string(70, row2, "V", COLOR_YELLOW);
    
    // CH2 Pk-Pk
    if (scope.ch2_enabled) {
        vga_draw_string(90, row2, "Pk:", COLOR_DARK_GRAY);
        draw_float(114, row2, scope.ch2_pkpk, 2, COLOR_CYAN);
        vga_draw_string(156, row2, "V", COLOR_CYAN);
    }
    
    // DC coupling indicator
    vga_draw_string(260, row1, "DC", COLOR_WHITE);
    
    // AD7705 indicator
    vga_draw_string(260, row2, "16bit", COLOR_DARK_GRAY);
}

// ============================================================================
// Waveform Drawing
// ============================================================================

void vga_clear_waveform_area(void) {
    vga_draw_filled_box(GRID_X + 1, GRID_Y + 1, GRID_W - 2, GRID_H - 2, COLOR_BLACK);
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
    vline(x, GRID_Y + 1, GRID_Y + GRID_H - 2, COLOR_BLACK);
    
    // Restore grid
    int div_w = GRID_W / DIV_X;
    int div_h = GRID_H / DIV_Y;
    int cx = GRID_X + GRID_W / 2;
    int cy = GRID_Y + GRID_H / 2;
    int col = x - GRID_X;
    
    // Vertical grid line?
    if (col > 0 && col % div_w == 0 && x != cx) {
        for (int y = GRID_Y; y < GRID_Y + GRID_H; y += 5) {
            vga_draw_pixel(x, y, COLOR_GRID_BLUE);
        }
    }
    
    // Horizontal grid lines
    for (int i = 1; i < DIV_Y; i++) {
        int gy = GRID_Y + i * div_h;
        if ((x - GRID_X) % 5 == 0) {
            vga_draw_pixel(x, gy, COLOR_GRID_BLUE);
        }
    }
    
    // Center horizontal line
    if ((x - GRID_X) % 3 == 0) {
        vga_draw_pixel(x, cy, COLOR_DARK_GRAY);
    }
    
    // Center vertical line
    if (x == cx) {
        for (int y = GRID_Y; y < GRID_Y + GRID_H; y += 3) {
            vga_draw_pixel(x, y, COLOR_DARK_GRAY);
        }
    }
    
    // Ticks on center line
    int minor = div_w / 5;
    if (col % div_w == 0) {
        vline(x, cy - 3, cy + 3, COLOR_DARK_GRAY);
    } else if (col % minor == 0) {
        vline(x, cy - 1, cy + 1, COLOR_DARK_GRAY);
    }
}

// Convert raw ADC value (0-65535) to Screen Y Coordinate
int vga_adc_to_screen_y(uint16_t adc_value) {
    // AD7705 is 16-bit. Midpoint 32768 should be center screen.
    // Scale: Let's assume full scale ADC = full grid height
    
    int grid_height = GRID_BOTTOM - GRID_TOP;
    
    // Invert because Screen Y=0 is top, but High Voltage is Top.
    // Normalized: (adc / 65535) * height
    // But we want 32768 at Center.
    
    // Simple linear map:
    // y = bottom - (adc / max_adc) * height
    
    float ratio = (float)adc_value / 65535.0f;
    int pixel_h = (int)(ratio * grid_height);
    
    int y = GRID_BOTTOM - pixel_h;
    
    // Clamp to ensure we don't draw over header/footer
    if (y < GRID_TOP + 1) y = GRID_TOP + 1;
    if (y > GRID_BOTTOM - 1) y = GRID_BOTTOM - 1;
    
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
    vga_clear_screen(COLOR_BLACK);
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






