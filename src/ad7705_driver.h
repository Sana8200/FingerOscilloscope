#ifndef AD7705_DRIVER_H
#define AD7705_DRIVER_H

#include <stdint.h>


// AD7705 Register Commands page 16 datasheet 
#define REG_COMM        0x00 // Communications Register
#define REG_SETUP       0x10 // Setup Register
#define REG_CLOCK       0x20 // Clock Register
#define REG_DATA        0x30 // Data Register

// Write/Read Operations 
#define WRITE_SETUP_REG 0x10
#define WRITE_CLOCK_REG 0x20
#define READ_DATA_REG   0x38

// Setup Register Settings 
#define MODE_SELF_CAL   0x40 // Self-calibration  MD1=0, MD0=1
#define GAIN_1          0x00 // 000, Gain = 1
#define UNIPOLAR_MODE   0x04 // 0: Bipolar, 1: Unipolar

// Clock Register Settings, provdide a clock to MCLK IN (Pin 2)
// CLK=1, CLKDIV=0 (For 2.4576 MHz clock)
#define CLOCK_CONFIG    0x04 



void ad7705_init();
uint16_t ad7705_read_data();


#endif // AD7705_DRIVER_H





