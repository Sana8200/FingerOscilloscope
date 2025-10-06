#include "framebuffer.h" // For access to drawing functions (like draw_waveform)
#include "vga_driver.h"  // For access to the VGA loop (vga_display_loop)
#include "ad7705_driver.h"     
#include <stdint.h>
#include <stdio.h> 

/* ************ 1. DTEKV HARDWARE DEFINITIONS (VERIFY THESE ADDRESSES/MASKS) ************ */

// Memory-mapped address of the GPIO Output Data Register (0x40000e00 from DTEK-V table)
#define GPIO_DATA_REG (*((volatile unsigned int *)0x40000e00))
// Hypothetical address for the GPIO Direction Register (CRITICAL: MUST be verified!)
#define GPIO_DIRECTION_REG (*((volatile unsigned int *)0x40000e04)) 
// Processor Clock Frequency (30 MHz from the DTEK-V table)
#define CPU_CLOCK_MHZ 30.0f 

/* ********** 2. DTEKV HARDWARE IMPLEMENTATION FUNCTIONS ********** */
/**
 * @brief Sets the R, G, and B GPIO pins based on the 3-bit color value.
 */
void GPIO_set_pixel_data(Color color) {
    unsigned int current_state = GPIO_DATA_REG;
    current_state &= ~(PIN_MASK_R | PIN_MASK_G | PIN_MASK_B);
    if (color & 0b100) { current_state |= PIN_MASK_R; } // Red (Bit 2)
    if (color & 0b010) { current_state |= PIN_MASK_G; } // Green (Bit 1)
    if (color & 0b001) { current_state |= PIN_MASK_B; } // Blue (Bit 0)
    GPIO_DATA_REG = current_state;
}

/**
 * @brief Sets the Horizontal Sync pin state.
 */
void GPIO_set_hsync(int state) {
    if (state) {
        GPIO_DATA_REG |= PIN_MASK_HSYNC;
    } else {
        GPIO_DATA_REG &= ~PIN_MASK_HSYNC;
    }
}

/**
 * @brief Sets the Vertical Sync pin state.
 */
void GPIO_set_vsync(int state) {
    if (state) {
        GPIO_DATA_REG |= PIN_MASK_VSYNC;
    } else {
        GPIO_DATA_REG &= ~PIN_MASK_VSYNC;
    }
}

/**
 * @brief Creates an approximate time delay in microseconds.
 */
void delay_us(float us) {
    volatile unsigned int cycles_to_wait = (unsigned int)(us * CPU_CLOCK_MHZ / 3.0f); 
    while (cycles_to_wait-- > 0) {
        __asm__ volatile ("nop"); 
    }
}

/* *********** 3. Main Program Entry Point - MERGED ADC AND VGA ********** */

// Global variable to hold the latest ADC value for the drawing function
// This needs to be updated by a separate IRQ/Thread for continuous sampling.
volatile unsigned int latest_adc_value = 0; 

int main() {
    
    // --- PART 1: HARDWARE INITIALIZATION ---

    // 1. Configure the VGA GPIO pins as OUTPUTS
    // This line is essential to enable the VGA pins.
    GPIO_DIRECTION_REG |= VGA_PINS_MASK;
    
    // 2. Configure the SPI pins for the AD7705
    spi_setup(); 
    
    // 3. Configure the AD7705 ADC (Your original code)
    // NOTE: CS_PIN must be defined elsewhere (e.g., ad7705_driver.h or another global file)
    *GPIO_DATA_REG &= ~CS_PIN;       
    spi_send_byte(CMD_SETUP_WRITE);   
    spi_send_byte(SETUP_CONFIG);      
    *GPIO_DATA_REG |= CS_PIN;        

    // --- PART 2: START VGA DISPLAY ---

    // 4. Initial draw of the grid and axes (before entering the high-speed loop)
    draw_waveform(); 

    // 5. Start the continuous, high-speed VGA output loop.
    // WARNING: This function takes control of the CPU and runs forever.
    vga_display_loop(); 

    return 0; // This line will never be reached in a functional embedded system
}



