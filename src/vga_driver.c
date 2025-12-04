#include "vga_driver.h"

/** Macros for Grid Math
 * GRAPH_W / GRID_DIVS_X -> The width of one grid division (in pixels)
 * (i) * GRAPH_W / GRID_DIVS_X -> How far from the left edge the i-th grid line is
 * GRAPH_X + ... -> Shift that value to the actual screen coordinate
 * same for GRID_Y(i)
 */
#define GRID_X(i) (GRAPH_X + ((i) * GRAPH_W) / GRID_DIV_X)
#define GRID_Y(i) (GRAPH_Y + ((i) * GRAPH_H) / GRID_DIV_Y)


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
 *  Draw a line from (x0, y0) to (x1, y1) using Bresenham's line drawing algorithm
 */
void vga_draw_line(int x0, int y0, int x1, int y1, uint8_t color, bool dashed) {
    // Distance between points (pixels)
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    // Direction between the points to move down, up, left, right (step directions +1 or -1)
    int stepX = (x0 < x1) ? 1 : -1; 
    int stepY = (y0 < y1) ? 1 : -1;
    // Dicision error (error term) for the algorithm, chooses when to move in x or y
    int err = dx - dy, e2, count = 0;

    while (1) {
        // Draw if solid, or if dashed pattern (3 pixels on, 5 off)
        if (!dashed || (count++ % 5) < 2) { 
            vga_draw_pixel(x0, y0, color);
        }
        // Stop when we reach the end point
        if (x0 == x1 && y0 == y1) break;
        // Calculate next pixel position
        e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += stepX; }
        if (e2 < dx)  { err += dx; y0 += stepY; }
    }
}


/**
 * Drawing rectangular border (just the border doesn't fill inside)(no boundry checking)
 * x horizontal starting position, y vertical starting position
 * w width of rectangle in pixels, h height of rectangle in pixles
 */
