#include "ad7705_driver.h"
#include "spi_driver.h"     
#include "hardware.h"      

// Initializes the AD7705 
void ad7705_init() {
    // 1. Hard Reset ADC
    spi_reset_pin(false); // Pull reset pin LOW (active)
    for(volatile int d=0; d < 100; d++); // Short delay
    spi_reset_pin(true);  // Pull reset pin HIGH (inactive)
    
    // Wait for the chip to be ready (min 500ns)
    for(volatile int d=0; d < 1000; d++);

    // 2. Configure the Clock Register
    spi_select_chip();   
    spi_transfer_byte(REG_COMM | WRITE_CLOCK_REG); // Command: "Next write is to Clock Reg"
    
    spi_transfer_byte(CLOCK_CONFIG);      // Using 4.9152MHz clock config (0x0C)         
    spi_deselect_chip();

    // 3. Configure the Setup Register and start self-calibration
    spi_select_chip();    
    spi_transfer_byte(REG_COMM | WRITE_SETUP_REG); // Command: "Next write is to Setup Reg"
    spi_transfer_byte(MODE_SELF_CAL | GAIN_1 | UNIPOLAR_MODE); 
    spi_deselect_chip();

    // 4. Wait for the calibration to finish (~200ms)
    // The ~DRDY pin will go LOW when calibration is done.
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
