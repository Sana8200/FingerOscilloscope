/**
 * @file main.c
 * @brief Main application entry point for the mini-oscilloscope project.
 * Handles initialization, the main data acquisition loop, and debugging output.
 */

#include "ad7705_driver.h" // Includes all necessary function prototypes
// Note: This file relies on 'print_string' and 'delay' being defined in utility.c
#include "hardware.h"

// Define LED_BASE_ADDR as a pointer to the LED register (adjust address as needed)


// =============================================================================
// 1. UTILITY FUNCTIONS (Needed only by main.c for debug printing)
// =============================================================================

/**
 * @brief Converts an unsigned integer to a string and prints it via UART.
 * This is a debug utility to view the ADC value.
 * @param num The unsigned integer value to print.
 */
void print_integer(unsigned int num) {
    // Array to hold the ASCII characters of the number (max 5 digits for 65535)
    char buffer[6]; 
    int i = 5;

    // Handle the zero case
    if (num == 0) {
        buffer[--i] = '0';
    } else {
        // Convert digits one by one, right to left
        while (num > 0 && i > 0) {
            buffer[--i] = (char)((num % 10) + '0'); // Get remainder, convert to ASCII
            num /= 10;
        }
    }
    
    // Null terminate the string at the end of the printed digits
    buffer[5] = '\0'; 

    // Print the string starting from where the first digit was placed
    print_string(&buffer[i]);
}


// =============================================================================
// 2. MAIN PROGRAM ENTRY POINT
// =============================================================================

/**
 * @brief The entry point of the application.
 */
int main() {
    unsigned int raw_adc_value;
    
    // 1. Initialize Communication and ADC Chip
    spi_setup(); // Initialize GPIO pins for SPI
    ad7705_init(); // Reset and configure the AD7705 chip

    print_string("AD7705 Driver Initialized. Starting data acquisition loop.\n");

    // 2. Main Oscilloscope/Debug Loop
    while (1) {
        // --- Data Acquisition ---
        // Wait for the AD7705 to complete a conversion and read the 16-bit result.
        raw_adc_value = ad7705_read_data();
        
        // --- Debug Output ---
        // Display the raw reading on the console (UART)
        print_string("Raw ADC Value: ");
        print_integer(raw_adc_value);
        print_string("\n");

        // --- Visual Feedback (Using LEDs for simple testing) ---
        // Use the highest 10 bits of the ADC value to drive the 10 LEDs.
        // This gives a quick visual indication of the input voltage level.
        *LED_BASE_ADDR = raw_adc_value >> 6; 

        // In a final oscilloscope, this section is where you would call:
        // vga_plot_sample(raw_adc_value);
    }

    // Program should never exit the infinite loop
    return 0; 
}