#ifndef SPI_DRIVER_H
#define SPI_DRIVER_H

#include <stdint.h>

//=============================================================================
//                     SPI Driver Function Prototypes
//=============================================================================
// This is a "bit-banged" SPI driver, meaning the software manually controls
// the GPIO pins to generate the SPI signals. It is designed to use the
// pin definitions from our hardware.h file (SPI_SCK_PIN, SPI_MOSI_PIN, etc.).

/**
 * @brief Initializes the GPIO pins used for SPI communication.
 *
 * This function must be called once before any other SPI function.
 * It configures the SCK, MOSI, and CS pins as outputs, and the MISO pin
 * as an input. It also sets the initial state of the clock and chip select pins.
 */
void SPI_init(void);

/**
 * @brief Transfers a single byte to and from the SPI slave device.
 *
 * This function sends 8 bits of data from `byte_to_send` on the MOSI line
 * while simultaneously reading 8 bits of data from the MISO line.
 *
 * @param byte_to_send The 8-bit value to be sent to the slave device.
 * @return uint8_t The 8-bit value received from the slave device during the transfer.
 */
uint8_t SPI_transfer_byte(uint8_t byte_to_send);

#endif // SPI_DRIVER_H