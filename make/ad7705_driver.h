#ifndef AD7705_DRIVER_H
#define AD7705_DRIVER_H

#include <stdint.h>


// AD7705 Register Commands
#define REG_COMM        0x00 // Communications Register
#define REG_SETUP       0x10 // Setup Register
#define REG_CLOCK       0x20 // Clock Register
#define REG_DATA        0x30 // Data Register

// REG_COMM Write operations
#define WRITE_SETUP_REG 0x10
#define WRITE_CLOCK_REG 0x20

// REG_COMM Read operations
#define READ_DATA_REG   0x38

// REG_SETUP flags
#define MODE_SELF_CAL   0xC0 // Self-calibration
#define GAIN_1          0x00 // Gain = 1
#define UNIPOLAR_MODE   0x04 // Unipolar mode

// REG_CLOCK flags
#define CLOCK_DIV_1     0x04 // Master Clock /1


#define CLOCK_REG_CONFIG           (0x0C)
#define SETUP_REG_CONFIG_CALIBRATE (0x10)
#define SETUP_REG_READ_CMD         (AD7705_SELECT_SETUP_REG | AD7705_READ_OP)



void ad7705_init();
uint16_t ad7705_read_data();


#endif // AD7705_DRIVER_H