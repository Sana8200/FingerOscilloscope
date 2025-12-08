#include "timer.h"

// Initializes the hardware timer to tick at a specific frequency
void timer_init(int target_frequency_hz) {

    *TIMER_CTRL = TIMER_CTRL_STOP; 
    *TIMER_STATUS = 0;

    // the period count => Clock_Freq / Target_Freq (target frequency we can modify)
    int period_count = SYSTEM_CLOCK_FREQ / target_frequency_hz;

    *TIMER_PERIODL = (period_count & 0xFFFF);        
    *TIMER_PERIODH = ((period_count >> 16) & 0xFFFF); 

    *TIMER_CTRL = TIMER_CTRL_START | TIMER_CTRL_CONT;
}


bool timer_check_tick() {
    if (*TIMER_STATUS & TIMER_STATUS_TO) {
        *TIMER_STATUS = 0; 
        return true;
    }
    return false;
}
