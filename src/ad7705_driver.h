#ifndef AD7705_DRIVER_H
#define AD7705_DRIVER_H

#include <stdint.h>


// AD7705 Register Commands page 16 datasheet 
#define REG_COMM        0x00 // Communications Register
#define REG_SETUP       0x10 // Setup Register
#define REG_CLOCK       0x20 // Clock Register
#define REG_DATA        0x30 // Data Register

// REG_COMM Write operations table 11
#define WRITE_SETUP_REG 0x10
#define WRITE_CLOCK_REG 0x20

// REG_COMM Read operations
#define READ_DATA_REG   0x38

// REG_SETUP flags_ table 14 from datasheet
#define MODE_SELF_CAL   0x40 // Self-calibration  MD1=0, MD0=1
#define GAIN_1          0x00 // Gain = 1
#define UNIPOLAR_MODE   0x04 // Unipolar mode

// The datasheet (Figure 21) uses 0x0C.
// 0x0C = 0b00001100 -> CLK=1, CLKDIV=1 (For 4.9152 MHz clock)
#define CLOCK_CONFIG    0x0C 




void ad7705_init();
uint16_t ad7705_read_data();


#endif // AD7705_DRIVER_H