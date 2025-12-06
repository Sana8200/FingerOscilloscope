
#ifndef HARDWARE_H
#define HARDWARE_H

#include <stdint.h> 
#include <stdbool.h> 

// DE10-Lite Memory-Mapped Base Addresses
#define PUSH_BUTTON_BASE_ADDR        0x040000D0
#define LED_BASE_ADDR                0x04000000
#define SEV_SEG_DISPLAY_BASE_ADDR    0x04000050
#define SWITCH_BASE_ADDR             0x04000010

#define pPUSH_BUTTONS       ((volatile uint32_t *) PUSH_BUTTON_BASE_ADDR)
#define pSWITCHES           ((volatile uint32_t *) SWITCH_BASE_ADDR)
#define pLEDS               ((volatile uint32_t *) LED_BASE_ADDR)

#define DEBOUNCE_DELAY_MS  50


// Switch bit masks
#define SW0_FREEZE      0x001   // SW0: Freeze screen
#define SW1             0x002   // SW1: 
#define SW2_GAIN2       0x004   // SW2: Gain = 2
#define SW3_GAIN4       0x008   // SW3: Gain = 4
#define SW4_GAIN8       0x010   // SW4: Gain = 8
#define SW5             0x020   // SW5: 
#define SW6_PERF_TEST   0x040   // SW6: Run Performance Anayliss test 
#define SW7             0x080   // SW7: 
#define SW8_RATE_DOWN   0x100   // SW8: Sample rate -50
#define SW9_RATE_UP     0x200   // SW9: Sample rate +50


void set_leds(int led_mask);
int get_sw(void);
int get_btn(void);

void set_display(int display_number, int value);
void display_7seg_voltage_gain(float voltage, int gain);
void set_display_raw(int display_number, int bit_pattern);

int read_gain_from_switches(int switches);
bool switch_pressed(int current, int previous, int bit);

#endif // HARDWARE_H