#include <stdio.h> 
#include <stdint.h>
#include "framebuffer.h" // For drawing functions and screen dimensions
#include "vga_driver.h"  // For the main vga_display_loop()
#include "ad7705_driver.h" // For ADC/SPI functions

typedef enum {
    LIVE_VIEW,
    PAUSED,
    MENU
} OscilloscopeState;

volatile OscilloscopeState current_state = LIVE_VIEW;

// =========================================================
// 1. DTEKV HARDWARE DEFINITIONS (Memory-Mapped Registers)
// =========================================================

// GPIO Registers (Verified from DTEK-V Map)
#define GPIO_DATA_REG (*((volatile unsigned int *)0x40000e00))
#define GPIO_DIRECTION_REG (*((volatile unsigned int *)0x40000e04)) // Assumed offset for Direction/Output Enable

// Timer Registers (Verified from DTEK-V Map)
#define TIMER_LOAD_REG (*((volatile unsigned int *)0x40000200)) // Used to set the counter value (2000 for 15kHz)
#define TIMER_CTRL_REG (*((volatile unsigned int *)0x40000204)) // Assumed offset for control register

// Processor Clock Frequency (30 MHz)
#define CPU_CLOCK_MHZ 30.0f 

// !!! --- CRITICAL: CUSTOMIZE THESE PIN MASKS BASED ON YOUR WIRING --- !!!
// Example Pin Assignments (Adjust these based on your wiring!)
#define PIN_MASK_B     (1 << 0) 
#define PIN_MASK_G     (1 << 1) 
#define PIN_MASK_R     (1 << 2) 
#define PIN_MASK_HSYNC (1 << 3) 
#define PIN_MASK_VSYNC (1 << 4) 
#define CS_PIN         (1 << 5) // Chip Select pin for AD7705 (Example)

// Mask to configure all VGA pins (0-4) as outputs
#define VGA_PINS_MASK (PIN_MASK_B | PIN_MASK_G | PIN_MASK_R | PIN_MASK_HSYNC | PIN_MASK_VSYNC)



// =========================================================
// 2. DTEKV HARDWARE IMPLEMENTATION FUNCTIONS
// =========================================================

void GPIO_set_pixel_data(Color color) {
    unsigned int current_state = GPIO_DATA_REG;
    current_state &= ~(PIN_MASK_R | PIN_MASK_G | PIN_MASK_B);
    if (color & 0b100) { current_state |= PIN_MASK_R; } 
    if (color & 0b010) { current_state |= PIN_MASK_G; } 
    if (color & 0b001) { current_state |= PIN_MASK_B; } 
    GPIO_DATA_REG = current_state;
}

void GPIO_set_hsync(int state) {
    if (state) {
        GPIO_DATA_REG |= PIN_MASK_HSYNC;
    } else {
        GPIO_DATA_REG &= ~PIN_MASK_HSYNC;
    }
}

void GPIO_set_vsync(int state) {
    if (state) {
        GPIO_DATA_REG |= PIN_MASK_VSYNC;
    } else {
        GPIO_DATA_REG &= ~PIN_MASK_VSYNC;
    }
}

void delay_us(float us) {
    volatile unsigned int cycles_to_wait = (unsigned int)(us * CPU_CLOCK_MHZ / 3.0f); 
    while (cycles_to_wait-- > 0) {
        __asm__ volatile ("nop"); 
    }
}

// =========================================================
// 3. Main Program Entry Point
// =========================================================

// Global variable to hold the latest ADC value (used for linking to interrupt logic)
volatile unsigned int latest_adc_value = 0; 

int main() {
    
    // --- PART 1: HARDWARE INITIALIZATION ---

    // 1. Configure the VGA GPIO pins (0-4) as OUTPUTS
    GPIO_DIRECTION_REG |= VGA_PINS_MASK;
    
    // 2. Configure the Timer for 15 kHz sampling rate (30 MHz / 15000 = 2000)
    *TIMER_LOAD_REG = 2000; 
    
    // 3. Enable Timer Interrupt (Assumes 0x01 is the bitmask for enable/start)
    // This allows handle_interrupt.c to run at 15kHz
    *TIMER_CTRL_REG = 0x01; 
    
    // 4. Configure the SPI pins and AD7705 (Your original ADC setup)
    spi_setup(); 
    
    // Assuming CMD_SETUP_WRITE and SETUP_CONFIG are defined in ad7705_driver.h
    *GPIO_DATA_REG &= ~CS_PIN;
    spi_send_byte(CMD_SETUP_WRITE);
    spi_send_byte(SETUP_CONFIG);
    *GPIO_DATA_REG |= CS_PIN;

    // --- PART 2: START DISPLAY ---

    // 5. Initial draw of the grid and axes
    draw_waveform(); 

    // 6. Start the continuous, high-speed VGA output loop. (Never returns)
    vga_display_loop(); 

    return 0; 
}