#include "timer.h"




// Initializes the hardware timer to tick at a specific frequency
void timer_init(int target_frequency_hz) {
    // 1. Stop the timer first to clear state
    // Writing to the control register. STOP bit is bit 3 (0x8).
    *TIMER_CTRL = TIMER_CTRL_STOP; 
    
    // 2. Clear the Time-Out (TO) status bit
    // Writing 0 to status clears the TO bit (based on TIMER.pdf)
    *TIMER_STATUS = 0;

    // 3. Calculate the period count
    // Formula: Clock_Freq / Target_Freq
    // Example: 30,000,000 / 100 = 300,000 ticks for 10ms (100Hz)
    int period_count = SYSTEM_CLOCK_FREQ / target_frequency_hz;

    // 4. Write the period to the Low and High registers
    // The timer loads this value when it resets.
    *TIMER_PERIODL = (period_count & 0xFFFF);        // Lower 16 bits
    *TIMER_PERIODH = ((period_count >> 16) & 0xFFFF); // Upper 16 bits

    // 5. Start the timer
    // We set START (bit 2) and CONT (bit 1) for continuous mode.
    // 0x4 | 0x2 = 0x6
    *TIMER_CTRL = TIMER_CTRL_START | TIMER_CTRL_CONT;
}


bool timer_check_tick() {
    // Check bit 0 (TO - Timeout) of the status register
    if (*TIMER_STATUS & TIMER_STATUS_TO) {
        // The timer reached 0.
        // Clear the interrupt/status bit by writing 0 to it.
        *TIMER_STATUS = 0; 
        return true;
    }
    return false;
}
