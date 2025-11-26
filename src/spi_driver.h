/**
 * spi_driver.h - Bit-banged SPI driver for AD7705 ADC
 * SPI Mode 3: CPOL=1, CPHA=1
 */

#ifndef SPI_DRIVER_H
#define SPI_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

void spi_init(void);
void spi_select_chip(void);
void spi_deselect_chip(void);
void spi_reset_pin(bool high);
bool spi_wait_for_ready(void);
bool spi_is_ready(void);
uint8_t spi_transfer_byte(uint8_t byte_out);
void spi_interface_reset(void);

#endif // SPI_DRIVER_H