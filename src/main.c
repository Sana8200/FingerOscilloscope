#include <stdint.h>
#include <stdbool.h>
#include "hardware.h"
#include "spi_driver.h"
#include "ad7705_driver.h"
#include "vga_driver.h"
#include "timer.h"
#include "dtekv-lib.h"
#include "delay.h"

// Configuration
#define V_MIN  0.0f 
#define V_MAX  3.3f 
#define ERASE_BAR_WIDTH 8 

// Screen Layout Constants (Must match vga_driver.c)
#define LEFT_MARGIN 30
#define RIGHT_MARGIN 10
#define GRAPH_WIDTH (SCREEN_WIDTH - LEFT_MARGIN - RIGHT_MARGIN)

void ftoa_simple(float f, char *buffer) {
    int whole = (int)f;
    int frac = (int)((f - whole) * 10); // 1 decimal place
    if (frac < 0) frac = -frac;
    // Manual conversion since we might lack sprintf
    buffer[0] = '0' + (whole % 10); // Single digit support for 0-9V
    buffer[1] = '.';
    buffer[2] = '0' + (frac % 10);
    buffer[3] = 'V';
    buffer[4] = '\0';
}

int main(void) {
    display_string("\n=== OSCILLOSCOPE START ===\n");

    // Init Hardware
    timer_init(200);
    //spi_init();
    delay_ms(50);
    //ad7705_init(CHN_AIN1);
    delay_ms(100);

    // Setup Screen UI
    vga_clear_screen(COLOR_BLACK);
    vga_draw_grid_axis();
    
    // Draw Header
    vga_draw_string(30, 5, "CH1 =", COLOR_YELLOW); // Label
    vga_draw_string(150, 5, "PRO-SCOPE V1.0", COLOR_BLUE);

    int x_index = 0; // 0 to GRAPH_WIDTH
    int prev_x = LEFT_MARGIN;
    int prev_y = SCREEN_HEIGHT / 2;
    char text_buf[8];

    while (1) {
        // --- Read ADC ---
        float voltage = ad7705_read_voltage(CHN_AIN1);
        display_voltage_7seg(voltage);

        // --- Update Top Status Bar (Every ~32 pixels to reduce flicker) ---
        if (x_index % 32 == 0) {
            // Draw a black box to erase old text
            vga_draw_filled_rect(65, 5, 40, 8, COLOR_BLACK); 
            ftoa_simple(voltage, text_buf);
            vga_draw_string(65, 5, text_buf, COLOR_YELLOW);
        }

        // --- Calculate Coordinates ---
        int current_y = vga_map_voltage(voltage, V_MIN, V_MAX);
        int current_x = LEFT_MARGIN + x_index;

        // --- Erase Future Bar ---
        // Only erase INSIDE the graph area
        int top_y = 20;
        int graph_h = SCREEN_HEIGHT - 40;
        
        int erase_x_start = current_x + 1;
        int erase_x_end = current_x + ERASE_BAR_WIDTH;
        
        for (int ex = erase_x_start; ex <= erase_x_end; ex++) {
            if (ex >= (LEFT_MARGIN + GRAPH_WIDTH)) continue; // Don't erase right border
            
            int wrapped_ex = LEFT_MARGIN + ((ex - LEFT_MARGIN) % GRAPH_WIDTH);
            
            // Draw vertical black line inside graph bounds
            vga_draw_line(wrapped_ex, top_y + 1, wrapped_ex, top_y + graph_h - 1, COLOR_BLACK);
            
            // Re-draw Dashed Grid lines if we erased them
            // Vertical Grid Lines (approx every 35px inside graph)
            if ((wrapped_ex - LEFT_MARGIN) % (GRAPH_WIDTH / 8) == 0) {
                 vga_draw_dashed_line(wrapped_ex, top_y + 1, wrapped_ex, top_y + graph_h - 1, COLOR_GRID_GRAY);
            }
            // Horizontal Grid Lines need to be redrawn pixel by pixel
            for (int i = 1; i < 6; i++) {
                int gy = top_y + (i * (graph_h / 6));
                // Redraw dash pattern logic: (x % 6 < 2)
                if (wrapped_ex % 6 < 2) {
                    vga_draw_pixel(wrapped_ex, gy, COLOR_GRID_GRAY);
                }
            }
        }

        // --- Draw Waveform ---
        if (x_index > 0) {
            vga_draw_line(prev_x, prev_y, current_x, current_y, COLOR_ORANGE);
        } else {
            vga_draw_pixel(current_x, current_y, COLOR_ORANGE);
            // Fix left edge continuity
            prev_x = current_x; 
        }

        // --- Advance ---
        prev_x = current_x;
        prev_y = current_y;
        x_index++;
        
        if (x_index >= GRAPH_WIDTH) {
            x_index = 0;
            prev_x = LEFT_MARGIN; // Reset to left edge
        }
    }
    return 0;
}