
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
#define SPI_CS_PIN        (1 << 0)    // Chip Select (CS)
#define SPI_SCK_PIN       (1 << 1)    // Serial Clock (SCK)
#define SPI_MOSI_PIN      (1 << 2)    // Master Out Slave In (MOSI)
#define SPI_MISO_PIN      (1 << 3)    // Master In Slave Out (MISO)
#define ADC_DRDY_PIN      (1 << 4)    // Data Ready (DRDY)
#define ADC_RST_PIN       (1 << 5)    // Reset (RST)


// Register Definitions (Pointers to Hardware)
#define pGPIO_DATA          ((volatile uint32_t *) (GPIO_BASE + 0))   // Data Register
#define pGPIO_DIRECTION     ((volatile uint32_t *) (GPIO_BASE + 4))   // Direction Register
#define pPUSH_BUTTONS       ((volatile uint32_t *) PUSH_BUTTON_BASE)  // Push Buttons Register
#define pSWITCHES           ((volatile uint32_t *) SWITCH_BASE)       // Switches Register
#define pLEDS               ((volatile uint32_t *) LED_BASE)          // LEDs Register



#endif // HARDWARE_H