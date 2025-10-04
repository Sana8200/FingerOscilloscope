/*
 * set_all_displays
 * This function takes hours, minutes, and seconds and displays them across the six 7-segment displays.
 * Displays 5 & 4: Hours
 * Displays 3 & 2: Minutes
 * Displays 1 & 0: Seconds
 */

#include "hardware.h"

void set_timer_display(int hours, int minutes, int seconds) {

    set_display(0, seconds % 10);        
    set_display(1, (seconds / 10) % 10); 

    set_display(2, minutes % 10);        
    set_display(3, (minutes / 10) % 10); 

    set_display(4, hours % 10);          
    set_display(5, (hours / 10) % 10);   
    
}
