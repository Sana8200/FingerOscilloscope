/**
 * vga_driver.c 
 * 
 * Styled after classic HP oscilloscope displays:
 * - Monochrome green phosphor aesthetic
 * - 10x8 division graticule with center crosshairs
 * - Small tick marks on axes
 * - Clean dotted grid pattern
 * - Header/footer information areas
 * 
 * Hardware: DE10-Lite VGA, 320x240, 16-bit buffer (using RGB332 in lower 8 bits)
 */

#include "vga_driver.h"
#include <stdint.h>
#include "font5x7.h"


// ============================================================================
// Layout Constants
// ============================================================================
#define HEADER_HEIGHT       16      // Top info bar
#define FOOTER_HEIGHT       20      // Bottom menu bar  
#define BORDER_WIDTH        1       // Border around graticule

// Graticule area (main oscilloscope display)
#define GRAT_LEFT           1
#define GRAT_TOP            (HEADER_HEIGHT + 1)
#define GRAT_WIDTH          (SCREEN_WIDTH - 2)
#define GRAT_HEIGHT         (SCREEN_HEIGHT - HEADER_HEIGHT - FOOTER_HEIGHT - 2)
#define GRAT_RIGHT          (GRAT_LEFT + GRAT_WIDTH - 1)
#define GRAT_BOTTOM         (GRAT_TOP + GRAT_HEIGHT - 1)

// Grid divisions
#define GRID_DIVISIONS_X    20      // Horizontal divisions
#define GRID_DIVISIONS_Y    15       // Vertical divisions

// Tick mark sizes
#define MAJOR_TICK_SIZE     4       // Large ticks at divisions
#define MINOR_TICK_SIZE     2       // Small ticks (5 per division)
#define TICKS_PER_DIV       5       // Minor ticks between major divisions

// ============================================================================
// Color Palette - Classic Green Phosphor Style
// ============================================================================
// RGB332 format: RRR GGG BB

// Primary oscilloscope colors (green phosphor theme)
#define COLOR_BACKGROUND    0x00    // Pure black
#define COLOR_PHOSPHOR      0x1C    // Bright green (main elements)
#define COLOR_PHOSPHOR_DIM  0x0C    // Dim green (grid)
#define COLOR_PHOSPHOR_VDIM 0x04    // Very dim green (minor grid dots)

// Specific UI colors
#define COLOR_GRID_LINE     0x04    // Very subtle grid
#define COLOR_GRID_DOT      0x08    // Slightly brighter dots at intersections
#define COLOR_AXIS          0x0C    // Center axis lines
#define COLOR_TICK          0x1C    // Tick marks (bright)
#define COLOR_BORDER        0x0C    // Border around graticule
#define COLOR_TEXT          0x1C    // Text (bright green)
#define COLOR_TEXT_DIM      0x0C    // Dimmer text
#define COLOR_WAVEFORM      0x1C    // Waveform trace
#define COLOR_TRIGGER       0x1C    // Trigger marker

// Alternative highlight colors for selected items
#define COLOR_HIGHLIGHT     0x3C    // Brighter green for highlights
#define COLOR_SELECTED      0x1C    // Selected menu item

// ============================================================================
// Oscilloscope State
// ============================================================================
typedef struct {
    uint8_t  channel;           // Current channel (1 or 2)
    float    v_per_div_ch1;     // Volts per division CH1
    float    v_per_div_ch2;     // Volts per division CH2
    float    time_per_div;      // Time per division (ms or us)
    uint8_t  time_unit;         // 0=ms, 1=us
    float    voltage;           // Current voltage reading
    float    v_max;             // Max voltage
    float    v_min;             // Min voltage
    float    v_pp;              // Peak-to-peak
    float    frequency;         // Measured frequency
    uint8_t  coupling;          // 0=DC, 1=AC
    uint8_t  running;           // 0=STOP, 1=RUN
    char     mode[4];           // "XY", "YT", etc.
} ScopeState;

static ScopeState scope = {
    .channel = 1,
    .v_per_div_ch1 = 20.0f,     // 20.0mV
    .v_per_div_ch2 = 5.0f,      // 5.00V
    .time_per_div = 1.0f,
    .time_unit = 0,
    .voltage = 0.0f,
    .v_max = 0.0f,
    .v_min = 0.0f,
    .v_pp = 0.0f,
    .frequency = 0.0f,
    .coupling = 0,
    .running = 1,
    .mode = "XY"
};


