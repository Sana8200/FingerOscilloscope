#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H
#include <math.h> 
#include <stdint.h>
#include <stdbool.h>

// Screen Definitions 
// 320x240 effectively, horizontal resolution and vertical resolution. 
// The required data output rate (pixel clock) here is reduced(instead of using 640x480). Now timing is more likely  manageable for the tight loops and simple timers used in the vga_driver.c.
// The framebuffer: is a block of RAM used to store the screen image.
// -> For 320x240 using 3-bit color (one byte per pixel, as defined by Color), the memory needed is 320×240 bytes, which equals 76,800 bytes (about 75 KB). 
// This is a large but manageble for in RAM on embedded systems.
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240
#define FRAMEBUFFER_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT)

// 3-bit color definition (R, G, B in bits 2, 1, 0)
typedef enum {
    COLOR_BLACK = 0b000,
    COLOR_BLUE  = 0b001,
    COLOR_GREEN = 0b010,
    COLOR_CYAN  = 0b011,
    COLOR_RED   = 0b100,
    COLOR_MAGENTA = 0b101,
    COLOR_YELLOW = 0b110,
    COLOR_WHITE = 0b111
} Color;

// Framebuffer: Stores one Color byte per pixel (320 * 240 = 76,800 bytes)
extern Color framebuffer[FRAMEBUFFER_SIZE];

/* ********** Waveform Data Structures for Oscilloscope ********** */

// Buffer size matches screen width for a clean 1-to-1 pixel plot
#define WAVEFORM_BUFFER_SIZE SCREEN_WIDTH 

// Global circular buffer to hold the latest 16-bit ADC samples
extern volatile uint16_t waveform_data[WAVEFORM_BUFFER_SIZE];

// Global index indicating where the next sample should be written
extern volatile uint16_t waveform_write_index;
 /* ********** Drawing Function Prototypes ********** */

void set_pixel(int x, int y, Color color);
void draw_waveform();
void clear_screen(Color color);

#endif // FRAMEBUFFER_H
