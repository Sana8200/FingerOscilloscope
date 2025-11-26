/**
 * ad7705_driver.c 
 * The AD7705 is a 16-bit ADC with 2 differential input channels.
 * It uses SPI Mode 3 (CPOL=1, CPHA=1) for communication.
 * 
 * Communication sequence:
 * 1. Write to Communication Register to select next operation
 * 2. Read/Write the selected register
 * 
 * DRDY pin goes low when conversion data is ready.
 */

#include "ad7705_driver.h"
#include "spi_driver.h"
#include "hardware.h"
#include "lib.h"
#include "dtekv-lib.h"
#include "delay.h"


/**
 * Write a single byte to the AD7705
 */
static void write_byte(uint8_t data) {
    spi_select_chip();
    spi_transfer_byte(data);
    spi_deselect_chip();
}

/**
 * Write to the Communication Register to set up next operation
 * 
 * Communication Register Format (8 bits):
 * Bit 7: 0 (must be zero)
 * Bit 6-4: RS2-RS0 (Register Select)
 * Bit 3: R/W (0=write, 1=read)
 * Bit 2: STBY (standby mode)
 * Bit 1-0: CH1-CH0 (Channel Select)
 */
static void set_next_operation(uint8_t reg, uint8_t channel, bool read) {
    uint8_t comm_byte = 0;
    comm_byte |= (reg & 0x07) << 4;      // RS2-RS0 in bits 6-4
    comm_byte |= (read ? 1 : 0) << 3;    // R/W in bit 3
    comm_byte |= (channel & 0x03);       // CH1-CH0 in bits 1-0
    
    write_byte(comm_byte);
}

/**
 * Write to the Clock Register
 * 
 * Clock Register Format (8 bits):
 * Bit 7: ZERO (must be 0)
 * Bit 6: ZERO (must be 0)
 * Bit 5: ZERO (must be 0)
 * Bit 4: CLKDIS (0=master clock enabled)
 * Bit 3: CLKDIV (0=no divide, 1=divide by 2)
 * Bit 2: CLK (0=MCLK IN is 2.4576MHz, 1=MCLK IN is 4.9152MHz)
 * Bit 1-0: FS1-FS0 (Output update rate)
 * 
 * For 1MHz crystal with CLKDIV=0, CLK=0:
 * FS1 FS0 | Update Rate
 * 0   0   | 20 Hz
 * 0   1   | 25 Hz
 * 1   0   | 100 Hz
 * 1   1   | 200 Hz
 */
static void write_clock_register(uint8_t channel, uint8_t clkdis, uint8_t clkdiv, 
                                  uint8_t clk, uint8_t update_rate) {
    set_next_operation(REG_CLOCK, channel, false);
    
    uint8_t clock_byte = 0;
    clock_byte |= (clkdis & 0x01) << 4;
    clock_byte |= (clkdiv & 0x01) << 3;
    clock_byte |= (clk & 0x01) << 2;
    clock_byte |= (update_rate & 0x03);
    
    write_byte(clock_byte);
}

/**
 * Write to the Setup Register
 * 
 * Setup Register Format (8 bits):
 * Bit 7-6: MD1-MD0 (Operating Mode)
 *          00 = Normal mode
 *          01 = Self-calibration
 *          10 = Zero-scale system calibration
 *          11 = Full-scale system calibration
 * Bit 5-3: G2-G0 (Gain selection)
 * Bit 2: B/U (0=Bipolar, 1=Unipolar)
 * Bit 1: BUF (0=Unbuffered, 1=Buffered)
 * Bit 0: FSYNC (Filter sync, 0=normal)
 */
static void write_setup_register(uint8_t channel, uint8_t mode, uint8_t gain,
                                  uint8_t bipolar_unipolar, uint8_t buffered, uint8_t fsync) {
    set_next_operation(REG_SETUP, channel, false);
    
    uint8_t setup_byte = 0;
    setup_byte |= (mode & 0x03) << 6;
    setup_byte |= (gain & 0x07) << 3;
    setup_byte |= (bipolar_unipolar & 0x01) << 2;
    setup_byte |= (buffered & 0x01) << 1;
    setup_byte |= (fsync & 0x01);
    
    write_byte(setup_byte);
}

