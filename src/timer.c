#include "timer.h"

// Initializes timer to tick at a specific frequency

void timer_init(int frequency_hz) {
    // 1. Stop the timer
    *TIMER_CTRL = 0; 
    *TIMER_STATUS = 0; // Clear TO bit

    // 2. Calculate Period
    // Period = Clock_Freq / Target_Freq
    // Example: 50,000,000 / 1000 = 50,000 ticks for 1ms
    int period = 50000000 / frequency_hz;

    // 3. Set Period Registers (Low and High 16-bits)
    *TIMER_PERIODL = (period & 0xFFFF);
    *TIMER_PERIODH = ((period >> 16) & 0xFFFF);

    // 4. Start Timer: Continuous mode (CONT) + Start (START)
    // We are NOT using interrupts (ITO) to keep main.c simple (Polling mode)
    *TIMER_CTRL = TIMER_CTRL_CONT | TIMER_CTRL_START;
}

