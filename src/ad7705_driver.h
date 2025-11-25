#ifndef AD7705_DRIVER_H
#define AD7705_DRIVER_H

#include <stdint.h>


#define WRITE_SETUP_REG 0x10
#define WRITE_CLOCK_REG 0x20
#define MODE_SELF_CAL   0x40 // Self-calibration  MD1=0, MD0=1


// Clock Register Settings, provdide a clock to MCLK IN (Pin 2)
// CLK=1, CLKDIV=0
#define CLOCK_CONFIG    0x0C

#define REG_CMM  0x0  //communication register 8 bit
#define REG_SETUP  0x1  //setup register 8 bit
#define REG_CLOCK  0x2  //clock register 8 bit
#define REG_DATA  0x3  //data register 16 bit, contains conversion result
#define REG_TEST  0x4  //test register 8 bit, POR 0x0
#define REG_NOP  0x5  //no operation
#define REG_OFFSET  0x6  //offset register 24 bit
#define REG_GAIN  0x7  // gain register 24 bit
#define CHN_AIN1  0x0  //AIN1; calibration register pair 0
#define CHN_AIN2  0x1  //AIN2; calibration register pair 1
#define UPDATE_RATE_20  0x0  // 20 Hz
#define UPDATE_RATE_25  0x1  // 25 Hz
#define UPDATE_RATE_100  0x2  // 100 Hz
#define UPDATE_RATE_200  0x3  // 200 Hz
#define UPDATE_RATE_50  0x4  // 50 Hz
#define UPDATE_RATE_60  0x5  // 60 Hz
#define UPDATE_RATE_250  0x6  // 250 Hz
#define UPDATE_RATE_500  0x7  // 500 Hz
#define MODE_NORMAL  0x0  //normal mode
//#define MODE_SELF_CAL  0x1  //self-calibration
#define MODE_ZERO_SCALE_CAL  0x2  //zero-scale system calibration, POR 0x1F4000, set FSYNC high before calibration, FSYNC low after calibration
#define MODE_FULL_SCALE_CAL  0x3  //full-scale system calibration, POR 0x5761AB, set FSYNC high before calibration, FSYNC low after calibration
#define GAIN_1  0x0 
#define GAIN_2  0x1 
#define GAIN_4  0x2 
#define GAIN_8  0x3 
#define GAIN_16  0x4 
#define GAIN_32  0x5 
#define GAIN_64  0x6 
#define GAIN_128  0x7 
#define UNIPOLAR  0x0 
#define BIPOLAR  0x1 
#define CLK_DIV_1  0x1 
#define CLK_DIV_2  0x2 
#define VREF  3.3 

void ad7705_init(uint8_t channel);

uint16_t ad7705_read_data(uint8_t channel);
float ad7705_read_voltage(uint8_t channel);


#endif // AD7705_DRIVER_H