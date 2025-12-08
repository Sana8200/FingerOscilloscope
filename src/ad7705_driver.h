#ifndef AD7705_DRIVER_H
#define AD7705_DRIVER_H
#include <stdint.h>
#include <stdbool.h>

#define VREF        3.0f   

#define REG_CMM     0x0    // Communication Register (8-bit)
#define REG_SETUP   0x1    // Setup Register (8-bit)
#define REG_CLOCK   0x2    // Clock Register (8-bit)
#define REG_DATA    0x3    // Data Register (16-bit, read-only)
#define REG_TEST    0x4    // Test Register (8-bit) 
#define REG_NOP     0x5    // No operation
#define REG_OFFSET  0x6    // Offset Register (24-bit)
#define REG_GAIN    0x7    // Gain Register (24-bit)


#define CHN_AIN1    0x0    // AIN1(+)/AIN1(-)
#define CHN_AIN2    0x1    // AIN2(+)/AIN2(-)


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


#define MODE_NORMAL          0x0    
#define MODE_SELF_CAL        0x1    
#define MODE_ZERO_SCALE_CAL  0x2    
#define MODE_FULL_SCALE_CAL  0x3   


#define GAIN_1      0x0     
#define GAIN_2      0x1    
#define GAIN_4      0x2   
#define GAIN_8      0x3    
#define GAIN_16     0x4    
#define GAIN_32     0x5    
#define GAIN_64     0x6    
#define GAIN_128    0x7   


#define BIPOLAR     0x0    // -Vref to +Vref
#define UNIPOLAR    0x1    // 0 to +Vref


#define WRITE_SETUP_REG   0x10    
#define WRITE_CLOCK_REG   0x20    
#define CLOCK_CONFIG      0x0C    



#define ADC_OK             0
#define ADC_TIMEOUT       -1
#define ADC_INIT_FAILED   -2


void ad7705_init(uint8_t channel);
uint16_t ad7705_read_data(uint8_t channel);
float ad7705_read_voltage(uint8_t channel);
void ad7705_set_gain(uint8_t channel, int gain_value);


#endif /* AD7705_DRIVER_H */