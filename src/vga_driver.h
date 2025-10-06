#ifndef VGA_DRIVER_H
#define VGA_DRIVER_H

#include "framebuffer.h" // Now includes the header directly from the same src folder
// The hardware functions are now declared in main.c, but we 
// still declare them here so vga_driver.c so we know they exist!
extern void GPIO_set_pixel_data(Color color);
extern void GPIO_set_hsync(int state);
extern void GPIO_set_vsync(int state);
extern void delay_us(float us);

#endif // // VGA_DRIVER_H
