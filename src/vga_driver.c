#include "vga_driver.h"


// helper absolute value function 
static int abs(int x) {
    return (x < 0) ? -x : x;
}


/** 
 * Filles the entire screen with only one color
 * goes through all pixels (320 * 240 = 76800 pixel) and sets the color for each one 
 */
void vga_clear_screen(uint8_t color) {
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        pVGA_PIXEL_BUFFER[i] = color;
    }
}


/**
 * Draws a single pixel at position (x, y) with a desired color
 * checks if position is valid (fits in the screen boundries) before drawing
 */             
void vga_draw_pixel(int x, int y, uint8_t color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        pVGA_PIXEL_BUFFER[y * SCREEN_WIDTH + x] = color;   // row*width + column 
    }
}


/**
 * Draw a line from (x0, y0) to (x1, y1) using Bresenham's line drawing algorithm
 */
void vga_draw_line(int x0, int y0, int x1, int y1, uint8_t color) {
    // Distance between points (pixels)
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    // Direction between the points to move down, up, left, right (step directions +1 or -1)
    int stepX = (x0 < x1) ? 1 : -1;
    int stepY = (y0 < y1) ? 1 : -1;
    // Dicision error (error term) for the algorithm, chooses when to move in x or y
    int e = dx - dy;

    while (1) {
        vga_draw_pixel(x0, y0, color);
        // Stop when we reach the end point
        if (x0 == x1 && y0 == y1) break;
        // Calculate next pixel position
        int e2 = 2 * e;
        if (e2 > -dy) { e -= dy; x0 += stepX; }
        if (e2 < dx) { e += dx; y0 += stepY; }
    }
}

/**
 * Draw a dashed line for grid lines (we use this for grid drawing), 3 pixels on , 5 pixels off and repeating this
 * (kind of same algorithm as drawing a complete line just with donditions for pixel ON and OFF)
 */
void vga_draw_dashed_line(int x0, int y0, int x1, int y1, uint8_t color) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int stepX = (x0 < x1) ? 1 : -1;
    int stepY = (y0 < y1) ? 1 : -1;
    int e = dx - dy;
    int count = 0;

    while (1) {
        // draw only if count is in first 3 of every 8 pixels 
        if ((count % 8) < 3) {
            vga_draw_pixel(x0, y0, color);
        }
        count++;
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * e;
        if (e2 > -dy) { e -= dy; x0 += stepX; }
        if (e2 < dx) { e += dx; y0 += stepY; }
    }
}


/**
 * Drawing rectangular border (just the border doesn't fill inside)(no boundry checking)
 * slow since drawing pixel by pixel which the calculation y*width + x happens for every single pixel 
 * x horizontal starting position, y vertical starting position
 * w width of rectangle in pixels, h height of rectangle in pixles
 */
void vga_draw_rect(int x, int y, int w, int h, uint8_t color) {
    // top and bottom lines  (-1 because pixel counts starts at 0(index 0))
    for (int i = x; i < x + w; i++) {
        vga_draw_pixel(i, y, color);
        vga_draw_pixel(i, y + h - 1, color);
    }
    // left and right lines 
    for (int i = y; i < y + h; i++) {
        vga_draw_pixel(x, i, color);
        vga_draw_pixel(x + w - 1, i, color);
    }
}
/**
 * Draw filled rectangular border (color inside) (fast since drawing row by row)
 * boundry checking and using clipping (chopping off)
 * x,y top left corner of rectangle
 * w,h width and height 
 */
void vga_draw_filled_rect(int x, int y, int w, int h, uint8_t color) {
    // checking (clippingl)
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > SCREEN_WIDTH) { w = SCREEN_WIDTH - x; }
    if (y + h > SCREEN_HEIGHT) { h = SCREEN_HEIGHT - y; }
    
    // Fill rectangular row by row (changed from pixel by pixel for drawing faster and optimization) (Direct Memory access)
    // calculating the starting address one time per row : (y+row) * width + x 
    for (int row = 0; row < h; row++) {
        volatile uint8_t *line = &pVGA_PIXEL_BUFFER[(y + row) * SCREEN_WIDTH + x];
        // filling that row (it doesn't calculate y coordinates each time)
        for (int col = 0; col < w; col++) {
            line[col] = color;
        }
    }
}



