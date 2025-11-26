#include "hardware.h"
#include "timer.h" 
#include <stdint.h>
#include <stdbool.h>


/*
 * set_leds(int led_mask)
 * Takes an integer and writes it to the LED base address to control the 10 LEDs.
 */
void set_leds(int led_mask){
    volatile int * led_pointer = (volatile int *) LED_BASE_ADDR;
    *led_pointer = led_mask;
}

/*
 * get_btn reads the status of teh push button
 */
int get_btn(void){
    volatile int *push_button_pointer = (volatile int *) PUSH_BUTTON_BASE_ADDR;
    return (*push_button_pointer) & 0x01 ;    
}
    

/*
 * int get_sw(void);
 * Reads the status of the 10 toggle switches on the board, no parameter
 */
int get_sw(void){
    volatile int *switch_pointer = (volatile int *) SWITCH_BASE_ADDR;
    return (*switch_pointer) & 0x3FF; 
}


/*
 * set_display writes a value to one of the six 7-segment displays
 */
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
    int bit_pattern;

    if(value >= 0 && value <= 9){
        bit_pattern = sev_seg_map[value];
    } else {
        // Turning off all the segments for invalid numbers 
        // bit_pattern = 0x7F;    

        // for invalid numbers bigger than 9 or negative numbers show 0 on the display 
        bit_pattern = sev_seg_map[0];            
    }


    // Calculating the address for the specified display 
    unsigned int displayer_address = SEV_SEG_DISPLAY_BASE_ADDR + (display_number * 0x10);

    volatile int *display_pointer = (volatile int *) displayer_address;
    *display_pointer = bit_pattern; 
}





// 7-Segment Display - Show voltage 
void display_voltage_7seg(float voltage) {
    // Display format: X.XX V
    // HEX displays 5-0: [5][4][3][2][1][0]
    // We'll use: [5]=tens, [4]=ones, [3]='.', [2]=tenths, [1]=hundredths, [0]=blank
    
    // Clamp to 0-9.99
    if (voltage < 0) voltage = 0;
    if (voltage > 9.99f) voltage = 9.99f;
    
    int v_hundredths = (int)(voltage * 100 + 0.5f);  // e.g., 1.65V -> 165
    
    int ones = (v_hundredths / 100) % 10;
    int tenths = (v_hundredths / 10) % 10;
    int hundredths = v_hundredths % 10;
    
    // Display on 7-segments
    set_display(5, ones);       // Ones digit
    set_display(4, tenths);     // Tenths
    set_display(3, hundredths); // Hundredths
    // Display 2, 1, 0 could show "V" or be blank
    set_display(2, 0);
    set_display(1, 0);
    set_display(0, 0);
}


