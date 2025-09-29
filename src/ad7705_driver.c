


#include "ad7705_driver.h"


// C Functions for SPI 

// function to print a string to the console
void print_string(const char* str) {
    while (*str) {
        *UART_JTAG_ADDR = *str;
        str++;
    }
}



// software delay loop
void delay(int cycles) {
    for (volatile int i = 0; i < cycles; i++);
}


void handle_interrupt(unsigned cause) {
}


// Function to set up the GPIO pin directions
void spi_setup() {
    // Set SCLK, MOSI, and CS as outputs, and MISO as an input.
    // We do this by setting the corresponding bits in the direction register.
    *GPIO_DIR_ADDR = (*GPIO_DIR_ADDR | SCK_PIN | MOSI_PIN | CS_PIN) & ~MISO_PIN;
}




// Function to send one byte over SPI
void spi_send_byte(unsigned char byte_to_send) {
    for (int i = 0; i < 8; i++) {
        // 1. Set Clock Low
        *GPIO_DATA_ADDR &= ~SCK_PIN;

        // 2. Set Data Pin (MOSI)
        if (byte_to_send & 0x80) { // Check the most significant bit
            *GPIO_DATA_ADDR |= MOSI_PIN; // Set MOSI high
        } else {
            *GPIO_DATA_ADDR &= ~MOSI_PIN; // Set MOSI low
        }
        byte_to_send <<= 1; // Shift to prepare the next bit

        // 3. Set Clock High (ADC reads the data)
        *GPIO_DATA_ADDR |= SCK_PIN;
    }
}



// Function to receive one byte over SPI
unsigned char spi_receive_byte() {
    unsigned char received_byte = 0;
    for (int i = 0; i < 8; i++) {
        // 1. Set Clock Low
        *GPIO_DATA_ADDR &= ~SCK_PIN;
        
        // 2. Set Clock High (ADC places data on the line)
        *GPIO_DATA_ADDR |= SCK_PIN;

        // 3. Read Data Pin (MISO)
        received_byte <<= 1; // Make space for the new bit
        if (*GPIO_DATA_ADDR & MISO_PIN) {
            received_byte |= 1;
        }
    }
    return received_byte;
}



// Function to convert an unsigned integer to an ASCII string and print it.
void print_integer(unsigned int num) {
    char buffer[11]; // Buffer to hold the string (10 digits for a 32-bit uint + null)
    int i = 0;

    // Handle the special case of 0
    if (num == 0) {
        *UART_JTAG_ADDR = '0';
        return;
    }

    // Convert number to string, but in reverse order
    while (num > 0) {
        // Get the last digit and convert it to its ASCII character value
        // For example, 5 becomes '5' by adding the ASCII value of '0'
        buffer[i] = (num % 10) + '0';
        num = num / 10; // Remove the last digit
        i++;
    }

    // Print the string in the correct (reversed) order
    while (i > 0) {
        i--;
        *UART_JTAG_ADDR = buffer[i];
    }
}