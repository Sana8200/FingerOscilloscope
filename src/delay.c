/**
 * delay.c - Timing delay functions for RISC-V at 30 MHz
 * 
 * These are software delays using busy loops. Not precise, but sufficient
 * for SPI timing where we need minimum delays rather than exact timing.
 */

#include "delay.h"

/**
 * Delay for approximately the specified number of CPU cycles
 * Each iteration of the loop takes approximately 3-4 cycles on RISC-V
 */
void delay_cycles(uint32_t cycles) {
    // Volatile to prevent optimization
    volatile uint32_t count = cycles / 4;  // Approximate loop overhead
    while (count > 0) {
        count--;
    }
}

/**
 * Delay for approximately the specified number of microseconds
 * At 30 MHz: 1 µs = 30 cycles
 */
void delay_us(uint32_t microseconds) {
    // Each iteration ~4 cycles, so need ~7-8 iterations per µs at 30 MHz
    uint32_t iterations = microseconds * 8;
    delay_cycles(iterations);
}

/**
 * Delay for approximately the specified number of milliseconds
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
 * 
 * Note: Very short delays (< 100ns) may not be accurate due to
 * function call overhead. Minimum achievable delay is ~4 cycles (~133ns).
 */
void delay_ns(uint64_t nanoseconds) {
    // 30 MHz = 33.33 ns per cycle
    // cycles = nanoseconds / 33.33 = nanoseconds * 3 / 100
    uint32_t cycles = (uint32_t)((nanoseconds * 3) / 100);
    
    // Minimum delay is a few cycles due to function overhead
    if (cycles < 4) {
        cycles = 4;
    }
    
    delay_cycles(cycles);
}