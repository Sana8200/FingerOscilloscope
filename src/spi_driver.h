#ifndef SPI_DRIVER_H
#define SPI_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

#define GPIO_BASE        0x040000E0


// GPIO Pin Definitions for AD7705 SPI
#define SPI_CS_PIN        (1 << 0)   // GPIO_[0] - Chip Select (directly active low)
#define SPI_SCK_PIN       (1 << 1)   // GPIO_[1] - SPI Clock
#define SPI_MOSI_PIN      (1 << 2)   // GPIO_[2] - Master Out, Slave In (AD7705 DIN)
#define SPI_MISO_PIN      (1 << 3)   // GPIO_[3] - Master In, Slave Out (AD7705 DOUT)
#define ADC_DRDY_PIN      (1 << 4)   // GPIO_[4] - Data Ready (active low)
#define ADC_RST_PIN       (1 << 5)   // GPIO_[5] - Reset (active low)



#define pGPIO_DATA          ((volatile uint32_t *) (GPIO_BASE + 0))
#define pGPIO_DIRECTION     ((volatile uint32_t *) (GPIO_BASE + 4))


void spi_init(void);
void spi_select_chip(void);
void spi_deselect_chip(void);
void spi_reset_pin(bool high);
bool spi_wait_for_ready(void);
uint8_t spi_transfer_byte(uint8_t byte_out);
void spi_interface_reset(void);

#endif // SPI_DRIVER_H