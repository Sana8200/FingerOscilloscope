/*
 * get_btn reads the status of teh push button
 */

#define PUSH_BUTTON_BASE_ADDR  0x040000d0

int get_btn(void){
    volatile int *push_button_pointer = (volatile int *) PUSH_BUTTON_BASE_ADDR;
    int btn_value = *push_button_pointer & 0x01 ;   
    return btn_value;
}
    


