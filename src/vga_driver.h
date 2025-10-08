#ifndef VGA_DRIVER_H
#define VGA_DRIVER_H

#include "framebuffer.h"

// VGA timing constants
#define PIXEL_CLOCK_DELAY 0.125f  // 125 ns per pixel
#define H_FRONT_PORCH_PIXELS 16
#define H_SYNC_PULSE_PIXELS  96
#define H_BACK_PORCH_PIXELS  48

#define V_FRONT_PORCH_LINES  10
#define V_SYNC_PULSE_LINES   2
#define V_BACK_PORCH_LINES   33

// Functions
void vga_display_loop(void);
extern void GPIO_set_pixel_data(Color color);
extern void GPIO_set_hsync(int state);
extern void GPIO_set_vsync(int state);
extern void delay_us(float us);

#endif
