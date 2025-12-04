// VGA driver 320x240 pixels, 8-bit color (RGB332)
#include "vga_driver.h"

static uint8_t vertical_grid_x[GRID_DIVS_X];
static uint8_t horizontal_grid_y[GRID_DIVS_Y];



static int my_abs(int x) {
    return (x < 0) ? -x : x;
}

// Important Basic Drawings 
void vga_clear_screen(uint8_t color) {
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        pVGA_PIXEL_BUFFER[i] = color;
    }
}

void vga_draw_pixel(int x, int y, uint8_t color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        pVGA_PIXEL_BUFFER[y * SCREEN_WIDTH + x] = color;
    }
}

void vga_draw_line(int x0, int y0, int x1, int y1, uint8_t color) {
    int dx = my_abs(x1 - x0);
    int dy = my_abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        vga_draw_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx)  { err += dx; y0 += sy; }
    }
}

void vga_draw_dashed_line(int x0, int y0, int x1, int y1, uint8_t color) {
    int dx = my_abs(x1 - x0);
    int dy = my_abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    int count = 0;

    while (1) {
        if ((count % 8) < 3) {
            vga_draw_pixel(x0, y0, color);
        }
        count++;
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx)  { err += dx; y0 += sy; }
    }
}

void vga_draw_rect(int x, int y, int w, int h, uint8_t color) {
    for (int i = x; i < x + w; i++) {
        vga_draw_pixel(i, y, color);
        vga_draw_pixel(i, y + h - 1, color);
    }
    for (int i = y; i < y + h; i++) {
        vga_draw_pixel(x, i, color);
        vga_draw_pixel(x + w - 1, i, color);
    }
}

void vga_draw_filled_rect(int x, int y, int w, int h, uint8_t color) {
    // draw to screen once
    if (x < 0) { 
        w += x; 
        x = 0; 
    }
    if (y < 0) {
        h += y;
        y = 0; 
    }
    if (x + w > SCREEN_WIDTH) {
        w = SCREEN_WIDTH - x;
    }
    if (y + h > SCREEN_HEIGHT) {
        h = SCREEN_HEIGHT - y;
    }
    
    // Direct memory access
    for (int row = 0; row < h; row++) {
        volatile uint8_t *line = &pVGA_PIXEL_BUFFER[(y + row) * SCREEN_WIDTH + x];
        for (int col = 0; col < w; col++) {
            line[col] = color;
        }
    }
}




// Gain and labels 
void vga_draw_grid(void) {
    int div_w = GRAPH_W / GRID_DIVS_X;
    int div_h = GRAPH_H / GRID_DIVS_Y;
    
    // Border
    vga_draw_rect(GRAPH_X, GRAPH_Y, GRAPH_W, GRAPH_H, COLOR_WHITE);
    
    // Vertical grid lines
    for (int i = 1; i < GRID_DIVS_X; i++) {
        int x = GRAPH_X + i * div_w;
        vga_draw_dashed_line(x, GRAPH_Y + 1, x, GRAPH_Y + GRAPH_H - 2, COLOR_GRID);
    }
    
    // Horizontal grid lines
    for (int i = 1; i < GRID_DIVS_Y; i++) {
        int gy = GRAPH_Y + i * div_h;
        vga_draw_dashed_line(GRAPH_X + 1, gy, GRAPH_X + GRAPH_W - 2, gy, COLOR_GRID);
    }
}
void vga_init_grid_cache(void) {
    int div_w = GRAPH_W / GRID_DIVS_X;
    int div_h = GRAPH_H / GRID_DIVS_Y;
    
    for (int i = 0; i < GRID_DIVS_X; i++) {
        vertical_grid_x[i] = GRAPH_X + (i + 1) * div_w;
    }
    
    for (int i = 0; i < GRID_DIVS_Y; i++) {
        horizontal_grid_y[i] = GRAPH_Y + (i + 1) * div_h;
    }
}

