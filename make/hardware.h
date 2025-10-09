#ifndef HARDWARE_H
#define HARDWARE_H

// --- Standard Includes ---
#include <stdint.h> // For standard integer types like uint32_t
#include <stdbool.h> // For the boolean type (true/false)

//=============================================================================
//           DE10-Lite Memory-Mapped Peripheral Base Addresses
//=============================================================================
// These are the raw addresses from the memory map. The drivers will
// cast these to volatile pointers for actual use.

// --- On-board General Purpose I/O (GPIO) ---
// Connected to the AD7705 ADC
#define GPIO_BASE               0x040000E0

// --- On-board Push-buttons ---
// Parallel I/O (PIO) for the 2 push-buttons
#define PUSH_BUTTON_BASE        0x040000D0

// --- On-board LEDs ---
// Parallel I/O (PIO) for the 10 red LEDs
#define LED_BASE                0x04000000

// --- On-board 7-Segment Displays ---
// Parallel I/O (PIO) for the 7-segment displays
#define SEV_SEG_DISPLAY_BASE    0x04000050

// --- VGA Controller ---
// The start address of the 320x240 pixel buffer in memory.
// IMPORTANT: Please verify this address is correct for your system design!
#define VGA_PIXEL_BUFFER_BASE   0x08000000


//=============================================================================
//                GPIO Pin Definitions for AD7705 SPI
//=============================================================================
#define SPI_CS_N_PIN            (1 << 0)
#define SPI_SCK_PIN             (1 << 1)
#define SPI_MOSI_PIN            (1 << 2)
#define SPI_MISO_PIN            (1 << 3)
#define ADC_DRDY_PIN            (1 << 4)
#define ADC_RST_PIN             (1 << 5)

//=============================================================================
//              Register Definitions (Pointers to Hardware)
//=============================================================================
#define pGPIO_DATA              ((volatile uint32_t *) (GPIO_BASE + 0))
#define pGPIO_DIRECTION         ((volatile uint32_t *) (GPIO_BASE + 4))
#define pPUSH_BUTTONS           ((volatile uint32_t *) PUSH_BUTTON_BASE)
#define pSWITCHES               ((volatile uint32_t *) SWITCH_BASE)
#define pLEDS                   ((volatile uint32_t *) LED_BASE)
#define pVGA_PIXEL_BUFFER       ((volatile uint16_t*) VGA_PIXEL_BUFFER_BASE)


#endif // HARDWARE_H