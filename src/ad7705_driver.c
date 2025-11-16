#include "ad7705_driver.h"
#include "spi_driver.h"
#include "hardware.h"

void ad7705_init() {
    // Hardware Reset 
    spi_reset_pin(false); 
    for(volatile int d=0; d < 10000; d++); // Longer delay for reset
    spi_reset_pin(true);  
    for(volatile int d=0; d < 10000; d++);

    // Interface Synchronization  
    spi_select_chip();
    for(int i=0; i<4; i++) { 
        spi_transfer_byte(0xFF);
    }
    spi_deselect_chip();
    
    // Configure Clock Register
    spi_select_chip();   
    spi_transfer_byte(REG_COMM | WRITE_CLOCK_REG); 
    spi_transfer_byte(CLOCK_CONFIG);             
    spi_deselect_chip();

    // Configure Setup Register
    // Self-Calibration, Gain=1, Unipolar
    spi_select_chip();    
    spi_transfer_byte(REG_COMM | WRITE_SETUP_REG); 
    spi_transfer_byte(MODE_SELF_CAL | GAIN_1 | UNIPOLAR_MODE); 
    spi_deselect_chip();

    // Wait for calibration to complete (DRDY goes low when calibration is finished.)
    spi_wait_for_ready();
}



uint16_t ad7705_read_data() {
    uint16_t data = 0;

    // Wait for data ready
    spi_wait_for_ready();
    
    // Read Data
    spi_select_chip();
    spi_transfer_byte(REG_COMM | READ_DATA_REG); // Request Data Read
    
    uint8_t high_byte = spi_transfer_byte(0x00); // Clock out MSB
    uint8_t low_byte  = spi_transfer_byte(0x00); // Clock out LSB
    
    spi_deselect_chip();
    
    data = (high_byte << 8) | low_byte;
    return data;
}





 