/**
 * Check if data is ready by reading the Communication Register
 * DRDY bit (bit 7) = 0 means data ready
 */
static bool check_drdy_register(uint8_t channel) {
    set_next_operation(REG_CMM, channel, true);  // Read communication register
    
    spi_select_chip();
    uint8_t status = spi_transfer_byte(0x00);
    spi_deselect_chip();
    
    return (status & 0x80) == 0;  // DRDY is bit 7, active low
}



/**
 * Initialize the AD7705 ADC
 * 
 * This performs:
 * 1. Hardware reset via RST pin
 * 2. SPI interface reset (32 clock cycles with DIN high)
 * 3. Configure clock register
 * 4. Configure setup register and start self-calibration
 * 5. Wait for calibration to complete
 */
void ad7705_init(uint8_t channel) {
    display_string("AD7705 init start\n");
    
    // Step 1: Hardware reset
    display_string("  Hardware reset...\n");
    spi_reset_pin(false);   // Assert reset (active low)
    delay_ms(10);           // Hold reset
    spi_reset_pin(true);    // Release reset
    delay_ms(10);           // Wait for ADC to stabilize
    
    // Step 2: Reset SPI interface (send 32 ones)
    display_string("  SPI interface reset...\n");
    spi_interface_reset();
    
    // Step 3: Configure Clock Register
    // Using: CLKDIS=0 (clock enabled), CLKDIV=0, CLK=1 (for higher freq crystal)
    // Update rate = 0x03 (200 Hz for faster scope updates)
    display_string("  Config clock reg...\n");
    write_clock_register(channel, 
                         0,                    // CLKDIS: 0 = clock enabled
                         0,                    // CLKDIV: 0 = no division  
                         1,                    // CLK: 1 = MCLK > 2MHz
                         UPDATE_RATE_200);     // 200 Hz update rate
    
    // Step 4: Configure Setup Register with Self-Calibration
    // Mode = Self-Cal, Gain = 1, Unipolar, Unbuffered
    display_string("  Config setup reg + self-cal...\n");
    write_setup_register(channel,
                         MODE_SELF_CAL,        // Self-calibration mode
                         GAIN_1,               // Gain = 1
                         UNIPOLAR,             // Unipolar mode (0 to Vref)
                         0,                    // Unbuffered
                         0);                   // FSYNC = 0
    
    // Step 5: Wait for self-calibration to complete
    // DRDY goes low when calibration is done
    display_string("  Waiting for calibration...\n");
    
    int timeout = 500000;
    while (timeout > 0) {
        if (check_drdy_register(channel)) {
            break;
        }
        timeout--;
    }
    
    if (timeout == 0) {
        display_string(" ERROR: Cal timeout!\n");
    } else {
        display_string(" Calibration done!\n");
    }
    
    display_string("AD7705 init complete\n");
}

/**
 * Read raw 16-bit ADC data from specified channel, Blocks until data is ready
 */
uint16_t ad7705_read_data(uint8_t channel) {
    // Wait for data ready
    while (!check_drdy_register(channel)) {
        // Busy wait
    }
    
    // Set up to read Data Register
    set_next_operation(REG_DATA, channel, true);
    
    // Read 16-bit data (MSB first)
    spi_select_chip();
    uint8_t high_byte = spi_transfer_byte(0x00);
    uint8_t low_byte = spi_transfer_byte(0x00);
    spi_deselect_chip();
    
    return ((uint16_t)high_byte << 8) | low_byte;
}

/**
 * Read ADC data with timeout (non-blocking option)
 */
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

/**
 * Convert raw ADC value to voltage
 * 
 * For unipolar mode: Voltage = (ADC_Value / 65535) * Vref
 * For bipolar mode:  Voltage = ((ADC_Value - 32768) / 32768) * Vref
 */
float ad7705_read_voltage(uint8_t channel) {
    uint16_t raw = ad7705_read_data(channel);
    
    // Unipolar mode calculation
    return (float)raw * VREF / 65535.0f;
}



/**
 * Check if data is ready without blocking
 */
bool ad7705_data_ready(uint8_t channel) {
    return check_drdy_register(channel);
}