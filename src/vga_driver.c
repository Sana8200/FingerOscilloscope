#include "vga_driver.h"
#include "framebuffer.h"

void vga_display_loop() {
    while (1) {
        draw_waveform(); // update framebuffer

        // Vertical sync pulse
        GPIO_set_vsync(0);
        delay_us(V_SYNC_PULSE_LINES * (SCREEN_WIDTH + H_FRONT_PORCH_PIXELS + H_SYNC_PULSE_PIXELS + H_BACK_PORCH_PIXELS) * PIXEL_CLOCK_DELAY);
        GPIO_set_vsync(1);

        // Vertical back porch
        for (int line = 0; line < V_BACK_PORCH_LINES; line++) {
            GPIO_set_pixel_data(COLOR_BLACK);
            delay_us(H_FRONT_PORCH_PIXELS * PIXEL_CLOCK_DELAY);

            GPIO_set_hsync(0);
            delay_us(H_SYNC_PULSE_PIXELS * PIXEL_CLOCK_DELAY);
            GPIO_set_hsync(1);

            GPIO_set_pixel_data(COLOR_BLACK);
            delay_us(H_BACK_PORCH_PIXELS * PIXEL_CLOCK_DELAY);
        }

        // Display active lines
        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            // Front porch
            GPIO_set_pixel_data(COLOR_BLACK);
            delay_us(H_FRONT_PORCH_PIXELS * PIXEL_CLOCK_DELAY);

            // HSync pulse
            GPIO_set_hsync(0);
            delay_us(H_SYNC_PULSE_PIXELS * PIXEL_CLOCK_DELAY);
            GPIO_set_hsync(1);

            // Active video
            for (int x = 0; x < SCREEN_WIDTH; x++) {
                GPIO_set_pixel_data(framebuffer[y][x]);
                delay_us(PIXEL_CLOCK_DELAY);
            }

            // Back porch
            GPIO_set_pixel_data(COLOR_BLACK);
            delay_us(H_BACK_PORCH_PIXELS * PIXEL_CLOCK_DELAY);
        }

        // Vertical front porch
        for (int line = 0; line < V_FRONT_PORCH_LINES; line++) {
            GPIO_set_pixel_data(COLOR_BLACK);
            delay_us(H_FRONT_PORCH_PIXELS * PIXEL_CLOCK_DELAY);

            GPIO_set_hsync(0);
            delay_us(H_SYNC_PULSE_PIXELS * PIXEL_CLOCK_DELAY);
            GPIO_set_hsync(1);

            GPIO_set_pixel_data(COLOR_BLACK);
            delay_us(H_BACK_PORCH_PIXELS * PIXEL_CLOCK_DELAY);
        }
    }
}
