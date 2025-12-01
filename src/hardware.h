
#ifndef HARDWARE_H
#define HARDWARE_H

#include <stdint.h> 
#include <stdbool.h> 

// DE10-Lite Memory-MappedBase Addresses
#define PUSH_BUTTON_BASE_ADDR        0x040000D0
#define LED_BASE_ADDR                0x04000000
#define SEV_SEG_DISPLAY_BASE_ADDR    0x04000050
#define SWITCH_BASE_ADDR             0x04000010

#define pPUSH_BUTTONS       ((volatile uint32_t *) PUSH_BUTTON_BASE_ADDR)
#define pSWITCHES           ((volatile uint32_t *) SWITCH_BASE_ADDR)
#define pLEDS               ((volatile uint32_t *) LED_BASE_ADDR)


void set_leds(int led_mask);
int get_sw(void);
int get_btn(void);
void set_display(int display_number, int value);
void display_voltage_7seg(float voltage);

#endif // HARDWARE_H