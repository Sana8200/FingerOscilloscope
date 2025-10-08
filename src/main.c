#include <stdint.h>
#include "framebuffer.h"
#include "vga_driver.h"
#include "ad7705_driver.h"

typedef enum { LIVE_VIEW, PAUSED, MENU } OscilloscopeState;
volatile OscilloscopeState current_state = LIVE_VIEW;

// GPIO definitions
#define GPIO_DATA_REG (*((volatile unsigned int *)0x40000e00))
#define GPIO_DIRECTION_REG (*((volatile unsigned int *)0x40000e04))
#define PIN_MASK_B     (1 << 0)
#define PIN_MASK_G     (1 << 1)
#define PIN_MASK_R     (1 << 2)
#define PIN_MASK_HSYNC (1 << 3)
#define PIN_MASK_VSYNC (1 << 4)
#define CS_PIN         (1 << 5)
#define VGA_PINS_MASK  (PIN_MASK_B | PIN_MASK_G | PIN_MASK_R | PIN_MASK_HSYNC | PIN_MASK_VSYNC)

void GPIO_set_pixel_data(Color color) {
    uint32_t state = GPIO_DATA_REG;
    state &= ~(PIN_MASK_R | PIN_MASK_G | PIN_MASK_B);
    if (color & 0b100) state |= PIN_MASK_R;
    if (color & 0b010) state |= PIN_MASK_G;
    if (color & 0b001) state |= PIN_MASK_B;
    GPIO_DATA_REG = state;
}

void GPIO_set_hsync(int state) {
    if (state) GPIO_DATA_REG |= PIN_MASK_HSYNC;
    else       GPIO_DATA_REG &= ~PIN_MASK_HSYNC;
}

void GPIO_set_vsync(int state) {
    if (state) GPIO_DATA_REG |= PIN_MASK_VSYNC;
    else       GPIO_DATA_REG &= ~PIN_MASK_VSYNC;
}

void delay_us(float us) {
    volatile unsigned int cycles = (unsigned int)(us * 30.0f / 3.0f);
    while (cycles--) __asm__ volatile ("nop");
}

// Dummy button functions
int button_live_pressed(void)  { return 0; } // Replace with actual GPIO read
int button_pause_pressed(void) { return 0; }
int button_menu_pressed(void)  { return 0; }

// Button handler
void handle_buttons(void) {
    if (button_live_pressed()) current_state = LIVE_VIEW;
    if (button_pause_pressed()) current_state = PAUSED;
    if (button_menu_pressed()) current_state = MENU;
}

int main() {
    // VGA pins as outputs
    GPIO_DIRECTION_REG |= VGA_PINS_MASK;

    // SPI setup for ADC
    spi_setup();
    ad7705_init();

    // AD7705 setup (example)
    GPIO_DATA_REG &= ~CS_PIN;
    spi_send_byte(CMD_SETUP_WRITE);
    spi_send_byte(SETUP_CONFIG);
    GPIO_DATA_REG |= CS_PIN;

    // Start VGA loop (blocking)
    vga_display_loop();

    return 0;
}
