#include "ad7705_driver.h"
#include "spi_driver.h"
#include "hardware.h"
#include "lib.h"


void set_next_operation(uint8_t reg, uint8_t channel, uint8_t readWrite);
void write_clock_register(uint8_t CLKDIS, uint8_t CLKDIV, uint8_t outputUpdateRate);
void write_setup_register(uint8_t operationMode, uint8_t gain, uint8_t unipolar, uint8_t buffered, uint8_t fsync);
bool data_ready(uint8_t channel);


void set_next_operation(uint8_t reg, uint8_t channel, uint8_t readWrite) {
    uint8_t r = 0;
    r = reg << 4 | readWrite << 3 | channel;

    spi_select_chip();
    spi_transfer_byte(r);
    spi_deselect_chip();
}

void write_clock_register(uint8_t CLKDIS, uint8_t CLKDIV, uint8_t outputUpdateRate) {
    uint8_t r = CLKDIS << 4 | CLKDIV << 3 | outputUpdateRate;

    r &= ~(1 << 2); // clear CLK

    spi_select_chip();
    spi_transfer_byte(r);
    spi_deselect_chip();
}

void write_setup_register(uint8_t operationMode, uint8_t gain, uint8_t unipolar, uint8_t buffered, uint8_t fsync) {
    uint8_t r = operationMode << 6 | gain << 3 | unipolar << 2 | buffered << 1 | fsync;

    spi_select_chip();
    spi_transfer_byte(r);
    spi_deselect_chip();
}

bool data_ready(uint8_t channel) {
    set_next_operation(REG_CMM, channel, 1);

    spi_select_chip();
    uint8_t high_byte = spi_transfer_byte(0x00); // Clock out MSB
    spi_deselect_chip();
    print_dec(high_byte);
    return (high_byte & 0x80) == 0x0;
}

void ad7705_init(uint8_t channel) {
//    // Hardware Reset 
    spi_reset_pin(false); 
    for(volatile int d=0; d < 10000; d++); // Longer delay for reset
    spi_reset_pin(true);  
    for(volatile int d=0; d < 10000; d++);

    // Interface Synchronization  
//    spi_select_chip();
//    for(int i=0; i<4; i++) { 
//        spi_transfer_byte(0xFF);
//    }
//    spi_deselect_chip();
    

    display_string("ADC initializing ...");

    set_next_operation(REG_CLOCK, channel, 0);
    write_clock_register(0,CLK_DIV_1,UPDATE_RATE_25);
    // Configure Clock Register
    //spi_select_chip();   
//    spi_transfer_byte(REG_COMM | WRITE_CLOCK_REG); 
//    spi_transfer_byte(CLOCK_CONFIG);             
//    spi_deselect_chip();

    // Configure Setup Register
    // Self-Calibration, Gain=1, Unipolar
    //spi_select_chip();    
    //spi_transfer_byte(REG_COMM | WRITE_SETUP_REG); 
    //spi_transfer_byte(MODE_SELF_CAL | GAIN_1 | UNIPOLAR_MODE); 
    //spi_deselect_chip();
    set_next_operation(REG_SETUP, channel, 0);
    write_setup_register(MODE_SELF_CAL, GAIN_1, BIPOLAR, 0, 0);

    // Wait for calibration to complete (DRDY goes low when calibration is finished.)

    display_string("ADC init wait ...");
    while (!data_ready(channel)) {
    };

    display_string("ADC init ready ...");
}


uint16_t ad7705_read_data(uint8_t channel) {

    // Wait for data ready
    // spi_wait_for_ready();

    display_string("ADC wait for read ...");
    while(!data_ready(channel)) {}; 
    display_string("ADC data ready !"); 

    set_next_operation(REG_DATA, channel, 1);

    // Read Data
    spi_select_chip();
    //spi_transfer_byte(REG_COMM | READ_DATA_REG); // Request Data Read
    
    uint8_t high_byte = spi_transfer_byte(0x00); // Clock out MSB
    uint8_t low_byte  = spi_transfer_byte(0x00); // Clock out LSB
    
    spi_deselect_chip();
    
    uint16_t data = (high_byte << 8) | low_byte;
    return data;
}


float ad7705_read_voltage(uint8_t channel) {
    return ad7705_read_data(channel) * 1.0 / UINT16_MAX * VREF;
}
 