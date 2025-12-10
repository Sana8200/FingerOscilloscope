#include "vga_driver.h"

#define GRID_X(i) (GRAPH_X + ((i) * GRAPH_W) / GRID_DIV_X)
#define GRID_Y(i) (GRAPH_Y + ((i) * GRAPH_H) / GRID_DIV_Y)


static int abs(int x) {
    return (x < 0) ? -x : x;
}

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


void vga_draw_line(int x0, int y0, int x1, int y1, uint8_t color, bool dashed) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int stepX = (x0 < x1) ? 1 : -1; 
    int stepY = (y0 < y1) ? 1 : -1;
    int err = dx - dy, e2, count = 0;
    while (1) {
        if (!dashed || (count++ % 5) < 2) { 
            vga_draw_pixel(x0, y0, color);
        }
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += stepX; }
        if (e2 < dx)  { err += dx; y0 += stepY; }
    }
}


void vga_draw_rect(int x, int y, int w, int h, uint8_t color) {
    vga_draw_line(x, y, x + w - 1, y, color, false);        
    vga_draw_line(x, y + h - 1, x + w - 1, y + h - 1, color, false); 
    vga_draw_line(x, y, x, y + h - 1, color, false);       
    vga_draw_line(x + w - 1, y, x + w - 1, y + h - 1, color, false); 
}


void vga_draw_filled_rect(int x, int y, int w, int h, uint8_t color) {
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > SCREEN_WIDTH) { w = SCREEN_WIDTH - x; }
    if (y + h > SCREEN_HEIGHT) { h = SCREEN_HEIGHT - y; }  
    for (int row = 0; row < h; row++) {
        volatile uint8_t *line = &pVGA_PIXEL_BUFFER[(y + row) * SCREEN_WIDTH + x];
        // filling that row (it doesn't calculate y coordinates each time)
        for (int col = 0; col < w; col++) {
            line[col] = color;
        }
    }
}


void vga_draw_grid(void) {
    vga_draw_rect(GRAPH_X, GRAPH_Y, GRAPH_W, GRAPH_H, COLOR_WHITE);  
    for (int i = 1; i < GRID_DIV_X; i++) {
        vga_draw_line(GRID_X(i), GRAPH_Y + 1, GRID_X(i), GRAPH_Y + GRAPH_H - 2, COLOR_GRID, true);
    }    
    for (int i = 1; i < GRID_DIV_Y; i++) {
        vga_draw_line(GRAPH_X + 1, GRID_Y(i), GRAPH_X + GRAPH_W - 2, GRID_Y(i), COLOR_GRID, true);
    }
}


void vga_draw_display_info(float voltage, float v_max, float v_min, int gain, int sample_rate) {
    vga_draw_filled_rect(0, 0, SCREEN_WIDTH, TOP_MARGIN - 2, COLOR_BLACK);  
      
    vga_draw_string(4, 5, "CH1:", COLOR_YELLOW);
    vga_draw_float(30, 5, voltage, COLOR_YELLOW);
    vga_draw_char(55, 5, 'V', COLOR_YELLOW);
    
    vga_draw_string(80, 5, "G:", COLOR_CYAN);
    vga_draw_int(93, 5, gain, COLOR_CYAN);
    
    vga_draw_string(115, 5, "R:", COLOR_CYAN);
    vga_draw_int(128, 5, sample_rate, COLOR_CYAN);
    
    vga_draw_string(175, 5, "Max:", COLOR_GREEN);
    vga_draw_float(200, 5, v_max, COLOR_GREEN);
    
    vga_draw_string(250, 5, "Min:", COLOR_RED);
    vga_draw_float(280, 5, v_min, COLOR_RED);
    
    vga_draw_string(GRAPH_X, SCREEN_HEIGHT - 12, "0", COLOR_WHITE);
    vga_draw_string(GRAPH_X + GRAPH_W - 30, SCREEN_HEIGHT - 12, "50ms", COLOR_WHITE);

    vga_draw_string(5, GRAPH_Y - 2,               "3.3", COLOR_WHITE);
    vga_draw_string(5, GRAPH_Y + GRAPH_H/4 - 3,   "2.5", COLOR_WHITE);
    vga_draw_string(5, GRAPH_Y + GRAPH_H/2 - 3,   "1.6", COLOR_WHITE);
    vga_draw_string(5, GRAPH_Y + 3*GRAPH_H/4 - 3, "0.8", COLOR_WHITE);
    vga_draw_string(5, GRAPH_Y + GRAPH_H - 7,     "0.0", COLOR_WHITE); 

    vga_draw_string(90, SCREEN_HEIGHT - 12, "DTEK-V FingerOscilloscope", COLOR_GRID);
}


