#ifndef AD7705DRIVER_H // #ifndef checks if AD7705DRIVER_H is not defined, and if not, then defines it and includes the code.
#define AD7705DRIVER_H

/* ========================================================================= */
/* --- SINGLE HEADER FOR AD7705 DRIVER AND PERIPHERALS --- */
/* This file defines all hardware addresses, pin assignments, and function   */
/* prototypes for the project.                                             */
/* ========================================================================= */

// C header file for a driver for the AD7705. It is a 16-bit analog-to-digital converter (ADC)

// --- Hardware Memory Addresses ---
// These codes define pointers to memory-mapped hardware registers.
// volatile tells the compiler the value can change unexpectedly (hardware changes it), so it shouldn’t optimize access.
// Like in GPIO_DATA_REG points to the data register of the GPIO hardware :)

#define GPIO_DATA_REG  ((volatile unsigned int *) 0x040000e0)
#define GPIO_DIR_REG   ((volatile unsigned int *) 0x040000e4)
#define UART_JTAG_REG  ((volatile unsigned char *) 0x04000040)
#define LED_REG        ((volatile unsigned int *) 0x04000000)

// --- GPIO Pin Assignments ---
// These define specific pins for communication with the AD7705 via SPI.
// 1 << x means a bitmask with only the x-th bit set. For example, CS_PIN corresponds to bit 0.

#define CS_PIN   (1 << 0)  // Chip Select Pin
#define SCK_PIN  (1 << 1)  // Serial Clock Pin
#define MOSI_PIN (1 << 2)  // Master Out, Slave In Pin
#define MISO_PIN (1 << 3)  // Master In, Slave Out Pin
#define DRDY_PIN (1 << 4)  // Data Ready Pin
#define RST_PIN  (1 << 5)  // Hardware Reset Pin


/* ========================================================================= */
/* --- PERIPHERAL FUNCTION PROTOTYPES --- */
/* ========================================================================= */

// These are generic functions for:
// -> Printing strings/numbers
// Delaying execution
// SPI communication setup and sending/receiving bytes

void print_string(const char* str);
void print_integer(unsigned int num);
void delay(int cycles);
void spi_setup(void);
void spi_send_byte(unsigned char byte);
unsigned char spi_receive_byte(void);


/* ========================================================================= */
/* --- AD7705 DRIVER FUNCTION PROTOTYPES --- */
/* ========================================================================= */

// These are specific functions for the AD7705 ADC
// ad7705_init() — initializes the ADC.
// ad7705_wait_for_data_ready() — waits until the ADC has finished conversion.

ad7705_read_data() — reads the converted value from the ADC.
void ad7705_init(void);
void ad7705_wait_for_data_ready(void);
unsigned int ad7705_read_data(void);

#endif // AD7705DRIVER_H

/* In general: This file is a single-header driver for controlling the AD7705 ADC on a microcontroller.
   It defines hardware addresses and pin mappings.
   It declares peripheral functions for SPI communication and basic utilities.
   It declares ADC-specific functions.
   The actual implementation of these functions is in .c file called  AD7705Driver.c */