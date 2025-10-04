/*
 * int get_sw(void);
 * Reads the status of the 10 toggle switches on the board 
 */

#define SWITCH_ADDR  0x04000010

int get_sw(void){
    volatile int *switch_pointer = (volatile int *) SWITCH_ADDR;  
    return *switch_pointer & 0x3FF; 
}