// ============================================================================
// Utility Functions
// ============================================================================

int abs(int n) {
    return (n < 0) ? -n : n;
}

// ============================================================================
// Basic Drawing Primitives
// ============================================================================

void vga_clear_screen(uint16_t color) {
    int total = SCREEN_WIDTH * SCREEN_HEIGHT;
    for (int i = 0; i < total; i++) {
        pVGA_PIXEL_BUFFER[i] = color;
    }
}

void vga_draw_pixel(int x, int y, uint16_t color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        pVGA_PIXEL_BUFFER[y * SCREEN_WIDTH + x] = color;
    }
}

// Fast horizontal line
static void hline(int x1, int x2, int y, uint16_t color) {
    if (y < 0 || y >= SCREEN_HEIGHT) return;
    if (x1 > x2) { int t = x1; x1 = x2; x2 = t; }
    if (x1 < 0) x1 = 0;
    if (x2 >= SCREEN_WIDTH) x2 = SCREEN_WIDTH - 1;
    
    volatile uint16_t *p = &pVGA_PIXEL_BUFFER[y * SCREEN_WIDTH + x1];
    for (int x = x1; x <= x2; x++) *p++ = color;
}

// Fast vertical line
static void vline(int x, int y1, int y2, uint16_t color) {
    if (x < 0 || x >= SCREEN_WIDTH) return;
    if (y1 > y2) { int t = y1; y1 = y2; y2 = t; }
    if (y1 < 0) y1 = 0;
    if (y2 >= SCREEN_HEIGHT) y2 = SCREEN_HEIGHT - 1;
    
    volatile uint16_t *p = &pVGA_PIXEL_BUFFER[y1 * SCREEN_WIDTH + x];
    for (int y = y1; y <= y2; y++) {
        *p = color;
        p += SCREEN_WIDTH;
    }
}

