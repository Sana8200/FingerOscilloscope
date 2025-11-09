#ifndef SPI_DRIVER_H
#define SPI_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

void spi_init();
void spi_select_chip();
void spi_deselect_chip();
void spi_reset_pin(bool high);
void spi_wait_for_ready();
uint8_t spi_transfer_byte(uint8_t byte_out);


#endif // SPI_DRIVER_H