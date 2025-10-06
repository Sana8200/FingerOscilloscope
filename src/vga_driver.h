#ifndef VGA_DRIVER_H
#define VGA_DRIVER_H
#include "framebuffer.h" // Now includes the header directly from the same src folder

// VGA Standard Timing (Simplified for 320x240)
#define PIXEL_CLOCK_DELAY 0.125f // Time to output one pixel (in microseconds, ~125 ns)

#define H_FRONT_PORCH_PIXELS 16
#define H_SYNC_PULSE_PIXELS  96
#define H_BACK_PORCH_PIXELS  48
#define H_TOTAL_PIXELS (SCREEN_WIDTH + H_FRONT_PORCH_PIXELS + H_SYNC_PULSE_PIXELS + H_BACK_PORCH_PIXELS)

#define V_FRONT_PORCH_LINES  10
#define V_SYNC_PULSE_LINES   2
#define V_BACK_PORCH_LINES   33
#define V_TOTAL_LINES (SCREEN_HEIGHT + V_FRONT_PORCH_LINES + V_SYNC_PULSE_LINES + V_BACK_PORCH_LINES)

/**
 * @brief The main VGA display loop. Reads the framebuffer and outputs 
 * the signal via GPIO with precise timing.
 */
void vga_display_loop();
// The hardware functions are now declared in main.c, but we 
// still declare them here so vga_driver.c so we know they exist!
extern void GPIO_set_pixel_data(Color color);
extern void GPIO_set_hsync(int state);
extern void GPIO_set_vsync(int state);
extern void delay_us(float us);

#endif // // VGA_DRIVER_H
