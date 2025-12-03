#include "hardware.h"
#include "delay.h"

static const int sev_seg_map[] = {   // Look up table for the numbers on the 7 segment display 
    0xCF, // 0
    0xF9, // 1
    0xA4, // 2
    0xB0, // 3
    0x99, // 4
    0x92, // 5
    0x82, // 6
    0xF8, // 7
    0x80, // 8
    0x90, // 9
};

#define SEG_MIDDLE_DASH    0xBF
#define SEG_BOTTOM_DASH    0xF7 
#define SEG_BLANK   0xFF 
#define SEG_DP      0x7F  

// Takes an integer and writes it to the LED base address to control the 10 LEDs.
void set_leds(int led_mask){
    volatile int * led_pointer = (volatile int *) LED_BASE_ADDR;
    *led_pointer = led_mask;
}


// reads the status of teh push button
int get_btn(void){
    return (*pPUSH_BUTTONS) & 0x01 ;    
}
    

// Reads the status of the 10 toggle switches on the board, no parameter
int get_sw(void){
    return (*pSWITCHES) & 0x3FF; 
}


// Write raw pattern to display
void set_display_raw(int display_number, int bit_pattern) {
    unsigned int displayer_address = SEV_SEG_DISPLAY_BASE_ADDR + (display_number * 0x10);
    volatile int *display_pointer = (volatile int *) displayer_address;
    *display_pointer = bit_pattern;
}

// writes a value to one of the six 7-segment displays
void set_display( int display_number, int value){
    // If the value is valid, it will look up the digit in the array to find the correct bit pattern 
    int bit_pattern;

    if(value >= 0 && value <= 9){
        bit_pattern = sev_seg_map[value];
    } else {
        bit_pattern = SEG_BLANK;               
    }
    // Calculating the address for the specified display 
    set_display_raw(display_number, bit_pattern);
}


// 7-Segment Display - Show voltage and gain 
void display_7seg_voltage_gain(float voltage, int gain) {
    // Display format: X.XX V : HEX displays 5-0: [5][4][3][2][1][0]  => We use: [5]=tens, [4]=ones, [3]='.', [2]=tenths, [1]=dash, [0]=gain
    
    // Clamp to 0-9.99
    if (voltage < 0) voltage = 0;
    if (voltage > 9.99f) voltage = 9.99f;
    
    int v_hundredths = (int)(voltage * 100 + 0.5f);  // e.g., 1.65V -> 165
    int ones = (v_hundredths / 100) % 10;
    int tenths = (v_hundredths / 10) % 10;
    int hundredths = v_hundredths % 10;
    
    set_display(5, ones);       
    set_display_raw(4, SEG_DP);   
    set_display(3, tenths); 
    set_display(2, hundredths);
    set_display_raw(1, SEG_MIDDLE_DASH);   
    set_display(0, gain);
}


// checks if a switch just pressed (rising edge) with debouncing 
bool switch_pressed(int current, int previous, int bit) {
    if ((current & bit) && !(previous & bit)) {
        delay_ms(DEBOUNCE_DELAY_MS);  // rising edge detection for debouncing 
        int confirmed = get_sw();  
        return (confirmed & bit) != 0;
    }
    return false;
}


// Check Switches for gain change 
int read_gain_from_switches(int switches) {
    if (switches & SW3_GAIN4) {       
        return 4;
    } else if (switches & SW2_GAIN2) { 
        return 2; 
    } else if (switches & SW4_GAIN8){  
        return 8;     
    }
    return 1;  // Default gain
} 