/*
 * set_display writes a value to one of the six 7-segment displays
 * The display hardware uses active-low logic = 0 in a bit position turns the correspoinding segment ON
 * display_numbers corresponds to one of the 7 segment displays the user wants to use 
 * value is the 8 bit pattern to send to the display, the value we want to see on the display 
 * This funciton doesn't have any return value 
 */


#define SEV_SEG_DISPLAY_BASE_ADDR 0x04000050


void set_display( int display_number, int value){

    static const int sev_seg_map[] = {       // Look up table for the numbers on the 7 segment display 
        0x40, // 0
        0x79, // 1
        0x24, // 2
        0x30, // 3
        0x19, // 4
        0x12, // 5
        0x02, // 6
        0x78, // 7
        0x00, // 8
        0x10, // 9
    };
     

    // If the value is valid, it will look up the digit in the array to find the correct bit pattern 
    // This part also handles cases where the input value is not valid and not a single digit
    int bit_pattern;

    if(value >= 0 && value <= 9){
        bit_pattern = sev_seg_map[value];
    } else {
        bit_pattern = 0x7F;    // Turning off all the segments for invalid numbers 
    }



    // Calculating the address for the specified display 
    // Each display address is offset by 0x10 from the previous one 
    // So it depends which displayer of 6 we want to use (0-5)
    unsigned int displayer_address = SEV_SEG_DISPLAY_BASE_ADDR + (display_number * 0x10);

    // Creating a volatile pointer to the calculated hardware address 
    volatile int *display_pointer = (volatile int *) displayer_address;


    // Writing the bit_pattern of the value to the display's memory-mapped register 
    *display_pointer = bit_pattern; 
}




// Calling the function 
// set_leds(0, 5);   displays 5 on the 0 display 




/* Simplified code code to use just the 0 HEX0 display

void sevenSegDisplay(void){
    volatile int *segm7 = (volatile int *) 0x04000050;

    *segm7 = 0x19;
    
}

*/