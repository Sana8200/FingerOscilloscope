#include "vga_driver.h"
#include "framebuffer.h" // For framebuffer access

/**
 * @brief The main VGA display loop. Reads the framebuffer and outputs 
 * the signal via GPIO with precise timing.
 */
void vga_display_loop() {
    
    // The main loop runs continuously to refresh the screen
    while (1) {
        
        // --- 1. Vertical Synchronization Phase (VSync) ---
        GPIO_set_vsync(0); // VSync LOW (start of sync pulse)
        delay_us(V_SYNC_PULSE_LINES * H_TOTAL_PIXELS * PIXEL_CLOCK_DELAY);
        GPIO_set_vsync(1); // VSync HIGH (end of sync pulse)

        // --- 2. Vertical Back Porch (HSync timing still required) ---
        for (int line = 0; line < V_BACK_PORCH_LINES; line++) {
            // HSync Pulse + HFront Porch + HBack Porch timing needed here.
            
            GPIO_set_pixel_data(0); 
            delay_us(H_FRONT_PORCH_PIXELS * PIXEL_CLOCK_DELAY);

            GPIO_set_hsync(0); 
            delay_us(H_SYNC_PULSE_PIXELS * PIXEL_CLOCK_DELAY);
            GPIO_set_hsync(1); 

            GPIO_set_pixel_data(0); 
            delay_us(H_BACK_PORCH_PIXELS * PIXEL_CLOCK_DELAY);
        }

        // --- 3. Display Data Phase (Scanning the Framebuffer) ---
        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            
            // a. Horizontal Front Porch 
            GPIO_set_pixel_data(0); 
            delay_us(H_FRONT_PORCH_PIXELS * PIXEL_CLOCK_DELAY);

            // b. Horizontal Sync Pulse (HSync)
            GPIO_set_hsync(0); 
            delay_us(H_SYNC_PULSE_PIXELS * PIXEL_CLOCK_DELAY);
            GPIO_set_hsync(1); 

            // c. Display Data Output
            for (int x = 0; x < SCREEN_WIDTH; x++) {
                Color pixel_color = framebuffer[y][x];
                GPIO_set_pixel_data(pixel_color);
                
                // Wait for exactly one pixel clock period (THE MOST CRITICAL TIMING)
                delay_us(PIXEL_CLOCK_DELAY);
            }

            // d. Horizontal Back Porch 
            GPIO_set_pixel_data(0); 
            delay_us(H_BACK_PORCH_PIXELS * PIXEL_CLOCK_DELAY);
        }

        // --- 4. Vertical Front Porch (HSync timing still required) ---
        for (int line = 0; line < V_FRONT_PORCH_LINES; line++) {
            // HSync Pulse + HFront Porch + HBack Porch timing needed here.
            
            GPIO_set_pixel_data(0); 
            delay_us(H_FRONT_PORCH_PIXELS * PIXEL_CLOCK_DELAY);

            GPIO_set_hsync(0); 
            delay_us(H_SYNC_PULSE_PIXELS * PIXEL_CLOCK_DELAY);
            GPIO_set_hsync(1); 

            GPIO_set_pixel_data(0); 
            delay_us(H_BACK_PORCH_PIXELS * PIXEL_CLOCK_DELAY);
        }

        // Loop repeats, starting a new frame
    }
}