/**
 * Draw the main grid (border and internal lines)
 * this is where we want to display our waveform 
 */
void vga_draw_grid(void) {
    vga_draw_rect(GRAPH_X, GRAPH_Y, GRAPH_W, GRAPH_H, COLOR_WHITE);  // white border around graph
    
    // vertical grid lines 
    for (int i = 1; i < GRID_DIVS_X; i++) {
        int x = GRAPH_X + (i * GRAPH_W) / GRID_DIVS_X;
        vga_draw_line(x, GRAPH_Y + 1, x, GRAPH_Y + GRAPH_H - 2, COLOR_GRID);
    }
    
    // horizontal grid lines 
    for (int i = 1; i < GRID_DIVS_Y; i++) {
        int y = GRAPH_Y + (i * GRAPH_H) / GRID_DIVS_Y;
        vga_draw_dashed_line(GRAPH_X + 1, y, GRAPH_X + GRAPH_W - 2, y, COLOR_GRID);
    }
}


// INITIALIZATION
void vga_init_scope(int gain, int sample_rate) {
    vga_clear_screen(COLOR_BLACK);
    vga_draw_grid();
    vga_draw_labels();
    vga_draw_header(0.0f, 0.0f, 0.0f, gain, sample_rate);
    vga_draw_footer();
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

/**
 * Clear one vertical column and restore grid lines if needed
 */
void vga_clear_column(int x) {
    if (x <= GRAPH_X || x >= GRAPH_X + GRAPH_W - 1) return;
    
    // Clear entire column to black
    for (int y = GRAPH_Y + 1; y < GRAPH_Y + GRAPH_H - 1; y++) {
        vga_draw_pixel(x, y, COLOR_BLACK);
    }
    
    // Check if this column is a vertical grid line
    bool is_vert_grid = false;
    for (int i = 1; i < GRID_DIVS_X; i++) {
        int grid_x = GRAPH_X + (i * GRAPH_W) / GRID_DIVS_X;
        if (x == grid_x) {
            is_vert_grid = true;
            break;
        }
    }
    
    // Restore vertical grid line if needed
    if (is_vert_grid) {
        for (int y = GRAPH_Y + 1; y < GRAPH_Y + GRAPH_H - 1; y++) {
            if ((y - GRAPH_Y) % 8 < 3) {  // Dashed pattern
                vga_draw_pixel(x, y, COLOR_GRID);
            }
        }
    }
    
    // Restore horizontal grid line intersections
    int col_offset = x - GRAPH_X;
    if (col_offset % 8 < 3) {  // Dashed pattern
        for (int i = 1; i < GRID_DIVS_Y; i++) {
            int grid_y = GRAPH_Y + (i * GRAPH_H) / GRID_DIVS_Y;
            vga_draw_pixel(x, grid_y, COLOR_GRID);
        }
    }
}


// Pause Screen 
void vga_show_paused(void) {
    // Draw a box in the center with "PAUSED" text
    int box_w = 150;
    int box_h = 80;
    int box_x = (SCREEN_WIDTH - box_w) / 2;
    int box_y = (SCREEN_HEIGHT - box_h) / 2;
    
    // Dark background
    vga_draw_filled_rect(box_x, box_y, box_w, box_h, COLOR_BLACK);
    
    // Red border
    vga_draw_rect(box_x, box_y, box_w, box_h, COLOR_RED);
    vga_draw_rect(box_x + 1, box_y + 1, box_w - 2, box_h - 2, COLOR_RED);
    
    // "PAUSED" text centered
    vga_draw_string(box_x + 50, box_y + 25, "PAUSED", COLOR_RED);
    
    // "Press BTN" below
    vga_draw_string(box_x + 15, box_y + 50, "Press BTN To Continue", COLOR_WHITE);
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