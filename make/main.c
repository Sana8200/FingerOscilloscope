#include "vga_driver.h"
#include "hardware.h"
#include <stdint.h>
#include <stdlib.h> // For abs()

// These functions must be provided by your boot.S or another assembly file
// We include them here so the linker can find them.
extern void delay_cycles(int cycles);

// We need our own abs() function because we compile with -nostdlib
int abs(int n) {
    if (n < 0) {
        return -n;
    }
    return n;
}

void handle_interrupt(unsigned cause){

}

/**
 * @brief Main program entry point for testing the SPI driver.
 *
 * This program performs a loopback test. You must connect the MOSI pin
 * (GPIO Pin 3) to the MISO pin (GPIO Pin 4) with a jumper wire.
 * The program will continuously send an incrementing pattern and display
 * the received data on the on-board LEDs.
 */
int main() {

    // 1. Initialize the SPI driver. This sets up the GPIO pins.
    SPI_init();

    uint8_t byte_to_send = 0x01;
    uint8_t received_byte = 0;

    // 2. Enter an infinite loop to continuously test the SPI transfer.
    while (1) {
        // Pull the Chip Select line LOW to start the transaction.
        // In a real application, this would select the slave device.
        *pGPIO_DATA &= ~SPI_CS_N_PIN;

        // Transfer the byte. The value sent on MOSI should be looped back
        // to MISO and read by this function.
        received_byte = SPI_transfer_byte(byte_to_send);

        // Pull the Chip Select line HIGH to end the transaction.
        *pGPIO_DATA |= SPI_CS_N_PIN;

        // Display the byte we received back on the 8 rightmost LEDs.
        *pLEDS = received_byte;

        // Prepare the next pattern to send.
        // This will create a nice shifting pattern on the LEDs if it works.
        byte_to_send++;
        if (byte_to_send == 0) {
            byte_to_send = 0x01; // Restart the pattern
        }

        // Add a delay to make the LED changes visible to the human eye.
        for (volatile int i = 0; i < 100000; i++);
    }

    return 0; // Never reached
}
