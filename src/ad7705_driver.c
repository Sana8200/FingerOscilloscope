#include "ad7705_driver.h"
#include "spi_driver.h"
#include "lib.h"
#include "delay.h"

static void write_byte(uint8_t data);
static void set_next_operation(uint8_t reg, uint8_t channel, bool read);
static void write_clock_register(uint8_t channel, uint8_t clkdis, uint8_t clkdiv, uint8_t clk, uint8_t update_rate);
static void write_setup_register(uint8_t channel, uint8_t mode, uint8_t gain, uint8_t b_u, uint8_t buf, uint8_t fsync);
static bool check_drdy_register(uint8_t channel);
void self_cal_timout(int timeout, uint8_t channel);


/**
 * Initialize the AD7705 ADC
 */
void ad7705_init(uint8_t channel) {
    display_string("AD7705 init start\n");
    
    // Hardware reset
    spi_reset_pin(false);   // Assert reset (active low)
    delay_ms(10);           
    spi_reset_pin(true);    // Release reset
    delay_ms(10);           // Wait for ADC to stabilize
    
    spi_interface_reset();  // resetting spi (safety)
    
    // Configure Clock Register: CLKDIS=0 (clock enabled), CLKDIV (division), CLK=1 (MCLK > 2MHz), update rate
    write_clock_register(channel, 0, 1, 1, UPDATE_RATE_500);
                                  
    // Configure Setup Register: Mode = Self-Cal, Gain = 1, Unipolar, Unbuffered
    write_setup_register(channel, MODE_SELF_CAL, GAIN_1, UNIPOLAR, 0, 0);                   
    
    // Watiting for adc to start calibration and stabilize 
    delay_ms(10);  
    
    // Wait for self-calibration to complete: DRDY goes low when calibration is done - page 18 doc
    display_string("Waiting for calibration...\n");
    self_cal_timout(500000, channel);
    display_string("AD7705 init complete\n");
}


/**
 * Read and return raw 16-bit ADC data from specified channel, Blocks until data is ready
 */
uint16_t ad7705_read_data(uint8_t channel) {
    // Wait for data ready
    while (!check_drdy_register(channel)) {       
        // busy wait 
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
 * Convert raw ADC value to voltage
 * For unipolar mode: float Voltage = (ADC_Value / 65535) * VREF;
 * For bipolar mode:  float Voltage = ((ADC_Value - 32768) / 32768) * VREF
 */
float ad7705_read_voltage(uint8_t channel) {
    uint16_t raw = ad7705_read_data(channel);
    float Voltage = (raw / 65535) * VREF;
    return Voltage;
}



/**
 * Write a single byte to the AD7705
 */
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


/**
 * Check if data is ready by reading the Communication Register
 * DRDY bit (bit 7) = 0 means data ready (active low)
 */
static bool check_drdy_register(uint8_t channel) {
    set_next_operation(REG_CMM, channel, true);  // Read communication register
    
    spi_select_chip();
    uint8_t status = spi_transfer_byte(0x00); // sending a dummy byte to keep the clock running to get the chip current status 
    spi_deselect_chip();
    
    return (status & 0x80) == 0;  
}



void self_cal_timout(int timeout, uint8_t channel){
    while (timeout > 0) {
        if (check_drdy_register(channel)) {
            break;
        }
        timeout--;
    } 
    if (timeout == 0) {
        display_string("!!! ERROR: Cal timeout!\n");
    } else {
        display_string("---> Calibration done!\n");
    }
}




/**
 * Read ADC data with timeout (non-blocking)
 */
/*
bool ad7705_read_data_timeout(uint8_t channel, uint16_t *data) {
    int timeout = 100000;    
    while (timeout > 0) {
        if (check_drdy_register(channel)) {
            *data = ad7705_read_data(channel);
            return true;
        }
        timeout--;
    }
    return false;
}
*/


/**
 * Check if data is ready without blocking
 */
/*
bool ad7705_data_ready(uint8_t channel) {
    return check_drdy_register(channel);
}
*/
