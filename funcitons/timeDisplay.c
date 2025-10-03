
/*
 * set_all_displays
 * This function takes hours, minutes, and seconds and displays them across the six 7-segment displays.
 * Displays 5 & 4: Hours
 * Displays 3 & 2: Minutes
 * Displays 1 & 0: Seconds
 */


#include "hardware.h"


void set_timer_display(int hours, int minutes, int seconds) {
    // Display seconds (rightmost two displays: 0 and 1)
    set_display(0, seconds % 10);        
    set_display(1, (seconds / 10) % 10); 

    // Display minutes (middle two displays: 2 and 3)
    set_display(2, minutes % 10);        
    set_display(3, (minutes / 10) % 10); 

    // Display hours (leftmost two displays: 4 and 5)
    set_display(4, hours % 10);          
    set_display(5, (hours / 10) % 10);   
    
}

/*
  To display a number like 36, we isolate each digit. We get the ones digit (6) using the modulo operator (36 % 10), 
  and the tens digit (3) using integer division (36 / 10). Each digit is then sent to its respective display.
  % operator for the tens digit is just for safety, ensuring that we just send one digit to the displayers 
*/