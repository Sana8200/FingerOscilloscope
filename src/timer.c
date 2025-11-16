#include "timer.h"


// Initializes the hardware timer to tick at a specific frequency
void timer_init(int target_frequency_hz) {
    // Writing to the control register. STOP bit is bit 3 (0x8).
    *TIMER_CTRL = TIMER_CTRL_STOP; 
    
    // Clear the Time-Out (TO) status bit
    *TIMER_STATUS = 0;

    // Calculate the period count
    // Formula: Clock_Freq / Target_Freq (target frequency we can modify)
    int period_count = SYSTEM_CLOCK_FREQ / target_frequency_hz;

    // Write the period to the Low and High registers
    // The timer loads this value when it resets.
    *TIMER_PERIODL = (period_count & 0xFFFF);        // Lower 16 bits
    *TIMER_PERIODH = ((period_count >> 16) & 0xFFFF); // Upper 16 bits

    // 5. Start the timer
    // We set START (bit 2) and CONT (bit 1) for continuous mode.
    // 0x4 | 0x2 = 0x6
    *TIMER_CTRL = TIMER_CTRL_START | TIMER_CTRL_CONT;
}


bool timer_check_tick() {
    // Check bit 0 (TO - Timeout) of the status register (We need to check if timer reached 0)
    if (*TIMER_STATUS & TIMER_STATUS_TO) {
        // Clear the interrupt/status bit by writing 0 to it. (Writing anything clears it, so we write 0)
        *TIMER_STATUS = 0; 
        return true;
    }
    return false;
}