// Header and Footer drawing : Showing voltage, gain, sample rate, max, min 
void vga_draw_header_footer_labels(float voltage, float v_max, float v_min, int gain, int sample_rate) {
    vga_draw_filled_rect(0, 0, SCREEN_WIDTH, TOP_MARGIN - 2, COLOR_BLACK);  // Clears header part ( -2 to not clear the grid border)
      
    // CH1 voltage
    vga_draw_string(4, 5, "CH1:", COLOR_YELLOW);
    vga_draw_float(30, 5, voltage, COLOR_YELLOW);
    vga_draw_char(55, 5, 'V', COLOR_YELLOW);
    
    // Gain
    vga_draw_string(80, 5, "G:", COLOR_CYAN);
    vga_draw_int(93, 5, gain, COLOR_CYAN);
    
    // Sample rate
    vga_draw_string(115, 5, "R:", COLOR_CYAN);
    vga_draw_int(128, 5, sample_rate, COLOR_CYAN);
    
    // Max
    vga_draw_string(175, 5, "Max:", COLOR_GREEN);
    vga_draw_float(200, 5, v_max, COLOR_GREEN);
    
    // Min
    vga_draw_string(250, 5, "Min:", COLOR_RED);
    vga_draw_float(280, 5, v_min, COLOR_RED);
    
    // X-axis time labels
    vga_draw_string(GRAPH_X, SCREEN_HEIGHT - 12, "0", COLOR_WHITE);
    vga_draw_string(GRAPH_X + GRAPH_W - 30, SCREEN_HEIGHT - 12, "50ms", COLOR_WHITE);

    // Y-axis voltage labels
    vga_draw_string(5, GRAPH_Y - 2,               "3.3", COLOR_WHITE);
    vga_draw_string(5, GRAPH_Y + GRAPH_H/4 - 3,   "2.5", COLOR_WHITE);
    vga_draw_string(5, GRAPH_Y + GRAPH_H/2 - 3,   "1.6", COLOR_WHITE);
    vga_draw_string(5, GRAPH_Y + 3*GRAPH_H/4 - 3, "0.8", COLOR_WHITE);
    vga_draw_string(5, GRAPH_Y + GRAPH_H - 7,     "0.0", COLOR_WHITE); 

    // footer
    vga_draw_string(90, SCREEN_HEIGHT - 12, "DTEK-V FingerOscilloscope", COLOR_GRID);
}

// Update display for when gain or rate changes 
void vga_update_settings(int gain, int sample_rate) {
    // Just update the gain and rate portion of header
    vga_draw_filled_rect(80, 5, 80, 8, COLOR_BLACK);
    
    vga_draw_string(80, 5, "G:", COLOR_CYAN);
    vga_draw_int(95, 5, gain, COLOR_CYAN);
    
    vga_draw_string(115, 5, "R:", COLOR_CYAN);
    vga_draw_int(130, 5, sample_rate, COLOR_CYAN);
}


// INITIALIZATION
void vga_init_scope(int gain, int sample_rate) {
    vga_clear_screen(COLOR_BLACK);
    vga_draw_grid();
    vga_draw_header_footer_labels(0.0f, 0.0f, 0.0f, gain, sample_rate);
}

// Displaying waveform as screen values 
int vga_voltage_to_y(float voltage, float v_min, float v_max) {
    if (voltage < v_min) voltage = v_min;
    if (voltage > v_max) voltage = v_max;
    
    float percent = (voltage - v_min) / (v_max - v_min);
    int y = GRAPH_Y + GRAPH_H - 1 - (int)(percent * (GRAPH_H - 2));
    
    if (y < GRAPH_Y + 1) y = GRAPH_Y + 1;
    if (y > GRAPH_Y + GRAPH_H - 2) y = GRAPH_Y + GRAPH_H - 2;
    
    return y;
}


void vga_get_graph_area(int *left, int *right, int *top, int *bottom) {
    if (left)   *left   = GRAPH_X + 1;
    if (right)  *right  = GRAPH_X + GRAPH_W - 2;
    if (top)    *top    = GRAPH_Y + 1;
    if (bottom) *bottom = GRAPH_Y + GRAPH_H - 2;
}

