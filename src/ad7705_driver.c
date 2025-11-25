#include "ad7705_driver.h"
#include "spi_driver.h"
#include "hardware.h"
#include "lib.h"
#include "dtekv-lib.h"


void set_next_operation(uint8_t reg, uint8_t channel, uint8_t readWrite);
void write_clock_register(uint8_t CLKDIS, uint8_t CLKDIV, uint8_t outputUpdateRate);
void write_setup_register(uint8_t operationMode, uint8_t gain, uint8_t unipolar, uint8_t buffered, uint8_t fsync);
bool data_ready(uint8_t channel);

static inline void write_to_reg(uint8_t d) {
    spi_select_chip();
    spi_transfer_byte(d);
    spi_deselect_chip();
}

void set_next_operation(uint8_t reg, uint8_t channel, uint8_t readWrite) {
    uint8_t r = 0;
    r = reg << 4 | readWrite << 3 | channel;
    write_to_reg(r);
}

void write_clock_register(uint8_t CLKDIS, uint8_t CLKDIV, uint8_t outputUpdateRate) {
    uint8_t r = CLKDIS << 4 | CLKDIV << 3 | outputUpdateRate;
    r &= ~(1 << 2); // clear CLK
    write_to_reg(r);
}

void write_setup_register(uint8_t operationMode, uint8_t gain, uint8_t unipolar, uint8_t buffered, uint8_t fsync) {
    uint8_t r = operationMode << 6 | gain << 3 | unipolar << 2 | buffered << 1 | fsync;
    write_to_reg(r);
}

bool data_ready(uint8_t channel) {
    set_next_operation(REG_CMM, channel, 1);
    spi_select_chip();
    uint8_t high_byte = spi_transfer_byte(0x00); // Clock out MSB
    spi_deselect_chip();
    return (high_byte & 0x80) == 0x0;
}

void ad7705_init(uint8_t channel) {
    spi_reset_pin(false); 
    for(volatile int d=0; d < 10000; d++); 
    spi_reset_pin(true);  
    for(volatile int d=0; d < 10000; d++);
    display_string("ADC initializing ...");
    write_to_reg(WRITE_CLOCK_REG);
    write_to_reg(CLOCK_CONFIG);
    write_to_reg(WRITE_SETUP_REG);
    write_to_reg(MODE_SELF_CAL);   
    display_string("ADC init wait ...");
    while (!data_ready(channel)) {
    };
    display_string("ADC init ready ...");
}


uint16_t ad7705_read_data(uint8_t channel) {

    while(!data_ready(channel)) {}; 
    set_next_operation(REG_DATA, channel, 1);
    spi_select_chip();
    uint8_t high_byte = spi_transfer_byte(0x00); // Clock out MSB
    uint8_t low_byte  = spi_transfer_byte(0x00); // Clock out LSB
    spi_deselect_chip();
    uint16_t data = (high_byte << 8) | low_byte;
    return data;
}


float ad7705_read_voltage(uint8_t channel) {
    return ad7705_read_data(channel) * 1.0 / UINT16_MAX * VREF;
}
 