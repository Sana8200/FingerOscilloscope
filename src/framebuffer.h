#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H
#include <math.h> 

// Screen Definitions 
// 320x240 effectively, horizontal resolution and vertical resolution. 
// The required data output rate (pixel clock) here is reduced(instead of using 640x480). Now timing is more likely  manageable for the tight loops and simple timers used in the vga_driver.c.
// The framebuffer: is a block of RAM used to store the screen image.
// -> For 320x240 using 3-bit color (one byte per pixel, as defined by Color), the memory needed is 320×240 bytes, which equals 76,800 bytes (about 75 KB). 
// This is a large but manageble for in RAM on embedded systems.
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

#endif // FRAMEBUFFER_H