void vga_clear_column(int x) {
    if (x <= GRAPH_X || x >= GRAPH_X + GRAPH_W - 1) return;
    
    // Clear column
    for (int y = GRAPH_Y + 1; y < GRAPH_Y + GRAPH_H - 1; y++) {
        vga_draw_pixel(x, y, COLOR_BLACK);
    }
    
    // Check if this column is a vertical grid line (using cache)
    bool is_vert_grid = false;
    for (int i = 0; i < GRID_DIVS_X - 1; i++) {
        if (x == vertical_grid_x[i]) {
            is_vert_grid = true;
            break;
        }
    }
    
    // Restore vertical grid line if needed
    if (is_vert_grid) {
        for (int y = GRAPH_Y + 1; y < GRAPH_Y + GRAPH_H - 1; y++) {
            if ((y - GRAPH_Y) % 8 < 3) {
                vga_draw_pixel(x, y, COLOR_GRID);
            }
        }
    }
    
    // Restore horizontal grid intersections (using cache)
    int col_offset = x - GRAPH_X;
    if (col_offset % 8 < 3) {
        for (int i = 0; i < GRID_DIVS_Y - 1; i++) {
            vga_draw_pixel(x, horizontal_grid_y[i], COLOR_GRID);
        }
    }
}


// Pause Screen 
void vga_show_paused(void) {
    // Draw a box in the center with "PAUSED" text
    int box_w = 200;
    int box_h = 80;
    int box_x = (SCREEN_WIDTH - box_w) / 2;
    int box_y = (SCREEN_HEIGHT - box_h) / 2;
    
    // Dark background
    vga_draw_filled_rect(box_x, box_y, box_w, box_h, COLOR_BLACK);
    
    // Red border
    vga_draw_rect(box_x, box_y, box_w, box_h, COLOR_RED);
    vga_draw_rect(box_x + 1, box_y + 1, box_w - 2, box_h - 2, COLOR_RED);
    
    // "PAUSED" text centered
    vga_draw_string(box_x + 80, box_y + 25, "PAUSED", COLOR_RED);
    
    // "Press BTN" below
    vga_draw_string(box_x + 35, box_y + 50, "Press BTN To Continue", COLOR_WHITE);
}
void vga_hide_paused(void) {
    int box_w = 200;
    int box_h = 80;
    int box_x = (SCREEN_WIDTH - box_w) / 2;
    int box_y = (SCREEN_HEIGHT - box_h) / 2;
    
    // Clear the entire pause box to black
    vga_draw_filled_rect(box_x, box_y, box_w, box_h, COLOR_BLACK);
    
    // Redraw grid lines only (waveform will naturally fill in)
    int div_w = GRAPH_W / GRID_DIVS_X;
    int div_h = GRAPH_H / GRID_DIVS_Y;
    
    // Vertical lines
    for (int i = 1; i < GRID_DIVS_X; i++) {
        int grid_x = GRAPH_X + i * div_w;
        if (grid_x >= box_x && grid_x <= box_x + box_w) {
            vga_draw_dashed_line(grid_x, 
                                (box_y > GRAPH_Y) ? box_y : GRAPH_Y + 1,
                                grid_x, 
                                (box_y + box_h < GRAPH_Y + GRAPH_H) ? box_y + box_h : GRAPH_Y + GRAPH_H - 1,
                                COLOR_GRID);
        }
    }
    
    // Horizontal lines
    for (int i = 1; i < GRID_DIVS_Y; i++) {
        int grid_y = GRAPH_Y + i * div_h;
        if (grid_y >= box_y && grid_y <= box_y + box_h) {
            vga_draw_dashed_line((box_x > GRAPH_X) ? box_x : GRAPH_X + 1,
                                grid_y,
                                (box_x + box_w < GRAPH_X + GRAPH_W) ? box_x + box_w : GRAPH_X + GRAPH_W - 1,
                                grid_y,
                                COLOR_GRID);
        }
    }
}
