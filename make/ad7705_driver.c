#include "ad7705_driver.h"
#include "spi_driver.h"     
#include "hardware.h"      


void ad7705_init() {
    spi_init();
    spi_reset_pin(false); // Pull reset pin LOW (active)
    for(volatile int d=0; d < 100; d++); 
    spi_reset_pin(true);  // Pull reset pin HIGH (inactive)
    
    for(volatile int d=0; d < 1000; d++);


    spi_select_chip();
    spi_transfer_byte(REG_COMM | WRITE_CLOCK_REG); 

    spi_transfer_byte(CLOCK_DIV_1); // 4.9152MHz clock / 1
    spi_deselect_chip();


    spi_select_chip();
    spi_transfer_byte(REG_COMM | WRITE_SETUP_REG); 

    spi_transfer_byte(MODE_SELF_CAL | GAIN_1 | UNIPOLAR_MODE);
    spi_deselect_chip();

    spi_wait_for_ready();
}

uint16_t ad7705_read_data() {
    uint16_t data;


    spi_wait_for_ready();
    

    spi_select_chip();
    spi_transfer_byte(REG_COMM | READ_DATA_REG);
    
    // Read the two bytes (High Byte, then Low Byte)
    uint8_t high_byte = spi_transfer_byte(0x00); // Send dummy byte to read
    uint8_t low_byte = spi_transfer_byte(0x00);  // Send dummy byte to read
    
    spi_deselect_chip();
    
    data = ((uint16_t)high_byte << 8) | low_byte;
    
    return data;
}