/*
 * get_btn reads the status of teh push button
 * The second(push) button corresponds to bit 1 of the hardware register 
 * This function isolates that bit and returns it as the least significant bit (0 or 1)
 * 0 is when the button is not pressed 
 * 1 is when the button is pressed 
 */


#define PUSH_BUTTON_BASE_ADDR  0x040000d0

int get_btn(void){

    volatile int *push_button_pointer = (volatile int *) PUSH_BUTTON_BASE_ADDR;

    int btn_value = *push_button_pointer & 0x01 ;   // Masking operation 0x01 to zero out all other bits, isoloting the button state 

    return btn_value;
}
    
