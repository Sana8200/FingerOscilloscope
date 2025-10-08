/**
 * @file utility.c
 * @brief Contains low-level hardware communication functions (JTAG UART) 
 * and basic timing utilities required by the main application and drivers.
 */

#include "ad7705_driver.h" // Includes JTAG_UART_DATA address for console output

// =============================================================================
// 1. JTAG UART OUTPUT (Console Printing)
// =============================================================================

/**
 * @brief Sends a single character to the JTAG UART for console output.
 * @param character The character to send.
 */
static void print_char(char character) {
    // We write the character into the low byte of the 32-bit data register 
    // at the JTAG_UART_DATA address (0x04000040).
    *JTAG_UART_DATA = (unsigned int)character; 
}

/**
 * @brief Prints a null-terminated string to the JTAG UART.
 * This function is used for all text output and debugging.
 * @param str The string to print.
 */
void print_string(const char* str) {
    while (*str != '\0') {
        print_char(*str);
        str++;
    }
}

// =============================================================================
// 2. TIMING UTILITIES
// =============================================================================

/**
 * @brief Simple busy-wait loop for creating a delay.
 * The actual time depends on the CPU clock speed. Used to meet timing 
 * requirements for the AD7705 and for simple application delays.
 * @param cycles The number of loop iterations to perform.
 */
void delay(int cycles) {
    // Using 'volatile' ensures the compiler does not optimize this loop away.
    volatile int i;
    for (i = 0; i < cycles; i++) {
        // Busy wait loop
    }
}
