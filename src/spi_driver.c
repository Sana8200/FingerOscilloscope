/**
 * Bit-banged Manual SPI driver for AD7705 ADC : SPI Mode 3: CPOL=1 (clock idle high), CPHA=1 (sample on rising edge)
 * Data is shifted out on falling edge of SCLK, sampled on rising edge of SCLK, MSB first
 */

#include "spi_driver.h"
#include "hardware.h" 
#include "dtekv-lib.h"
#include "delay.h"

// Track output state to prevent read-modify-write errors
static uint32_t pio_output_state;

// Minimum SPI clock half-period delay
// AD7705 max SCLK is 5 MHz but for correction operation 3 MHZ (safe), total-period=1/3 MHz=333.33ns, half-period=333.33/2= 166.77ns
// 500 ns creates a half period of 500ns, period = 500ns x 2 = 1000ns    frequency = 1 MHz  /// we can triple it if needed 
static void spi_delay(void) {
    delay_ns(500);
}


// Initialize SPI GPIO pins
void spi_init(void) {
    //display_string("SPI init start...\n");    
    // Read current direction register
    uint32_t direction = *pGPIO_DIRECTION;
 
    // Set GPIO pin directions => Outputs: CS, SCK, MOSI, RST (setting to 1)   Inputs:  MISO, DRDY (setting to 0)   
    direction |= (SPI_CS_PIN | SPI_SCK_PIN | SPI_MOSI_PIN | ADC_RST_PIN) ;
    direction &= ~(SPI_MISO_PIN | ADC_DRDY_PIN) ;
    *pGPIO_DIRECTION = direction;

    // Set initial pin for SPI Mode 3: Setting SCK, CS, RST(not resetting) to HIGH (1), MOSI to low (MOSI doesn't matter when idle)
    pio_output_state = *pGPIO_DATA;
    pio_output_state |= (SPI_CS_PIN | ADC_RST_PIN | SPI_SCK_PIN);
    pio_output_state &= ~SPI_MOSI_PIN;
    *pGPIO_DATA = pio_output_state;  
    //display_string("SPI init done\n");
}


/**
 * Transfer one byte over SPI (Mode 3)
 * SPI Mode 3 timing: Clock starts high (CPOL=1) - On falling edge: shift out MOSI data - On rising edge: sample MISO data (CPHA=1)
 */
uint8_t spi_transfer_byte(uint8_t byte_out) {
    uint8_t byte_in = 0;
    
    for (int i = 0; i < 8; i++) {
        // shift input left BEFORE reading new bit (making space for the new bit)
        byte_in <<= 1;
        
        // FALLING EDGE: setting pu MOSI pin
        pio_output_state &= ~SPI_SCK_PIN;  // SCK low
        
        // Set MOSI based on MSB of byte_out
        if (byte_out & 0x80) {
            pio_output_state |= SPI_MOSI_PIN;
        } else {
            pio_output_state &= ~SPI_MOSI_PIN;
        }
        byte_out <<= 1;  // Prepare next bit
        
        *pGPIO_DATA = pio_output_state;
        spi_delay();  // Data setup time
        
        // RISING EDGE: Sample MISO data 
        pio_output_state |= SPI_SCK_PIN;  // SCK high
        *pGPIO_DATA = pio_output_state;
        spi_delay();  // Hold time
        
        // Sample MISO after rising edge
        if (*pGPIO_DATA & SPI_MISO_PIN) {
            byte_in |= 0x01;
        }
    }    
    // Clock ends high (Mode 3 idle state)
    return byte_in;
}


// Select ADC chip, CS LOW (Select and Deselect functions are optional, can be not used, instead we can just wire CS pin of ADC to ground)
void spi_select_chip(void) {
    pio_output_state &= ~SPI_CS_PIN;
    *pGPIO_DATA = pio_output_state;
    spi_delay();
}
// If CS is 0, chip is selected, active low - If CS is HIGH 1, chip is not selected, ADC will ignore the bus 
void spi_deselect_chip(void) {
    pio_output_state |= SPI_CS_PIN;
    *pGPIO_DATA = pio_output_state;
    spi_delay();
}


// Control the ADC hardware reset pin : true = release reset, false = assert reset (active low)
void spi_reset_pin(bool high) {
    if (high) {
        pio_output_state |= ADC_RST_PIN;
    } else {
        pio_output_state &= ~ADC_RST_PIN;
    }
    *pGPIO_DATA = pio_output_state;
}



// Wait for DRDY pin to go low (data ready), true if ready, false if timeout (blocking function)
bool spi_wait_for_ready(void) {
    int timeout = 1000000;

    while (timeout > 0) {
        if ((*pGPIO_DATA & ADC_DRDY_PIN) == 0) {
            return true;  // DRDY is low, data ready
        }
        timeout--;
    }
    display_string("SPI DRDY timeout!\n");
    return false;
}

/*
// Check if DRDY is low (non-blocking)
bool spi_wait_for_ready(void) {
    return (*pGPIO_DATA & ADC_DRDY_PIN) == 0;
}
*/


// Reset the SPI interface by sending 32 ones - page 31 doc 
void spi_interface_reset(void) {
    spi_select_chip();
    for (int i = 0; i < 4; i++) {
        spi_transfer_byte(0xFF);
    } 
    spi_deselect_chip();
    delay_ms(1);
}