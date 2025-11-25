#include "vga_driver.h"
#include <stdint.h>

// --- Fixed Font Data (5x7) ---
// Each character is 5 bytes, each byte represents a column
// Bit 0 = top, Bit 6 = bottom
static const uint8_t font_5x7[][5] = {
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    
    // Special characters
    {0x00, 0x36, 0x36, 0x00, 0x00}, // : (colon) - index 10
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ; (semicolon) - index 11  
    {0x00, 0x00, 0x00, 0x00, 0x00}, // space - index 12
    
    // Letters A-Z (uppercase)
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A - index 13
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B - index 14
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C - index 15
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D - index 16
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E - index 17
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // F - index 18
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // G - index 19
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H - index 20
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I - index 21
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J - index 22
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K - index 23
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L - index 24
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M - index 25
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N - index 26
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O - index 27
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P - index 28
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q - index 29
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R - index 30
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S - index 31
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T - index 32
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U - index 33
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V - index 34
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // W - index 35
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X - index 36
    {0x07, 0x08, 0x70, 0x08, 0x07}, // Y - index 37
    {0x61, 0x51, 0x49, 0x45, 0x43}, // Z - index 38
};




int get_char_idx(char c) {
    // Numbers 0-9
    if (c >= '0' && c <= '9') {
        return c - '0';  // Returns 0-9
    }
    
    // Uppercase letters A-Z
    if (c >= 'A' && c <= 'Z') {
        return 13 + (c - 'A');  // Returns 13-38
    }
    
    // Lowercase letters a-z (map to uppercase)
    if (c >= 'a' && c <= 'z') {
        return 13 + (c - 'a');  // Returns 13-38
    }
    
    // Special characters
    switch(c) {
        case ':': return 10;
        case ';': return 11;
        case ' ': return 12;
        default: return 12;  // Default to space for unknown chars
    }
}


void vga_draw_char(int x, int y,  char c, uint8_t color) {
    int idx = get_char_idx(c);
    const uint8_t *bitmap = font_5x7[idx];
    
    // Draw each column
    for (int col = 0; col < 5; col++) {
        uint8_t data = bitmap[col];
        // Draw each row in the column
        for (int row = 0; row < 7; row++) {
            if (data & (1 << row)) {
                vga_draw_pixel(x + col, y + row, color);
            }
        }
    }
}

void vga_draw_string1(int x, int y,const char *str, uint8_t color) {
    int cur_x = x;
    int cur_y = y;
    
    while (*str) {
        if (*str == '\n') {
            cur_y += 8;
            cur_x = x;  // Return to start X position
        } else {
            vga_draw_char(cur_x, cur_y, *str, color);
            cur_x += 6;  // Move to next character position
        }
        str++;
    }
}

// Draw integer
void vga_draw_int(int x, int y, int val, uint16_t color) {
    char buf[12];
    int i = 0, neg = 0;
    
    if (val < 0) { neg = 1; val = -val; }
    if (val == 0) buf[i++] = '0';
    else while (val > 0) { buf[i++] = '0' + (val % 10); val /= 10; }
    if (neg) buf[i++] = '-';
    
    while (i > 0) { vga_draw_char(x, y, buf[--i], color); x += 6; }
}

static void draw_float(int x, int y, float val, int dec, uint16_t color) {
    char buf[16];
    int idx = 0;
    
    if (val < 0) { buf[idx++] = '-'; val = -val; }
    
    int ipart = (int)val;
    float fpart = val - ipart;
    
    // Integer part (reversed)
    char ibuf[8];
    int ii = 0;
    if (ipart == 0) ibuf[ii++] = '0';
    else while (ipart > 0) { ibuf[ii++] = '0' + (ipart % 10); ipart /= 10; }
    while (ii > 0) buf[idx++] = ibuf[--ii];
    
    // Decimal part
    if (dec > 0) {
        buf[idx++] = '.';
        for (int d = 0; d < dec; d++) {
            fpart *= 10;
            int digit = (int)fpart;
            buf[idx++] = '0' + digit;
            fpart -= digit;
        }
    }
    buf[idx] = '\0';
    vga_draw_string(x, y, buf, color);
}