void vga_draw_line(int x1, int y1, int x2, int y2, uint16_t color) {
    if (y1 == y2) { hline(x1, x2, y1, color); return; }
    if (x1 == x2) { vline(x1, y1, y2, color); return; }
    
    int dx = abs(x2 - x1), dy = abs(y2 - y1);
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

void vga_draw_box_outline(int x, int y, int w, int h, uint16_t color) {
    hline(x, x + w - 1, y, color);
    hline(x, x + w - 1, y + h - 1, color);
    vline(x, y, y + h - 1, color);
    vline(x + w - 1, y, y + h - 1, color);
}

void vga_draw_filled_box(int x, int y, int w, int h, uint16_t color) {
    for (int row = y; row < y + h; row++) {
        hline(x, x + w - 1, row, color);
    }
}


void vga_draw_string(int x, int y, const char *s, uint16_t color) {
    while (*s) {
        vga_draw_char(x, y, *s++, color);
        x += 6;
    }
}


// ============================================================================
// Graticule Grid Drawing (HP oscilloscope style)
// ============================================================================

void vga_draw_grid(void) {
    int div_w = GRAT_WIDTH / GRID_DIVISIONS_X;
    int div_h = GRAT_HEIGHT / GRID_DIVISIONS_Y;
    int center_x = GRAT_LEFT + GRAT_WIDTH / 2;
    int center_y = GRAT_TOP + GRAT_HEIGHT / 2;
    
    // ========================================
    // Draw dotted grid lines at each division
    // ========================================
    
    // Vertical grid lines (dotted)
    for (int i = 0; i <= GRID_DIVISIONS_X; i++) {
        int x = GRAT_LEFT + i * div_w;
        if (x > GRAT_RIGHT) x = GRAT_RIGHT;
        
        // Skip center line (drawn separately)
        if (i == GRID_DIVISIONS_X / 2) continue;
        
        // Draw dots every 4 pixels
        for (int y = GRAT_TOP; y <= GRAT_BOTTOM; y += 4) {
            vga_draw_pixel(x, y, COLOR_GRID_DOT);
        }
    }
    
    // Horizontal grid lines (dotted)
    for (int i = 0; i <= GRID_DIVISIONS_Y; i++) {
        int y = GRAT_TOP + i * div_h;
        if (y > GRAT_BOTTOM) y = GRAT_BOTTOM;
        
        // Skip center line (drawn separately)
        if (i == GRID_DIVISIONS_Y / 2) continue;
        
        // Draw dots every 4 pixels
        for (int x = GRAT_LEFT; x <= GRAT_RIGHT; x += 4) {
            vga_draw_pixel(x, y, COLOR_GRID_DOT);
        }
    }
    
    // ========================================
    // Draw center crosshairs (brighter, with ticks)
    // ========================================
    
    // Horizontal center line (dotted, brighter)
    for (int x = GRAT_LEFT; x <= GRAT_RIGHT; x += 2) {
        vga_draw_pixel(x, center_y, COLOR_AXIS);
    }
    
    // Vertical center line (dotted, brighter)
    for (int y = GRAT_TOP; y <= GRAT_BOTTOM; y += 2) {
        vga_draw_pixel(center_x, y, COLOR_AXIS);
    }
    
    // ========================================
    // Draw tick marks on center lines
    // ========================================
    
    int minor_w = div_w / TICKS_PER_DIV;
    int minor_h = div_h / TICKS_PER_DIV;
    
    // Ticks on horizontal center line
    for (int i = 0; i <= GRID_DIVISIONS_X; i++) {
        int x = GRAT_LEFT + i * div_w;
        
        // Major tick at division
        for (int t = -MAJOR_TICK_SIZE; t <= MAJOR_TICK_SIZE; t++) {
            vga_draw_pixel(x, center_y + t, COLOR_TICK);
        }
        
        // Minor ticks between divisions
        if (i < GRID_DIVISIONS_X) {
            for (int m = 1; m < TICKS_PER_DIV; m++) {
                int mx = x + m * minor_w;
                for (int t = -MINOR_TICK_SIZE; t <= MINOR_TICK_SIZE; t++) {
                    vga_draw_pixel(mx, center_y + t, COLOR_TICK);
                }
            }
        }
    }
    
    // Ticks on vertical center line
    for (int i = 0; i <= GRID_DIVISIONS_Y; i++) {
        int y = GRAT_TOP + i * div_h;
        
        // Major tick at division
        for (int t = -MAJOR_TICK_SIZE; t <= MAJOR_TICK_SIZE; t++) {
            vga_draw_pixel(center_x + t, y, COLOR_TICK);
        }
        
        // Minor ticks between divisions
        if (i < GRID_DIVISIONS_Y) {
            for (int m = 1; m < TICKS_PER_DIV; m++) {
                int my = y + m * minor_h;
                for (int t = -MINOR_TICK_SIZE; t <= MINOR_TICK_SIZE; t++) {
                    vga_draw_pixel(center_x + t, my, COLOR_TICK);
                }
            }
        }
    }
    
    // ========================================
    // Draw center dot (crosshair intersection)
    // ========================================
    for (int dx = -2; dx <= 2; dx++) {
        for (int dy = -2; dy <= 2; dy++) {
            if (abs(dx) + abs(dy) <= 3) {
                vga_draw_pixel(center_x + dx, center_y + dy, COLOR_TEXT);
            }
        }
    }
    
    // ========================================
    // Draw border around graticule
    // ========================================
    vga_draw_box_outline(GRAT_LEFT - 1, GRAT_TOP - 1, 
                         GRAT_WIDTH + 2, GRAT_HEIGHT + 2, COLOR_BORDER);
}

// ============================================================================
// Header Drawing (Channel info, like "1 ╔20.0mV  2 ╔5.00V")
// ============================================================================

void vga_draw_header(void) {
    // Clear header area
    vga_draw_filled_box(0, 0, SCREEN_WIDTH, HEADER_HEIGHT, COLOR_BACKGROUND);
    
    // Channel 1 info: "1 ╔20.0mV"
    vga_draw_char(4, 4, '1', COLOR_TEXT);
    
    // Small channel indicator box
    vga_draw_filled_box(12, 4, 8, 7, COLOR_TEXT);
    vga_draw_char(13, 4, 'V', COLOR_BACKGROUND);
    
    // Voltage per div
    draw_float(24, 4, scope.v_per_div_ch1, 1, COLOR_TEXT);
    vga_draw_string(60, 4, "mV", COLOR_TEXT_DIM);
    
    // Channel 2 info: "2 ╔5.00V"
    vga_draw_char(90, 4, '2', COLOR_TEXT);
    vga_draw_filled_box(98, 4, 8, 7, COLOR_TEXT);
    vga_draw_char(99, 4, 'V', COLOR_BACKGROUND);
    draw_float(110, 4, scope.v_per_div_ch2, 2, COLOR_TEXT);
    vga_draw_string(146, 4, "V", COLOR_TEXT_DIM);
    
    // Mode indicator (XY, YT, etc.) - center
    vga_draw_string(155, 4, scope.mode, COLOR_TEXT);
    
    // RUN/STOP indicator - right side
    if (scope.running) {
        vga_draw_string(290, 4, "RUN", COLOR_TEXT);
    } else {
        vga_draw_string(285, 4, "STOP", COLOR_TEXT);
    }
}

// ============================================================================
// Footer Drawing (Menu items like HP scope)
// ============================================================================

void vga_draw_footer(void) {
    int y = SCREEN_HEIGHT - FOOTER_HEIGHT;
    
    // Clear footer area
    vga_draw_filled_box(0, y, SCREEN_WIDTH, FOOTER_HEIGHT, COLOR_BACKGROUND);
    
    // Draw separator line
    hline(0, SCREEN_WIDTH - 1, y, COLOR_BORDER);
    
    // Menu items (HP scope style)
    // Each item has label on top, value below (highlighted if selected)
    
    int text_y1 = y + 3;
    int text_y2 = y + 11;
    
    // Item 1: Connect to
    vga_draw_string(4, text_y1, "Connect to", COLOR_TEXT_DIM);
    vga_draw_string(4, text_y2, "HP Plot", COLOR_TEXT);
    
    // Item 2: Factors
    vga_draw_string(60, text_y1, "Factors", COLOR_TEXT_DIM);
    vga_draw_string(60, text_y2, "Off", COLOR_TEXT_DIM);
    vga_draw_string(78, text_y2, "On", COLOR_TEXT);
    
    // Item 3: Resolution
    vga_draw_string(115, text_y1, "Resolution", COLOR_TEXT_DIM);
    vga_draw_string(115, text_y2, "Low", COLOR_TEXT_DIM);
    vga_draw_string(139, text_y2, "High", COLOR_TEXT);
    
    // Item 4: Baud Rate
    vga_draw_string(175, text_y1, "Baud Rate", COLOR_TEXT_DIM);
    vga_draw_string(175, text_y2, "19200", COLOR_TEXT);
    
    // Item 5: Handshake
    vga_draw_string(225, text_y1, "Handshake", COLOR_TEXT_DIM);
    vga_draw_string(225, text_y2, "XON", COLOR_TEXT);
    vga_draw_string(249, text_y2, "DTR", COLOR_TEXT_DIM);
    
    // Item 6: Previous Menu
    vga_draw_string(280, text_y1, "Previous", COLOR_TEXT_DIM);
    vga_draw_string(280, text_y2, "Menu", COLOR_TEXT);
}

// ============================================================================
// Waveform Drawing
// ============================================================================

// Clear only the graticule area (preserve header/footer)
void vga_clear_waveform_area(void) {
    vga_draw_filled_box(GRAT_LEFT, GRAT_TOP, GRAT_WIDTH, GRAT_HEIGHT, COLOR_BACKGROUND);
}

// Draw waveform segment between two ADC samples
void vga_draw_waveform_segment(int x1, uint16_t y1_adc, int x2, uint16_t y2_adc, uint16_t color) {
    // Convert ADC values (0-65535) to screen Y coordinates
    int screen_y1 = GRAT_BOTTOM - ((y1_adc * GRAT_HEIGHT) >> 16);
    int screen_y2 = GRAT_BOTTOM - ((y2_adc * GRAT_HEIGHT) >> 16);
    
    // Clamp to graticule area
    if (screen_y1 < GRAT_TOP) screen_y1 = GRAT_TOP;
    if (screen_y1 > GRAT_BOTTOM) screen_y1 = GRAT_BOTTOM;
    if (screen_y2 < GRAT_TOP) screen_y2 = GRAT_TOP;
    if (screen_y2 > GRAT_BOTTOM) screen_y2 = GRAT_BOTTOM;
    
    vga_draw_line(x1, screen_y1, x2, screen_y2, color);
}

// Erase column and restore grid at that position
void vga_erase_column(int x) {
    if (x < GRAT_LEFT || x > GRAT_RIGHT) return;
    
    // Clear the column
    vline(x, GRAT_TOP, GRAT_BOTTOM, COLOR_BACKGROUND);
    
    int div_w = GRAT_WIDTH / GRID_DIVISIONS_X;
    int div_h = GRAT_HEIGHT / GRID_DIVISIONS_Y;
    int center_x = GRAT_LEFT + GRAT_WIDTH / 2;
    int center_y = GRAT_TOP + GRAT_HEIGHT / 2;
    int minor_w = div_w / TICKS_PER_DIV;
    
    // Restore grid dots on this column
    int col_in_grat = x - GRAT_LEFT;
    
    // Check if on a major division line
    if (col_in_grat % div_w == 0) {
        if (x != center_x) {
            // Dotted vertical line
            for (int y = GRAT_TOP; y <= GRAT_BOTTOM; y += 4) {
                vga_draw_pixel(x, y, COLOR_GRID_DOT);
            }
        }
    }
    
    // Restore horizontal grid dots that intersect this column
    for (int i = 0; i <= GRID_DIVISIONS_Y; i++) {
        int y = GRAT_TOP + i * div_h;
        if (i != GRID_DIVISIONS_Y / 2) {
            // Check if this x should have a dot (every 4 pixels)
            if ((x - GRAT_LEFT) % 4 == 0) {
                vga_draw_pixel(x, y, COLOR_GRID_DOT);
            }
        }
    }
    
    // Restore center axis at this x
    if ((x - GRAT_LEFT) % 2 == 0) {
        vga_draw_pixel(x, center_y, COLOR_AXIS);
    }
    
    // Restore vertical center line dots if on center column
    if (x == center_x) {
        for (int y = GRAT_TOP; y <= GRAT_BOTTOM; y += 2) {
            vga_draw_pixel(x, y, COLOR_AXIS);
        }
        
        // Restore ticks on vertical center line
        for (int i = 0; i <= GRID_DIVISIONS_Y; i++) {
            int y = GRAT_TOP + i * div_h;
            for (int t = -MAJOR_TICK_SIZE; t <= MAJOR_TICK_SIZE; t++) {
                if (x + t >= GRAT_LEFT && x + t <= GRAT_RIGHT) {
                    vga_draw_pixel(x + t, y, COLOR_TICK);
                }
            }
        }
    }
    
    // Restore ticks on horizontal center line
    // Major tick check
    if (col_in_grat % div_w == 0) {
        for (int t = -MAJOR_TICK_SIZE; t <= MAJOR_TICK_SIZE; t++) {
            vga_draw_pixel(x, center_y + t, COLOR_TICK);
        }
    }
    // Minor tick check
    else if (col_in_grat % minor_w == 0) {
        for (int t = -MINOR_TICK_SIZE; t <= MINOR_TICK_SIZE; t++) {
            vga_draw_pixel(x, center_y + t, COLOR_TICK);
        }
    }
}

// Convert ADC value to screen Y coordinate
int vga_adc_to_screen_y(uint16_t adc_value) {
    int y = GRAT_BOTTOM - ((adc_value * GRAT_HEIGHT) >> 16);
    if (y < GRAT_TOP) y = GRAT_TOP;
    if (y > GRAT_BOTTOM) y = GRAT_BOTTOM;
    return y;
}

// Get graticule bounds
void vga_get_waveform_bounds(int *top, int *bottom, int *left, int *right) {
    if (top) *top = GRAT_TOP;
    if (bottom) *bottom = GRAT_BOTTOM;
    if (left) *left = GRAT_LEFT;
    if (right) *right = GRAT_RIGHT;
}

// ============================================================================
// High-Level Functions
// ============================================================================

// Initialize complete oscilloscope display
void vga_scope_init(void) {
    vga_clear_screen(COLOR_BACKGROUND);
    vga_draw_header();
    vga_draw_grid();
    vga_draw_footer();
}

// Update scope state and redraw header
void vga_scope_update_info(uint8_t channel, float voltage, float v_per_div,
                           float time_per_div, float v_max, float v_min) {
    scope.channel = channel;
    scope.voltage = voltage;
    if (channel == 1) {
        scope.v_per_div_ch1 = v_per_div;
    } else {
        scope.v_per_div_ch2 = v_per_div;
    }
    scope.time_per_div = time_per_div;
    scope.v_max = v_max;
    scope.v_min = v_min;
    scope.v_pp = v_max - v_min;
    
    vga_draw_header();
}

void vga_scope_set_trigger(uint16_t level) {
    // Could draw trigger marker on screen
}

void vga_scope_set_frequency(float freq) {
    scope.frequency = freq;
}

void vga_scope_set_running(uint8_t running) {
    scope.running = running;
    vga_draw_header();
}

void vga_scope_set_mode(const char *mode) {
    scope.mode[0] = mode[0];
    scope.mode[1] = mode[1];
    scope.mode[2] = mode[2] ? mode[2] : '\0';
    scope.mode[3] = '\0';
    vga_draw_header();
}