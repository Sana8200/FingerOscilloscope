
#include "timer_driver.h"

void timer_init_1s() {
    // Set the timer's period to be exactly one second
    *TIMER_PERIOD_L_ADDR = CLOCK_FREQUENCY;
    *TIMER_PERIOD_H_ADDR = 0;

    // Start the timer in continuous mode
    *TIMER_CONTROL_ADDR = TIMER_CONTROL_START;
}

void timer_wait() {
    // Wait in a loop until the timeout flag in the status register is set
    while ((*TIMER_STATUS_ADDR & TIMER_STATUS_TIMEOUT) == 0) {
        // Do nothing, just wait. The processor is "polling" the hardware.
    }
    
    // Reset the timeout flag for the next wait.
    // We do this by writing a '1' to that bit.
    *TIMER_STATUS_ADDR = TIMER_STATUS_TIMEOUT;
}