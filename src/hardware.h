
#ifndef HARDWARE_H
#define HARDWARE_H

#include <stdint.h> 
#include <stdbool.h> 

// ============================================================================
// DE10-Lite Memory-Mapped Peripheral Base Addresses
// ============================================================================
#define GPIO_BASE                    0x040000E0
#define PUSH_BUTTON_BASE_ADDR        0x040000D0
#define LED_BASE_ADDR                0x04000000
#define SEV_SEG_DISPLAY_BASE_ADDR    0x04000050
#define SWITCH_BASE_ADDR             0x04000010

// ============================================================================
// GPIO Pin Definitions for AD7705 SPI
// ============================================================================
#define SPI_CS_PIN        (1 << 0)   // GPIO_[0] - Chip Select (directly active low)
#define SPI_SCK_PIN       (1 << 1)   // GPIO_[1] - SPI Clock
#define SPI_MOSI_PIN      (1 << 2)   // GPIO_[2] - Master Out, Slave In (AD7705 DIN)
#define SPI_MISO_PIN      (1 << 3)   // GPIO_[3] - Master In, Slave Out (AD7705 DOUT)
#define ADC_DRDY_PIN      (1 << 4)   // GPIO_[4] - Data Ready (active low)
#define ADC_RST_PIN       (1 << 5)   // GPIO_[5] - Reset (active low)

// ============================================================================
// GPIO Register Pointers
// ============================================================================
#define pGPIO_DATA          ((volatile uint32_t *) (GPIO_BASE + 0))
#define pGPIO_DIRECTION     ((volatile uint32_t *) (GPIO_BASE + 4))

// ============================================================================
// Other Peripheral Pointers
// ============================================================================
#define pPUSH_BUTTONS       ((volatile uint32_t *) PUSH_BUTTON_BASE_ADDR)
#define pSWITCHES           ((volatile uint32_t *) SWITCH_BASE_ADDR)
#define pLEDS               ((volatile uint32_t *) LED_BASE_ADDR)

// ============================================================================
// Function Prototypes
// ============================================================================
void set_leds(int led_mask);
int get_sw(void);
int get_btn(void);
void set_display(int display_number, int value);

#endif // HARDWARE_H