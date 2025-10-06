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

// Color Definition (3-bit RGB)
// Bit 0: Blue (B), Bit 1: Green (G), Bit 2: Red (R)
typedef unsigned char Color; // 8-bit value, only 3 bits (RGB) are used 

// Global Framebuffer Declaration 
// The actual storage for the screen data is defined in framebuffer.c
extern Color framebuffer[SCREEN_HEIGHT][SCREEN_WIDTH];

/**
 * @brief Sets a single pixel color in the framebuffer.
 */
void set_pixel(int x, int y, Color color);

/**
 * @brief Clears the screen and draws a mock waveform or grid.
 * * In the final project, this will read ADC data and draw the scope trace.
 */
void draw_waveform();

#endif // FRAMEBUFFER_H
