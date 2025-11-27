/**
 * AD7705 is a 2-channel, 16-bit ADC , 2 input channels, on chip settings for gain, update_rate
 * SPI (Mode 3), Self-calibration and system calibration modes
 */

#ifndef AD7705_DRIVER_H
#define AD7705_DRIVER_H
#include <stdint.h>
#include <stdbool.h>


// Reference Voltage in v
#define VREF        3.3f   


// AD7705 Register Addresses (for Communication Register RS2-RS0 bits) - page 17 doc 
#define REG_CMM     0x0    // Communication Register (8-bit)
#define REG_SETUP   0x1    // Setup Register (8-bit)
#define REG_CLOCK   0x2    // Clock Register (8-bit)
#define REG_DATA    0x3    // Data Register (16-bit, read-only)
#define REG_TEST    0x4    // Test Register (8-bit) 
#define REG_NOP     0x5    // No operation
#define REG_OFFSET  0x6    // Offset Register (24-bit)
#define REG_GAIN    0x7    // Gain Register (24-bit)


// Channel Selection
#define CHN_AIN1    0x0    // AIN1(+)/AIN1(-)
#define CHN_AIN2    0x1    // AIN2(+)/AIN2(-)


// Output Update Rates (for Clock Register FS1-FS0 bits) - depend on MCLK frequency and CLKDIV setting - page 19 doc 
// Values below are for MCLK = 4.9152 MHz, CLKDIV = 0
#define UPDATE_RATE_20    0x0    
#define UPDATE_RATE_25    0x1    
#define UPDATE_RATE_100   0x2    
#define UPDATE_RATE_200   0x3    
// For MCLK = 2.4576 MHz or MCLK = 4.9152 MHz with CLKDIV = 1
#define UPDATE_RATE_50    0x0   
#define UPDATE_RATE_60    0x1    
#define UPDATE_RATE_250   0x2    
#define UPDATE_RATE_500   0x3    


// Operating Modes (for Setup Register MD1-MD0 bits) - page 18 doc
#define MODE_NORMAL          0x0    // Normal operation 
#define MODE_SELF_CAL        0x1    // Self-calibration
#define MODE_ZERO_SCALE_CAL  0x2    // Zero-scale system calibration
#define MODE_FULL_SCALE_CAL  0x3    // Full-scale system calibration


// Gain Settings (for Setup Register G2-G0 bits) - page 18 doc
#define GAIN_1      0x0     
#define GAIN_2      0x1    
#define GAIN_4      0x2   
#define GAIN_8      0x3    
#define GAIN_16     0x4    
#define GAIN_32     0x5    
#define GAIN_64     0x6    
#define GAIN_128    0x7   


// Polarity (for Setup Register B/U bit)
#define BIPOLAR     0x0    // -Vref to +Vref
#define UNIPOLAR    0x1    // 0 to +Vref


#define WRITE_SETUP_REG   0x10    // Write to setup register, channel 0
#define WRITE_CLOCK_REG   0x20    // Write to clock register, channel 0
#define CLOCK_CONFIG      0x0C    // CLK=1, CLKDIV=0, update rate=0


void ad7705_init(uint8_t channel);
uint16_t ad7705_read_data(uint8_t channel);
float ad7705_read_voltage(uint8_t channel);
//bool ad7705_data_ready(uint8_t channel);
//bool ad7705_read_data_timeout(uint8_t channel, uint16_t *data);

#endif // AD7705_DRIVER_H