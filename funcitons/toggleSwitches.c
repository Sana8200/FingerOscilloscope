/*
 * int get_sw(void);
 * Reads the status of the 10 toggle switches on the board 
 * no parameter 
 * the return value will be an integer where 10 least significant bits correspond to the state of the switches SW0-SW9 
 * All other bits should be 0 
 */

#define SWITCH_ADDR  0x04000010


int get_sw(void){

    // Creating a volatile pointer to the switch hardware address
    volatile int *switch_pointer = (volatile int *) SWITCH_ADDR;

    int switch_value = *switch_pointer;   // Deferencing, and just getting the value at the switch address and putting it into the new variable switch_value

    // Creating a mask (masking operation), isolating the 10 least significant bits 
    // 0x3FF = 0011 1111 1111
    int mask = 0x3FF;

    // Applying the mask using & bitwise AND operation, all bits other than 10 least significant bits will be 0 
    int result = switch_value & mask;

    return result; 
}

/* Calling the function 

   while (1) {

     int switch_state = get_sw();

     set_leds(switch_state);  

     set_display(0, switch_state);

*/