void vga_update_settings(int gain, int sample_rate) {
    vga_draw_filled_rect(80, 5, 80, 8, COLOR_BLACK);  
    
    vga_draw_string(80, 5, "G:", COLOR_CYAN);
    vga_draw_int(95, 5, gain, COLOR_CYAN);
    
    vga_draw_string(115, 5, "R:", COLOR_CYAN);
    vga_draw_int(130, 5, sample_rate, COLOR_CYAN);
}


void vga_init_scope(int gain, int sample_rate) {
    vga_clear_screen(COLOR_BLACK);
    vga_draw_grid();
    vga_draw_display_info(0.0f, 0.0f, 0.0f, gain, sample_rate);
}


int vga_voltage_to_y(float voltage, float v_min, float v_max) {
    if (voltage < v_min) {voltage = v_min;}
    if (voltage > v_max) {voltage = v_max;}
    
    float percent = (voltage - v_min) / (v_max - v_min);
    int y = GRAPH_Y + GRAPH_H - 1 - (int)(percent * (GRAPH_H - 2));
    
    if (y < GRAPH_Y + 1){ y = GRAPH_Y + 1;}
    if (y > GRAPH_Y + GRAPH_H - 2) {y = GRAPH_Y + GRAPH_H - 2;}
    
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
    
    for (int y = GRAPH_Y + 1; y < GRAPH_Y + GRAPH_H - 1; y++) {
        vga_draw_pixel(x, y, COLOR_BLACK);
    }

    bool is_vert_grid = false;
    for (int i = 1; i < GRID_DIV_X; i++) {
        if (x == GRID_X(i)) {
            is_vert_grid = true;
            break;
        }
    }

    if (is_vert_grid) {
        for (int y = GRAPH_Y + 1; y < GRAPH_Y + GRAPH_H - 1; y++) {
            if ((y - GRAPH_Y) % 5 < 2) { 
                vga_draw_pixel(x, y, COLOR_GRID);
            }
        }
    }

    int col_offset = x - GRAPH_X;
    if (col_offset % 5 < 2) {  
        for (int i = 1; i < GRID_DIV_Y; i++) {
            vga_draw_pixel(x, GRID_Y(i), COLOR_GRID);
        }
    }
}


#define PAUSE_W  150
#define PAUSE_H  80
#define PAUSE_X  ((SCREEN_WIDTH - PAUSE_W) / 2)
#define PAUSE_Y  ((SCREEN_HEIGHT - PAUSE_H) / 2)
void vga_show_paused(void) {
    vga_draw_filled_rect(PAUSE_X, PAUSE_Y, PAUSE_W, PAUSE_H, COLOR_WHITE);
    vga_draw_rect(PAUSE_X, PAUSE_Y, PAUSE_W, PAUSE_H, COLOR_DARK_RED);
    vga_draw_rect(PAUSE_X + 1, PAUSE_Y + 1, PAUSE_W - 2, PAUSE_H - 2, COLOR_DARK_RED);
    vga_draw_string(PAUSE_X + 50, PAUSE_Y + 25, "PAUSED", COLOR_DARK_RED);
    vga_draw_string(PAUSE_X + 15, PAUSE_Y + 50, "Press BTN To Continue", COLOR_DARK_GREEN);
}
void vga_hide_paused(void) {
    vga_draw_filled_rect(PAUSE_X, PAUSE_Y, PAUSE_W, PAUSE_H, COLOR_BLACK); 
    vga_draw_grid();
}


#define FREEZE_BOX_SIZE  8   
#define FREEZE_TR_X  (GRAPH_X + GRAPH_W - FREEZE_BOX_SIZE - 3)
#define FREEZE_TR_Y  (GRAPH_Y + 3)
void vga_show_freeze_indicator(void) {
    vga_draw_filled_rect(FREEZE_TR_X, FREEZE_TR_Y, FREEZE_BOX_SIZE, FREEZE_BOX_SIZE, COLOR_RED);
}
void vga_hide_freeze_indicator(void) {
    vga_draw_filled_rect(FREEZE_TR_X, FREEZE_TR_Y, FREEZE_BOX_SIZE, FREEZE_BOX_SIZE, COLOR_BLACK);
    vga_draw_grid();
}