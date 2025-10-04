/*
 * set_leds(int led_mask)
 * Takes an integer and writes it to the LED base address to control the 10 LEDs.
 */

 #define LED_BASE_ADDR  0x04000000

void set_leds(int led_mask){
    volatile int * led_pointer = (volatile int *) LED_BASE_ADDR;
    *led_pointer = led_mask;
}



