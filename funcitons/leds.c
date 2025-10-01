

void set_leds(int led_mask){
    
    // Creating a volatile pointer to the LED address 
    // volatile : tells the compiler don't optimize since the address might change at this pointer 
    volatile int * led_pointer = (volatile int *) 0x04000000;

    // Defrenece the pointer to write the led_mask value to the hardware address, turning the LEDs on or off 
    *led_pointer = led_mask;

}

// Calling the function in the main.c 
// set_leds(0x2A);    0x2A is an exmple, we can modify it to turn on the desired leds 
