
#include "delay.h"

// Inline assembly delay - most accurate
inline void delay_cycles(uint32_t cycles) {
    __asm__ volatile (
        "1: addi %0, %0, -1\n"
        "   bnez %0, 1b\n"
        : "+r" (cycles)
    );
}

// Microsecond delay (30 MHz clock = 33.33 ns per cycle)
// Approximately 30 cycles per microsecond
void delay_us(uint32_t microseconds) {
    // Each iteration takes about 4-5 cycles
    // So we need roughly (30 * microseconds) / 4 iterations
    uint32_t cycles = (microseconds * 15) ;
    delay_cycles(cycles);
}

// Millisecond delay
void delay_ms(uint32_t milliseconds) {
    while (milliseconds--) {
        delay_us(1000);
    }
}

// Nanosecond delay (for fine SPI timing adjustments)
void delay_ns(uint64_t nanoseconds) {
    // 30 MHz = 33.33 ns per cycle
    // Calculate cycles needed
    uint64_t cycles = nanoseconds *  (3.0 / 200.0);
    if (cycles < 1) cycles = 1;
    delay_cycles(cycles);
}
