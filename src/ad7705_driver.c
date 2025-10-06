
#include "ad7705_driver.h"

/* ========================================================================= */
/* --- PERIPHERALS (HARDWARE ABSTRACTION LAYER) --- */
/* This file implements the functions that directly interact with the      */
/* DE10-Lite's hardware registers.                                         */
/* ========================================================================= */

void print_string(const char* str) {
    while (*str) {
        *UART_JTAG_REG = *str;
        str++;
    }
}

void print_integer(unsigned int num) {
    char buffer[11];
    int i = 0;

    if (num == 0) {
        *UART_JTAG_REG = '0';
        return;
    }

    while (num > 0) {
        buffer[i] = (num % 10) + '0';
        num /= 10;
        i++;
    }

    while (i > 0) {
        i--;
        *UART_JTAG_REG = buffer[i];
    }
}

void delay(int cycles) {
    for (volatile int i = 0; i < cycles; i++);
}


void spi_setup(void) {
    // Set SCK, MOSI, CS, and RST pins as outputs.
    // Set MISO pin as an input.
    *GPIO_DIR_REG |= (SCK_PIN | MOSI_PIN | CS_PIN | RST_PIN);
    *GPIO_DIR_REG &= ~MISO_PIN;
}


void spi_send_byte(unsigned char byte) {
    for (int i = 0; i < 8; i++) {
        // 1. Clock Low
        *GPIO_DATA_REG &= ~SCK_PIN;

        // 2. Set Data (MOSI)
        if (byte & 0x80) {
            *GPIO_DATA_REG |= MOSI_PIN;
        } else {
            *GPIO_DATA_REG &= ~MOSI_PIN;
        }
        byte <<= 1;

        // 3. Clock High (ADC reads the data bit)
        *GPIO_DATA_REG |= SCK_PIN;
    }
}



unsigned char spi_receive_byte(void) {
    unsigned char received_byte = 0;
    for (int i = 0; i < 8; i++) {
        // 1. Clock Low
        *GPIO_DATA_REG &= ~SCK_PIN;
        
        // 2. Clock High (ADC presents its data bit)
        *GPIO_DATA_REG |= SCK_PIN;

        // 3. Read Data (MISO)
        received_byte <<= 1;
        if (*GPIO_DATA_REG & MISO_PIN) {
            received_byte |= 1;
        }
    }
    return received_byte;
}