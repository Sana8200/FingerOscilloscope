#ifndef AD7705DRIVER_H
#define AD7705DRIVER_H

/* ========================================================================= */
/* --- SINGLE HEADER FOR AD7705 DRIVER AND PERIPHERALS --- */
/* This file defines all hardware addresses, pin assignments, and function   */
/* prototypes for the project.                                             */
/* ========================================================================= */



// --- Hardware Memory Addresses ---
#define GPIO_DATA_REG  ((volatile unsigned int *) 0x040000e0)
#define GPIO_DIR_REG   ((volatile unsigned int *) 0x040000e4)
#define UART_JTAG_REG  ((volatile unsigned char *) 0x04000040)
#define LED_REG        ((volatile unsigned int *) 0x04000000)




// --- GPIO Pin Assignments ---
#define CS_PIN   (1 << 0)  // Chip Select Pin
#define SCK_PIN  (1 << 1)  // Serial Clock Pin
#define MOSI_PIN (1 << 2)  // Master Out, Slave In Pin
#define MISO_PIN (1 << 3)  // Master In, Slave Out Pin
#define DRDY_PIN (1 << 4)  // Data Ready Pin
#define RST_PIN  (1 << 5)  // Hardware Reset Pin


/* ========================================================================= */
/* --- PERIPHERAL FUNCTION PROTOTYPES --- */
/* ========================================================================= */

void print_string(const char* str);
void print_integer(unsigned int num);
void delay(int cycles);
void spi_setup(void);
void spi_send_byte(unsigned char byte);
unsigned char spi_receive_byte(void);


/* ========================================================================= */
/* --- AD7705 DRIVER FUNCTION PROTOTYPES --- */
/* ========================================================================= */

void ad7705_init(void);
void ad7705_wait_for_data_ready(void);
unsigned int ad7705_read_data(void);

#endif // AD7705DRIVER_H