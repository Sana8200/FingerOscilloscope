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
     
    int bit_pattern;

    if(value >= 0 && value <= 9){
        bit_pattern = sev_seg_map[value];
    } else {
        // Turning off all the segments for invalid numbers 
        bit_pattern = 0x7F;    

        // for invalid numbers bigger than 9 or negative numbers show 0 on the display 
        // bit_pattern = sev_seg_map[0]; 
    }

    // Calculating the address for the specified display 
    unsigned int displayer_address = SEV_SEG_DISPLAY_BASE_ADDR + (display_number * 0x10);
    volatile int *display_pointer = (volatile int *) displayer_address;
    *display_pointer = bit_pattern; 
}
