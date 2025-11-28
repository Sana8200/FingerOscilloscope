/**
 * Timing software delay functions using busy loops (f = 30 MHz) 
 * not precise but sufficient for SPI timting where we need minimum delays (rather than excat timing)
 * calibrated and tested on board for getting the approximate numbers
 */

#include "delay.h"

/*
 * Frequency (de10-lite DTEK-V) = 30 MHZ (30,000,000 cycles/second)
 * Period(1 cycle) = 1 / 30000000 = 33.33 ns
 * 1 MicroSeconds = 1000 ns / 33.33 ns = 30 cycles 
 * each while loop iteration takes 9 cycles 
*/

/**
 * Delay for approximately the specified number of CPU cycles, Each iteration of the loop approximately takes 9 cycles 
 */
void delay_cycles(uint32_t cycles) {
    volatile uint32_t count = cycles / 9;  
    while (count > 0) {
        count--;
    }
}


/**
 * Delay for approximately the specified number of microseconds
 * At 30 MHz: 1 µs = 30 cycles 
 */
void delay_us(uint32_t microseconds) {
    uint32_t cycles = microseconds * 30;
    delay_cycles(cycles);
}

/**
 * Delay for approximately the specified number of milliseconds, 1 ms = 1000 µs, so we call delay_us(1000) for each millisecond
 */
void delay_ms(uint32_t milliseconds) {
    while (milliseconds > 0) {
        delay_us(1000);
        milliseconds--;
    }
}


/**
 * Delay for approximately the specified number of nanoseconds
 * At 30 MHz: 1 cycle = 33.33 ns 
 * cycles = nanoseconds / 33.33 = nanoseconds * 3 / 100
 * Very short delay and may not be accurate due 
 * Minimum achievable delay is ~9 cycles (~300ns). 
 */
void delay_ns(uint64_t nanoseconds) {
    uint32_t cycles = (uint32_t)((nanoseconds * 3) / 100);
    if (cycles < 9) {
        cycles = 9;
    } 
    delay_cycles(cycles);
}



// This is inline assembly function for delay_cycles, there is also assembly funciton but the inline version is faster, 
// and better for compiler optimization, inlines the function, No function overhead
/*
inline void delay_cycles(uint32_t cycles) {
    uint32_t iterations = cycles / 9;  // Adjust for loop overhead
    __asm__ volatile (
        "1: addi %0, %0, -1\n"   // Decrement counter (1 cycle)
        "   bnez %0, 1b\n"       // Branch if not zero (2-3 cycles + pipeline stalls)
        : "+r" (iterations)      // Input/output: iterations register
        :                        // No additional inputs
        : "memory"               // Memory clobber (prevents optimization)
    );
}
*/