#include "vga_driver.h"

// ==========================================
// MINIMAL 5x7 BITMAP FONT
// ==========================================
// Each byte represents a column of the character. 
// LSB (Bit 0) is the TOP of the character.
static const uint8_t font5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // space
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // "
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // &
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // )
    {0x14, 0x08, 0x3E, 0x08, 0x14}, // *
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // +
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // /
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    {0x00, 0x36, 0x36, 0x00, 0x00}, // :
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ;
    {0x08, 0x14, 0x22, 0x41, 0x00}, // <
    {0x14, 0x14, 0x14, 0x14, 0x14}, // =
    {0x00, 0x41, 0x22, 0x14, 0x08}, // >
    {0x02, 0x01, 0x51, 0x09, 0x06}, // ?
    {0x32, 0x49, 0x79, 0x41, 0x3E}, // @
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
};

static int abs(int n) { return (n < 0) ? -n : n; }

void vga_clear_screen(uint8_t color) {
    int total = SCREEN_WIDTH * SCREEN_HEIGHT;
    // Treating buffer as 8-bit pointer for byte access
    for (int i = 0; i < total; i++) {
        pVGA_PIXEL_BUFFER[i] = color;
    }
}

void vga_draw_pixel(int x, int y, uint8_t color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        pVGA_PIXEL_BUFFER[(y * SCREEN_WIDTH) + x] = color;
    }
}

void vga_draw_line(int x0, int y0, int x1, int y1, uint8_t color) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;
    while (1) {
        vga_draw_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

// Dashed line for Grid (matches scope image style)
void vga_draw_dashed_line(int x0, int y0, int x1, int y1, uint8_t color) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;
    int pixel_count = 0;
    
    while (1) {
        // Dash pattern: 2 pixels on, 4 pixels off
        if (pixel_count % 6 < 2) { 
            vga_draw_pixel(x0, y0, color);
        }
        pixel_count++;

        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void vga_draw_rect(int x, int y, int w, int h, uint8_t color) {
    vga_draw_line(x, y, x + w, y, color);
    vga_draw_line(x, y + h, x + w, y + h, color);
    vga_draw_line(x, y, x, y + h, color);
    vga_draw_line(x + w, y, x + w, y + h, color);
}

void vga_draw_filled_rect(int x, int y, int w, int h, uint8_t color) {
    for (int curr_y = y; curr_y < (y + h); curr_y++) {
        for (int curr_x = x; curr_x < (x + w); curr_x++) {
            vga_draw_pixel(curr_x, curr_y, color);
        }
    }
}

// Text Drawing - CORRECTED ORIENTATION
void vga_draw_char(int x, int y, char c, uint8_t color) {
    if (c < 32 || c > 90) c = 32; // basic bounds check
    int index = c - 32;
    for (int col = 0; col < 5; col++) {
        uint8_t line = font5x7[index][col];
        for (int row = 0; row < 7; row++) {
            if (line & (1 << row)) { 
                // FIXED: 'row' increases downwards. LSB (bit 0) is top pixel.
                vga_draw_pixel(x + col, y + row, color); 
            }
        }
    }
}

void vga_draw_string(int x, int y, const char *str, uint8_t color) {
    while (*str) {
        vga_draw_char(x, y, *str++, color);
        x += 6; // 5px wide + 1px spacing
    }
}

// Draw the Professional Grid UI
void vga_draw_grid_axis(void) {
    // These must align with margins in main.c
    int left_margin = 30; // space for Y-axis text
    int bottom_margin = 20; // space for X-axis text
    int graph_w = SCREEN_WIDTH - left_margin - 10;
    int graph_h = SCREEN_HEIGHT - bottom_margin - 20; // top margin 20
    int top_y = 20;

    // Draw main box
    vga_draw_line(left_margin, top_y, left_margin+graph_w, top_y, COLOR_WHITE); // Top
    vga_draw_line(left_margin, top_y+graph_h, left_margin+graph_w, top_y+graph_h, COLOR_WHITE); // Bottom
    vga_draw_line(left_margin, top_y, left_margin, top_y+graph_h, COLOR_WHITE); // Left
    vga_draw_line(left_margin+graph_w, top_y, left_margin+graph_w, top_y+graph_h, COLOR_WHITE); // Right

    // 2. Draw Dashed Grid
    // Vertical lines
    for (int i = 1; i < 8; i++) {
        int x = left_margin + (i * (graph_w / 8));
        vga_draw_dashed_line(x, top_y + 1, x, top_y + graph_h - 1, COLOR_GRID_GRAY);
    }
    // Horizontal lines
    for (int i = 1; i < 6; i++) {
        int y = top_y + (i * (graph_h / 6));
        vga_draw_dashed_line(left_margin + 1, y, left_margin + graph_w - 1, y, COLOR_GRID_GRAY);
    }

    // 3. Draw Axis Labels (Static for now, matching scope style)
    // Y-Axis
    vga_draw_string(2, top_y, "3.3", COLOR_WHITE);
    vga_draw_string(2, top_y + graph_h/2 - 4, "1.6", COLOR_WHITE);
    vga_draw_string(2, top_y + graph_h - 7, "0.0", COLOR_WHITE);

    // X-Axis (Time)
    vga_draw_string(left_margin, SCREEN_HEIGHT - 12, "0.0", COLOR_WHITE);
    vga_draw_string(SCREEN_WIDTH - 40, SCREEN_HEIGHT - 12, "50ms", COLOR_WHITE);

    // Footer
    vga_draw_string(10, SCREEN_HEIGHT - 8, "Freq: 60Hz", COLOR_GRID_GRAY);
    vga_draw_string(140, SCREEN_HEIGHT - 8, "Time (msec)", COLOR_WHITE);
}

int vga_map_voltage(float voltage, float min_v, float max_v) {
    if (voltage < min_v) voltage = min_v;
    if (voltage > max_v) voltage = max_v;
    
    // Graph area definitions (must match draw_grid_axis)
    int top_y = 20;
    int graph_h = SCREEN_HEIGHT - 20 - 20; 

    float percentage = (voltage - min_v) / (max_v - min_v);
    
    // Invert Y inside the graph box
    // top_y is 3.3V, top_y + graph_h is 0V
    return (top_y + graph_h) - (int)(percentage * graph_h);
}