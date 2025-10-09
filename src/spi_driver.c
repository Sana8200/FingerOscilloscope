#include "spi_driver.h"
#include "hardware.h" // We need this for the GPIO pointers and Pin definitions

// This function is defined in an Assembly file (e.g., boot.S)
// and provides a short, precise delay.
extern void delay_cycles(int cycles);

// A small delay used between SPI pin changes to ensure the slave device
// has enough time to register the signal. Can be tuned if needed.
#define SPI_DELAY_CYCLES 10

/**
 * @brief Initializes the GPIO pins used for SPI communication.
 */
void SPI_init(void) {
    // 1. Define which pins are outputs and which are inputs.
    // We use the standard names from hardware.h
    const uint32_t output_pins = SPI_CS_N_PIN | SPI_SCK_PIN | SPI_MOSI_PIN | ADC_RST_PIN;
    const uint32_t input_pins = SPI_MISO_PIN | ADC_DRDY_PIN;

    // 2. Configure the GPIO direction register.
    // This is a robust way to set directions: first clear the bits for input pins,
    // then set the bits for output pins, leaving other GPIO pins unaffected.
    *pGPIO_DIRECTION = (*pGPIO_DIRECTION & ~input_pins) | output_pins;

    // 3. Set the initial state of the output pins.
    // For SPI Mode 3, the clock (SCK) idles high. Chip Select (CS) also idles high (inactive).
    *pGPIO_DATA |= SPI_CS_N_PIN;  // Set CS high
    *pGPIO_DATA |= SPI_SCK_PIN;   // Set SCK high
    *pGPIO_DATA |= ADC_RST_PIN;   // Keep the ADC out of reset
}

/**
 * @brief Transfers a single byte to and from the SPI slave device (using SPI Mode 3).
 */
uint8_t SPI_transfer_byte(uint8_t byte_to_send) {
    uint8_t received_byte = 0;

    // We send and receive 8 bits, Most Significant Bit (MSB) first.
    for (int i = 7; i >= 0; i--) {
        // Step 1: Set the MOSI (Master Out) pin to the correct value for the current bit.
        if ((byte_to_send >> i) & 1) {
            *pGPIO_DATA |= SPI_MOSI_PIN; // Set the MOSI pin high for a '1'
        } else {
            *pGPIO_DATA &= ~SPI_MOSI_PIN; // Clear the MOSI pin low for a '0'
        }
        delay_cycles(SPI_DELAY_CYCLES);

        // Step 2: Create the falling edge of the clock (HIGH -> LOW).
        // In SPI Mode 3, the slave device reads the MOSI bit on this falling edge.
        *pGPIO_DATA &= ~SPI_SCK_PIN;
        delay_cycles(SPI_DELAY_CYCLES);

        // Step 3: Read the MISO (Master In) pin while the clock is low.
        received_byte <<= 1; // Make space for the new bit
        if (*pGPIO_DATA & SPI_MISO_PIN) {
            received_byte |= 1; // If the MISO pin is high, set the new bit to 1
        }

        // Step 4: Create the rising edge of the clock (LOW -> HIGH) to return to idle.
        // The slave device will prepare its next MISO bit on this rising edge.
        *pGPIO_DATA |= SPI_SCK_PIN;
        delay_cycles(SPI_DELAY_CYCLES);
    }

    return received_byte;
}
