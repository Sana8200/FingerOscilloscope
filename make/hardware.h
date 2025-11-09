
#ifndef HARDWARE_H
#define HARDWARE_H
#include <stdint.h> 
#include <stdbool.h> 



// DE10-Lite Memory-Mapped Peripheral Base Addresses
#define GPIO_BASE               0x040000E0
#define PUSH_BUTTON_BASE        0x040000D0
#define LED_BASE                0x04000000
#define SEV_SEG_DISPLAY_BASE    0x04000050
#define SWITCH_BASE             0x04000010



// GPIO Pin Definitions for AD7705 SPI
#define SPI_CS_PIN        (1 << 0)
#define SPI_SCK_PIN       (1 << 1)
#define SPI_MOSI_PIN      (1 << 2)
#define SPI_MISO_PIN      (1 << 3)
#define ADC_DRDY_PIN      (1 << 4)
#define ADC_RST_PIN       (1 << 5)


// Register Definitions (Pointers to Hardware)
#define pGPIO_DATA          ((volatile uint32_t *) (GPIO_BASE + 0))
#define pGPIO_DIRECTION     ((volatile uint32_t *) (GPIO_BASE + 4))
#define pPUSH_BUTTONS       ((volatile uint32_t *) PUSH_BUTTON_BASE)
#define pSWITCHES           ((volatile uint32_t *) SWITCH_BASE)
#define pLEDS               ((volatile uint32_t *) LED_BASE)



#endif // HARDWARE_H