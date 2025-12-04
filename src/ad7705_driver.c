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
int timeout(int timeout, uint8_t channel);
static uint8_t current_gain_setting = GAIN_1;


// Initialize the AD7705 ADC
void ad7705_init(uint8_t channel) {
    //display_string("AD7705 init start");
    // Hardware reset
    spi_reset_pin(false);   // Assert reset (active low)
    delay_ms(10);           
    spi_reset_pin(true);    // Release reset
    delay_ms(10);           // Wait for ADC to stabilize
    
    spi_interface_reset();  // resetting spi (safety)
    
    // Configure Clock Register: CLKDIS=0 (clock enabled), CLKDIV (division), CLK=1 (MCLK > 2MHz), update rate
    write_clock_register(channel, 0, 1, 1, UPDATE_RATE_500);
                                  
    // Configure Setup Register: Mode = Self-Cal, Gain, Unipolar, Unbuffered
    write_setup_register(channel, MODE_SELF_CAL, GAIN_1, UNIPOLAR, 0, 0);    
    current_gain_setting = GAIN_1;               
    
    // Watiting for adc to start calibration and stabilize 
    delay_ms(10);  
    
    // Wait for self-calibration to complete: DRDY goes low when calibration is done - page 18 doc
    display_string("Waiting for ad7705 for self-calibration...");
    if (timeout(500000, channel) != ADC_OK) {
        display_string("ADC init FAILED - timeout!\n");
        set_leds(0x3FF);  
        while(1);  // it will not continoue without adc initializaiton 
    }
    //display_string("ADC init complete.");
}



// Read and return raw 16-bit ADC data from specified channel, Blocks until data is ready
uint16_t ad7705_read_data(uint8_t channel) {
    // Wait for data ready
    if (timeout(100000, channel) != ADC_OK) {
        display_string("ADC read timeout\n");
        return 32768;   /// mid-scale value on timeout 
    }

    set_next_operation(REG_DATA, channel, true);  // read Data Register
    
    // Read 16-bit data (MSB first)
    spi_select_chip();
    uint8_t high_byte = spi_transfer_byte(0x00);
    uint8_t low_byte = spi_transfer_byte(0x00);
    spi_deselect_chip();
    
    return ((uint16_t)high_byte << 8) | low_byte;  
}


/**
 * Gain is like a zoom for small signals. It amplifies the input voltage internally
 * This functions is being called to change the gain by user without init 
 * Higher gain = more precision, smaller max input voltage3
 */
void ad7705_set_gain(uint8_t channel, int gain_value) {
    uint8_t gain_setting;
    
    // Convert gain values to register setting
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
    // Write new setup with self-calibration, recalibrates with the new gain
    write_setup_register(channel, MODE_SELF_CAL, gain_setting, UNIPOLAR, 0, 0);
    delay_ms(10);
    if (timeout(100000, channel) != ADC_OK) {
        display_string("Gain change failed\n");
        current_gain_setting = GAIN_1;
    }
}


/**
 * Convert raw ADC value to voltage
 * For unipolar mode: float Voltage = (ADC_Value / 65535.0f) * VREF / gain;
 * For bipolar mode:  float Voltage = ((ADC_Value - 32768.0f) / 32768.0f) * VREF / gain;
 * Dividing by gain is for amplifiing, higher gains make the output voltage less noisy and precise and smooht
 * but the disadvantage is lower maximum measurable voltage 
 */
float ad7705_read_voltage(uint8_t channel) {
    uint16_t raw = ad7705_read_data(channel);
    
    // Gain multiplier lookup table
    static const float gain_values[] = {1.0f, 2.0f, 4.0f, 8.0f, 16.0f, 32.0f, 64.0f, 128.0f};
    float gain = gain_values[current_gain_setting & 0x07];
    
    // Base voltage calculation (with gain compensation)
    float voltage = ((float)raw / 65535.0f) * VREF / gain;
    
    /** since there are internal and hardware reverence behaviour, voltgae divider and other components were reducing the signal
     * for capturing the real signal I applied this voltage calibration factor which gained by experiments and it's just exception and 
     * specific for this hardware 
     *  */ 
    voltage *= 1.5350f;
    
    return voltage;
}


// Write a single byte to the AD7705
static void write_byte(uint8_t data) {
    spi_select_chip();
    spi_transfer_byte(data);
    spi_deselect_chip();
}


/**
 * Write to the Communication Register(8-bit) to set up next operation
 * Bit 7: 0 (must be zero)
 * Bit 6-4: RS2-RS0 (Register Select)
 * Bit 3: R/W (0=write, 1=read)
 * Bit 2: STBY (standby mode)
 * Bit 1-0: CH1-CH0 (Channel Select)
 */
static void set_next_operation(uint8_t reg, uint8_t channel, bool read) {
    uint8_t comm_byte = 0;
    comm_byte |= (reg & 0x07) << 4;      
    comm_byte |= (read ? 1 : 0) << 3;   
    comm_byte |= (channel & 0x03);       
    
    write_byte(comm_byte);
}


/**
 * Write to the Clock Register (8-bits)
 * Bit 7-5: ZERO (must be 0)
 * Bit 4: CLKDIS (0=master clock enabled)
 * Bit 3: CLKDIV (0=no divide, 1=divide by 2)
 * Bit 2: CLK (0=MCLK IN is 2.4576MHz, 1=MCLK IN is 4.9152MHz)
 * Bit 1-0: FS1-FS0 (Output update rate)
 */
static void write_clock_register(uint8_t channel, uint8_t clkdis, uint8_t clkdiv, uint8_t clk, uint8_t update_rate) {
    set_next_operation(REG_CLOCK, channel, false);
    
    uint8_t clock_byte = 0;
    clock_byte |= (clkdis & 0x01) << 4;
    clock_byte |= (clkdiv & 0x01) << 3;
    clock_byte |= (clk & 0x01) << 2;
    clock_byte |= (update_rate & 0x03);
    
    write_byte(clock_byte);
}


/**
 * Write to the Setup Register (8-bits)
 * Bit 7-6: MD1-MD0 (Operating Mode)
 * Bit 5-3: G2-G0 (Gain selection)
 * Bit 2: B/U (0=Bipolar, 1=Unipolar)
 * Bit 1: BUF (0=Unbuffered, 1=Buffered)
 * Bit 0: FSYNC (Filter sync, 0=normal)
 */
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



// Check if data is ready by reading the Communication Register: DRDY bit (bit 7) = 0 means data ready (active low)
static bool check_drdy(uint8_t channel) {
    set_next_operation(REG_CMM, channel, true);
    
    spi_select_chip();
    uint8_t status = spi_transfer_byte(0x00);
    spi_deselect_chip();
    
    return (status & 0x80) == 0;
}



int timeout(int timeout, uint8_t channel) {
    while (timeout > 0) {
        if (check_drdy(channel)) {
            return ADC_OK;
        }
        timeout--;
    }
    display_string("ADC TIMEOUT!\n");
    return ADC_TIMEOUT;
}

