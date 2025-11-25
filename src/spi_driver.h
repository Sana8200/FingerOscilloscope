/**
 * spi_driver.h - Bit-banged SPI driver for AD7705 ADC
 * SPI Mode 3: CPOL=1, CPHA=1
 */

#ifndef SPI_DRIVER_H
#define SPI_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

/**
 * Initialize SPI GPIO pins and set idle state
 */
void spi_init(void);

/**
 * Select chip (CS low)
 */
void spi_select_chip(void);

/**
 * Deselect chip (CS high)
 */
void spi_deselect_chip(void);

/**
 * Control hardware reset pin
 * @param high: true = release reset, false = assert reset
 */
void spi_reset_pin(bool high);

/**
 * Blocking wait for DRDY to go low
 * @return true if ready, false if timeout
 */
bool spi_wait_for_ready(void);

/**
 * Non-blocking check if DRDY is low
 */
bool spi_is_ready(void);

/**
 * Transfer one byte (full duplex)
 * @param byte_out: byte to send
 * @return: byte received
 */
uint8_t spi_transfer_byte(uint8_t byte_out);

/**
 * Reset SPI interface by sending 32 ones
 * Call this before AD7705 initialization
 */
void spi_interface_reset(void);

#endif // SPI_DRIVER_H