
/* hardware.h
   Header file for hardware interaction functions. This file provides the "public interface" for the hardware.c implementation file.
*/

#ifndef HARDWARE_H
#define HARDWARE_H

// Hardware memory addresses
// define, preprocessor directive, macro 
#define LED_BASE_ADDR ((volatile unsigned int*)0x04000000)
#define SWITCH_BASE_ADDR        0x04000010
#define SEV_SEG_DISPLAY_BASE_ADDR 0x04000050
#define PUSH_BUTTON_BASE_ADDR   0x040000d0


void labinit(void);
void handle_interrupt(unsigned cause);

void set_leds(int led_mask);
void set_display(int display_number, int value);

int get_sw(void);
int get_btn(void);

#endif /* HARDWARE_H */