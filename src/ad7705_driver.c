#include "ad7705_driver.h"
#include "spi_driver.h"     
#include "hardware.h"      

void ad7705_init() {
    // Hard Reset ADC
    spi_reset_pin(false); 
    for(volatile int d=0; d < 1000; d++); 
    spi_reset_pin(true);  
    for(volatile int d=0; d < 1000; d++);


    spi_select_chip();
    for(int i=0; i<5; i++) {
        spi_transfer_byte(0xFF);
    }
    spi_deselect_chip();
    

    // Configure Clock
    spi_select_chip();   
    spi_transfer_byte(REG_COMM | WRITE_CLOCK_REG); 
    spi_transfer_byte(CLOCK_CONFIG);             
    spi_deselect_chip();

    // Configure Setup
    spi_select_chip();    
    spi_transfer_byte(REG_COMM | WRITE_SETUP_REG); 
    spi_transfer_byte(MODE_SELF_CAL | GAIN_1 | UNIPOLAR_MODE); 
    spi_deselect_chip();

    // Wait for calibration
    spi_wait_for_ready();
}

// Reads the 16-bit conversion result from the ADC.
uint16_t ad7705_read_data() {
    uint16_t data;

    // 1. Wait for ~DRDY to go LOW
    spi_wait_for_ready();
    
    // 2. Tell the ADC we want to read the data register
    spi_select_chip();
    spi_transfer_byte(REG_COMM | READ_DATA_REG);
    
    // 3. Read the two bytes (High Byte, then Low Byte)
    uint8_t high_byte = spi_transfer_byte(0x00); // Send dummy byte to read High
    uint8_t low_byte = spi_transfer_byte(0x00);  // Send dummy byte to read Low
    
    spi_deselect_chip();
    
    // 4. Combine the two 8-bit bytes into one 16-bit value
    data = ((uint16_t)high_byte << 8) | low_byte;
    
    return data;
}
