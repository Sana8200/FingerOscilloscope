#include "spi_driver.h"
#include "hardware.h" 
#include "dtekv-lib.h"
#include "delay.h"

static uint32_t pio_output_state;


static void spi_delay(void) {
    delay_ns(500);
}


void spi_init(void) {
    uint32_t direction = *pGPIO_DIRECTION;
 
    // Set GPIO pin directions => Outputs: CS, SCK, MOSI, RST (setting to 1)   Inputs:  MISO, DRDY (setting to 0)   
    direction |= (SPI_CS_PIN | SPI_SCK_PIN | SPI_MOSI_PIN | ADC_RST_PIN) ;
    direction &= ~(SPI_MISO_PIN | ADC_DRDY_PIN) ;
    *pGPIO_DIRECTION = direction;

    // Set initial pin for SPI Mode 3
    pio_output_state = *pGPIO_DATA;
    pio_output_state |= (SPI_CS_PIN | ADC_RST_PIN | SPI_SCK_PIN);
    pio_output_state &= ~SPI_MOSI_PIN;
    *pGPIO_DATA = pio_output_state;  
}



// Transfer one byte over SPI (Mode 3)
uint8_t spi_transfer_byte(uint8_t byte_out) {
    uint8_t byte_in = 0;
    
    for (int i = 0; i < 8; i++) {
        byte_in <<= 1;
        
        pio_output_state &= ~SPI_SCK_PIN;  

        if (byte_out & 0x80) {
            pio_output_state |= SPI_MOSI_PIN;
        } else {
            pio_output_state &= ~SPI_MOSI_PIN;
        }
        byte_out <<= 1;  
        
        *pGPIO_DATA = pio_output_state;
        spi_delay();  
        
        pio_output_state |= SPI_SCK_PIN;  
        *pGPIO_DATA = pio_output_state;
        spi_delay();  
        
        if (*pGPIO_DATA & SPI_MISO_PIN) {
            byte_in |= 0x01;
        }
    }    
    return byte_in;
}


void spi_select_chip(void) {
    pio_output_state &= ~SPI_CS_PIN;
    *pGPIO_DATA = pio_output_state;
    spi_delay();
}
void spi_deselect_chip(void) {
    pio_output_state |= SPI_CS_PIN;
    *pGPIO_DATA = pio_output_state;
    spi_delay();
}


void spi_reset_pin(bool high) {
    if (high) {
        pio_output_state |= ADC_RST_PIN;
    } else {
        pio_output_state &= ~ADC_RST_PIN;
    }
    *pGPIO_DATA = pio_output_state;
}



bool spi_wait_for_ready(void) {
    int timeout = 1000000;

    while (timeout > 0) {
        if ((*pGPIO_DATA & ADC_DRDY_PIN) == 0) {
            return true;  
        }
        timeout--;
    }
    display_string("SPI DRDY timeout!\n");
    return false;
}




void spi_interface_reset(void) {
    spi_select_chip();
    for (int i = 0; i < 4; i++) {
        spi_transfer_byte(0xFF);
    } 
    spi_deselect_chip();
    delay_ms(1);
}