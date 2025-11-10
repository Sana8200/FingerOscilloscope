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


// --- Gain Selection flags (Table 17) ---
// We map these directly to the 3 lowest switches
#define GAIN_1          0x00 // 0b000
#define GAIN_2          0x01 // 0b001
#define GAIN_4          0x02 // 0b010
#define GAIN_8          0x03 // 0b011
#define GAIN_16         0x04 // 0b100
#define GAIN_32         0x05 // 0b101
#define GAIN_64         0x06 // 0b110
#define GAIN_128        0x07 // 0b111



void ad7705_init();
uint16_t ad7705_read_data();
void set_gain(uint8_t gain_code);

#endif // AD7705_DRIVER_H