void vga_draw_rect(int x, int y, int w, int h, uint8_t color) {
    vga_draw_line(x, y, x + w - 1, y, color, false);         // Top
    vga_draw_line(x, y + h - 1, x + w - 1, y + h - 1, color, false); // Bottom
    vga_draw_line(x, y, x, y + h - 1, color, false);         // Left
    vga_draw_line(x + w - 1, y, x + w - 1, y + h - 1, color, false); // Right
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
    
    // vertical grid lines (dashed)
    for (int i = 1; i < GRID_DIV_X; i++) {
        vga_draw_line(GRID_X(i), GRAPH_Y + 1, GRID_X(i), GRAPH_Y + GRAPH_H - 2, COLOR_GRID, true);
    }
    
    // horizontal grid lines (dashed)
    for (int i = 1; i < GRID_DIV_Y; i++) {
        vga_draw_line(GRAPH_X + 1, GRID_Y(i), GRAPH_X + GRAPH_W - 2, GRID_Y(i), COLOR_GRID, true);
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


// helper funciton to update gain and smaple rate every time they change by switches 
void vga_update_settings(int gain, int sample_rate) {
    // Clearing that specific parts values of the header for gain and smaple rate 
    vga_draw_filled_rect(80, 5, 80, 8, COLOR_BLACK);  
    
    vga_draw_string(80, 5, "G:", COLOR_CYAN);
    vga_draw_int(95, 5, gain, COLOR_CYAN);
    
    vga_draw_string(115, 5, "R:", COLOR_CYAN);
    vga_draw_int(130, 5, sample_rate, COLOR_CYAN);
}



// initialization (so for VGA init we'll call it once)
void vga_init_scope(int gain, int sample_rate) {
    vga_clear_screen(COLOR_BLACK);
    vga_draw_grid();
    vga_draw_header_footer_labels(0.0f, 0.0f, 0.0f, gain, sample_rate);
    //display_string("VGA done!");
}



// Displaying waveform as screen values 
int vga_voltage_to_y(float voltage, float v_min, float v_max) {
    // checking for preventing drawing outside of the graph box
    if (voltage < v_min) {voltage = v_min;}
    if (voltage > v_max) {voltage = v_max;}
    
    // calculating the percentage 
    float percent = (voltage - v_min) / (v_max - v_min);
    /** calculating the height in pixels (percent*height) and subtracting 
     * from the bottom of the graph (graph_Y + graph_H) flipping the graph
     * for showing high voltage at the top*/
    int y = GRAPH_Y + GRAPH_H - 1 - (int)(percent * (GRAPH_H - 2));
    
    // ensuring the y is insdie the drawing area and not on the borders 
    if (y < GRAPH_Y + 1){ y = GRAPH_Y + 1;}
    if (y > GRAPH_Y + GRAPH_H - 2) {y = GRAPH_Y + GRAPH_H - 2;}
    
    return y;
}


/**
 * helper function, takes pointers, and filles the excat pixel coordinates of the writable area
 * inside the border
 * Added for preventing flickering on the screen 
 */
void vga_get_graph_area(int *left, int *right, int *top, int *bottom) {
    if (left)   *left   = GRAPH_X + 1;
    if (right)  *right  = GRAPH_X + GRAPH_W - 2;
    if (top)    *top    = GRAPH_Y + 1;
    if (bottom) *bottom = GRAPH_Y + GRAPH_H - 2;
}

/**
 * Clear one vertical column and restore grid lines if needed
 * in osilliscope we want to have the new data, so before drawing the new data point 
 * we are earsing the old data point that was from previous sweep
 * kind of redrawing for clearing the waveform for new data
 */
void vga_clear_column(int x) {
    if (x <= GRAPH_X || x >= GRAPH_X + GRAPH_W - 1) return;  // simple check 
    
    // Clear entire column to black
    for (int y = GRAPH_Y + 1; y < GRAPH_Y + GRAPH_H - 1; y++) {
        vga_draw_pixel(x, y, COLOR_BLACK);
    }
    
    // Check if this column is a vertical grid line
    bool is_vert_grid = false;
    for (int i = 1; i < GRID_DIV_X; i++) {
        if (x == GRID_X(i)) {
            is_vert_grid = true;
            break;
        }
    }
    
    // Restore vertical grid line if needed
    if (is_vert_grid) {
        for (int y = GRAPH_Y + 1; y < GRAPH_Y + GRAPH_H - 1; y++) {
            if ((y - GRAPH_Y) % 5 < 2) {  // Dashed pattern
                vga_draw_pixel(x, y, COLOR_GRID);
            }
        }
    }
    
    // Restore horizontal grid line intersections
    int col_offset = x - GRAPH_X;
    if (col_offset % 5 < 2) {  // Dashed pattern
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
    vga_draw_filled_rect(PAUSE_X, PAUSE_Y, PAUSE_W, PAUSE_H, COLOR_BLACK);
    vga_draw_rect(PAUSE_X, PAUSE_Y, PAUSE_W, PAUSE_H, COLOR_RED);
    vga_draw_rect(PAUSE_X + 1, PAUSE_Y + 1, PAUSE_W - 2, PAUSE_H - 2, COLOR_RED);
    vga_draw_string(PAUSE_X + 50, PAUSE_Y + 25, "PAUSED", COLOR_RED);
    vga_draw_string(PAUSE_X + 15, PAUSE_Y + 50, "Press BTN To Continue", COLOR_WHITE);
}
void vga_hide_paused(void) {
    vga_draw_filled_rect(PAUSE_X, PAUSE_Y, PAUSE_W, PAUSE_H, COLOR_BLACK);  // clearing the box area
    /**
     * For precision we can also redraw the grid lines only inside the box */
    /*
    for (int i = 1; i < GRID_DIV_X; i++) {
        int gx = GRID_X(i);
        if (gx >= PAUSE_X && gx <= PAUSE_X + PAUSE_W)
        vga_draw_line(gx, PAUSE_Y, gx, PAUSE_Y + PAUSE_H, COLOR_GRID, true);
    }

    for (int i = 1; i < GRID_DIV_Y; i++) {
        int gy = GRID_Y(i);
        if (gy >= PAUSE_Y && gy <= PAUSE_Y + PAUSE_H)
        vga_draw_line(PAUSE_X, gy, PAUSE_X + PAUSE_W, gy, COLOR_GRID, true);
    }
    */
   // for now continuing with just redrawing grid 
    vga_draw_grid();
}