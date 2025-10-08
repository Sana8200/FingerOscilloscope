/**
 * @file ad7705_driver.h
 * @brief Defines hardware addresses, pin assignments, and function prototypes
 * for the AD7705 ADC and SPI communication on the DE10-Lite.
 */
#ifndef AD7705DRIVER_H
#define AD7705DRIVER_H

/* ========================================================================= */
/* --- HARDWARE ABSTRACTION LAYER (H.A.L) --- */
/* Defines memory addresses and pin masks specific to the DE10-Lite GPIO.    */
/* ========================================================================= */

// --- Memory Addresses ---
// These pointers are volatile to prevent the compiler from optimizing out register access.
#define GPIO_PORT_DATA          ((volatile unsigned int *) 0x040000e0)
#define GPIO_PORT_DIRECTION     ((volatile unsigned int *) 0x040000e4)
#define JTAG_UART_DATA          ((volatile unsigned int *) 0x04000040) // Address for console output
#define LED_REG_ADDR            ((volatile unsigned int *) 0x04000000) // Address for the LEDs (used in main.c)

// --- GPIO Pin Assignments ---
// Pin 1=CS, Pin 2=SCK, Pin 3=DIN, Pin 4=DOUT, Pin 5=DRDY, Pin 6=RST
#define ADC_CS_PIN              (1 << 0)  // Chip Select (CS) - Output
#define SPI_SCK_PIN             (1 << 1)  // Serial Clock (SCK) - Output
#define SPI_DIN_PIN             (1 << 2)  // Data In (MOSI from MCU) - Output
#define SPI_DOUT_READ           (1 << 3)  // Data Out (MISO to MCU) - Input
#define ADC_DRDY_PIN            (1 << 4)  // Data Ready (DRDY) - Input
#define ADC_RST_PIN             (1 << 5)  // Hardware Reset (RST) - Output

// --- AD7705 Pin Control Macros ---
#define CS_LOW()                (*GPIO_PORT_DATA &= ~ADC_CS_PIN)
#define CS_HIGH()               (*GPIO_PORT_DATA |= ADC_CS_PIN)
#define SCK_LOW()               (*GPIO_PORT_DATA &= ~SPI_SCK_PIN)
#define SCK_HIGH()              (*GPIO_PORT_DATA |= SPI_SCK_PIN)
#define DIN_LOW()               (*GPIO_PORT_DATA &= ~SPI_DIN_PIN)
#define DIN_HIGH()              (*GPIO_PORT_DATA |= SPI_DIN_PIN)
#define RST_LOW()               (*GPIO_PORT_DATA &= ~ADC_RST_PIN)
#define RST_HIGH()              (*GPIO_PORT_DATA |= ADC_RST_PIN)
#define DRDY_READ()             (*GPIO_PORT_DATA & ADC_DRDY_PIN)

// --- AD7705 Register and Operation Commands ---
#define REG_CLOCK_WRITE         0x20 // Clock Register Address + Write Flag
#define REG_SETUP_WRITE         0x10 // Setup Register Address + Write Flag
#define REG_DATA_READ           0x38 // Data Register Address + Read Flag

// Configuration Settings (Channel 1, 1kHz update, Gain=1)
#define AD7705_CLOCK_INIT       0x0C 
#define AD7705_SETUP_INIT       0x00 

/* ========================================================================= */
/* --- SYSTEM UTILITY FUNCTION PROTOTYPES (Defined in utility.c) --- */
/* ========================================================================= */

void print_string(const char* str);
void delay(int cycles);


/* ========================================================================= */
/* --- PERIPHERAL FUNCTION PROTOTYPES (Defined in spi_driver.c) --- */
/* ========================================================================= */

void spi_setup(void);
unsigned char spi_transfer(unsigned char tx_byte);


/* ========================================================================= */
/* --- AD7705 DRIVER FUNCTION PROTOTYPES (Defined in ad7705_driver.c) --- */
/* ========================================================================= */

void ad7705_init(void);
void ad7705_hardware_reset(void);
void ad7705_wait_for_data_ready(void);
unsigned int ad7705_read_data(void);


/* ========================================================================= */
/* --- MAIN APPLICATION FUNCTION PROTOTYPES (Defined in main.c) --- */
/* ========================================================================= */

void print_integer(unsigned int num);


#endif // AD7705DRIVER_H