/**
 * spi_driver.c - Bit-banged SPI driver for AD7705 ADC
 * 
 * SPI Mode 3: CPOL=1 (clock idle high), CPHA=1 (sample on rising edge)
 * 
 * AD7705 SPI Timing:
 * - Data is shifted out on falling edge of SCLK
 * - Data is sampled on rising edge of SCLK
 * - MSB first
 */

#include "spi_driver.h"
#include "hardware.h" 
#include "dtekv-lib.h"
#include "lib.h"
#include "delay.h"

// Track output state to avoid read-modify-write races
static uint32_t pio_output_state;

// Minimum SPI clock half-period delay
// AD7705 max SCLK is 2.1 MHz, so half-period >= 238ns
// Using 500ns for safety margin
static void spi_delay(void) {
    delay_ns(500);
}

/**
 * Initialize SPI GPIO pins
 */
void spi_init(void) {
    display_string("SPI init...\n");
    
    // Read current direction register
    uint32_t direction = *pGPIO_DIRECTION;

    // Set pin directions:
    // Outputs: CS, SCK, MOSI, RST
    // Inputs:  MISO, DRDY
    direction |= (SPI_CS_PIN | SPI_SCK_PIN | SPI_MOSI_PIN | ADC_RST_PIN);
    direction &= ~(SPI_MISO_PIN | ADC_DRDY_PIN);
    *pGPIO_DIRECTION = direction;

    // Set initial pin states for SPI Mode 3:
    // - SCK: High (idle state for CPOL=1)
    // - CS: High (inactive/deselected)
    // - RST: High (not resetting, RST is active low)
    // - MOSI: Low (doesn't matter when idle)
    pio_output_state = *pGPIO_DATA;
    pio_output_state |= (SPI_CS_PIN | ADC_RST_PIN | SPI_SCK_PIN);
    pio_output_state &= ~SPI_MOSI_PIN;
    *pGPIO_DATA = pio_output_state;
    
    display_string("SPI init done\n");
}

/**
 * Select the ADC chip (assert CS low)
 */
void spi_select_chip(void) {
    pio_output_state &= ~SPI_CS_PIN;
    *pGPIO_DATA = pio_output_state;
    spi_delay();
}

/**
 * Deselect the ADC chip (release CS high)
 */
void spi_deselect_chip(void) {
    pio_output_state |= SPI_CS_PIN;
    *pGPIO_DATA = pio_output_state;
    spi_delay();
}

/**
 * Control the ADC hardware reset pin
 */
void spi_reset_pin(bool high) {
    if (high) {
        pio_output_state |= ADC_RST_PIN;
    } else {
        pio_output_state &= ~ADC_RST_PIN;
    }
    *pGPIO_DATA = pio_output_state;
}

/**
 * Wait for DRDY pin to go low (data ready)
 */
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


/**
 * Check if DRDY is asserted (low)
 */
bool spi_is_ready(void) {
    return (*pGPIO_DATA & ADC_DRDY_PIN) == 0;
}


/**
 * Transfer one byte over SPI (Mode 3)
 * 
 * SPI Mode 3 timing:
 * 1. Clock starts high (CPOL=1)
 * 2. On falling edge: shift out MOSI data
 * 3. On rising edge: sample MISO data (CPHA=1)
 */
uint8_t spi_transfer_byte(uint8_t byte_out) {
    uint8_t byte_in = 0;
    
    for (int i = 0; i < 8; i++) {
        // Prepare to receive - shift input left BEFORE reading new bit
        byte_in <<= 1;
        
        // === FALLING EDGE: Setup MOSI data ===
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
        
        // === RISING EDGE: Sample MISO data ===
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

/**
 * Reset the SPI interface by sending 32 ones
 * This is recommended by AD7705 datasheet to reset serial interface
 */
void spi_interface_reset(void) {
    display_string("SPI interface reset...\n");
    spi_select_chip();
    
    // Send at least 32 bits of 1s to reset the AD7705 serial interface
    for (int i = 0; i < 4; i++) {
        spi_transfer_byte(0xFF);
    }
    
    spi_deselect_chip();
    delay_ms(1);
    display_string("SPI interface reset done\n");
}