
#include <stdint.h>


// Starting the header for AD7705 
#ifndef AD7705_DRIVER_H
#define AD7705_DRIVER_H


// Hardware Definitions
// define macros 
#define GPIO_DATA_ADDR ((volatile unsigned int *) 0x040000e0)
#define GPIO_DIR_ADDR  ((volatile unsigned int *) 0x040000e4)
#define UART_JTAG_ADDR ((volatile unsigned char *) 0x04000040)
#define LEDD_ADDR      ((volatile unsigned int *) 0x04000000)


// GPIO Pin Assignments 
#define SCK_PIN  (1 << 0)
#define MOSI_PIN (1 << 1)
#define MISO_PIN (1 << 2)
#define CS_PIN   (1 << 3)


// AD7705 Commands 
#define CMD_SETUP_WRITE 0x10
#define CMD_DATA_READ   0x38
#define SETUP_CONFIG    0x40



// Function Prototypes (public)
// Declaring all the funcitons before using them 
void print_string(const char* str);
void delay(int cycles);
void spi_setup();
void spi_send_byte(unsigned char byte_to_send);
unsigned char spi_receive_byte();
void print_integer(unsigned int num);


#endif  // AD7705_DRIVER_H