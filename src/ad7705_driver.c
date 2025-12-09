#include "ad7705_driver.h"
#include "spi_driver.h"
#include "dtekv-lib.h"
#include "delay.h"
#include "hardware.h"


static void write_byte(uint8_t data);
static void set_next_operation(uint8_t reg, uint8_t channel, bool read);
static void write_clock_register(uint8_t channel, uint8_t clkdis, uint8_t clkdiv, uint8_t clk, uint8_t update_rate);
static void write_setup_register(uint8_t channel, uint8_t mode, uint8_t gain, uint8_t b_u, uint8_t buf, uint8_t fsync);
static bool check_drdy(uint8_t channel);
static int wait_for_ready(int timeout, uint8_t channel);
static uint8_t current_gain_setting = GAIN_1;



void ad7705_init(uint8_t channel) {
    spi_reset_pin(false);   
    delay_ms(10);           
    spi_reset_pin(true);    
    delay_ms(10);             
    spi_interface_reset();  

    write_clock_register(channel, 0, 1, 1, UPDATE_RATE_500);
    write_setup_register(channel, MODE_SELF_CAL, GAIN_1, UNIPOLAR, 0, 0);    
    current_gain_setting = GAIN_1;               
    
    delay_ms(10);  
    print("Waiting for ad7705 for self-calibration...");
    if (wait_for_ready(500000, channel) != ADC_OK) {
        print("ADC init FAILED - timeout!\n");
        set_leds(0x3FF);
        while (1);
    }
}



uint16_t ad7705_read_data(uint8_t channel) {
    if (wait_for_ready(100000, channel) != ADC_OK) {
        print("ADC read timeout\n");
        return 32768;   
    }
    set_next_operation(REG_DATA, channel, true); 

    spi_select_chip();
    uint8_t high_byte = spi_transfer_byte(0x00);
    uint8_t low_byte = spi_transfer_byte(0x00);
    spi_deselect_chip();

    return ((uint16_t)high_byte << 8) | low_byte;  
}



void ad7705_set_gain(uint8_t channel, int gain_value) {
    uint8_t gain_setting;  
    switch (gain_value) {
        case 1:   gain_setting = GAIN_1;   break;
        case 2:   gain_setting = GAIN_2;   break;
        case 4:   gain_setting = GAIN_4;   break;
        case 8:   gain_setting = GAIN_8;   break;
        case 16:  gain_setting = GAIN_16;  break;
        case 32:  gain_setting = GAIN_32;  break;
        case 64:  gain_setting = GAIN_64;  break;
        case 128: gain_setting = GAIN_128; break;
        default:  gain_setting = GAIN_1;   break;
    }
    if (gain_setting == current_gain_setting) {
        return;
    }

    current_gain_setting = gain_setting;
    write_setup_register(channel, MODE_SELF_CAL, gain_setting, UNIPOLAR, 0, 0);
    delay_ms(10);
    if (wait_for_ready(100000, channel) != ADC_OK) {
        print("Gain change failed\n");
        current_gain_setting = GAIN_1;
    }
}


float ad7705_read_voltage(uint8_t channel) {
    uint16_t raw = ad7705_read_data(channel);
    static const float gain_values[] = {1.0f, 2.0f, 4.0f, 8.0f, 16.0f, 32.0f, 64.0f, 128.0f};
    float gain = gain_values[current_gain_setting & 0x07];
    float voltage = ((float)raw / 65535.0f) * VREF / gain;
    voltage *= 1.5350f;  

    return voltage;
}


static void write_byte(uint8_t data) {
    spi_select_chip();
    spi_transfer_byte(data);
    spi_deselect_chip();
}


static void set_next_operation(uint8_t reg, uint8_t channel, bool read) {
    uint8_t comm_byte = 0;
    comm_byte |= (reg & 0x07) << 4;      
    comm_byte |= (read ? 1 : 0) << 3;   
    comm_byte |= (channel & 0x03);           
    write_byte(comm_byte);
}



static void write_clock_register(uint8_t channel, uint8_t clkdis, uint8_t clkdiv, uint8_t clk, uint8_t update_rate) {
    set_next_operation(REG_CLOCK, channel, false); 
    uint8_t clock_byte = 0;
    clock_byte |= (clkdis & 0x01) << 4;
    clock_byte |= (clkdiv & 0x01) << 3;
    clock_byte |= (clk & 0x01) << 2;
    clock_byte |= (update_rate & 0x03);  
    write_byte(clock_byte);
}



static void write_setup_register(uint8_t channel, uint8_t mode, uint8_t gain, uint8_t b_u, uint8_t buf, uint8_t fsync) {
    set_next_operation(REG_SETUP, channel, false); 
    uint8_t setup_byte = 0;
    setup_byte |= (mode & 0x03) << 6;
    setup_byte |= (gain & 0x07) << 3;
    setup_byte |= (b_u & 0x01) << 2;
    setup_byte |= (buf & 0x01) << 1;
    setup_byte |= (fsync & 0x01);    
    write_byte(setup_byte);
}



static bool check_drdy(uint8_t channel) {
    set_next_operation(REG_CMM, channel, true); 

    spi_select_chip();
    uint8_t status = spi_transfer_byte(0x00);
    spi_deselect_chip();   
     
    return (status & 0x80) == 0;
}



static int wait_for_ready(int timeout, uint8_t channel) {
    while (timeout > 0) {
        if (check_drdy(channel)) {
            return ADC_OK;
        }
        timeout--;
    }
    print("ADC TIMEOUT!\n");
    return ADC_TIMEOUT;
}
