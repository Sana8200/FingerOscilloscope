#ifndef DELAY_H
#define DELAY_H

#include <stdint.h>

void delay_cycles(uint32_t cycles);
void delay_us(uint32_t microseconds);
void delay_ms(uint32_t milliseconds);
void delay_ns(uint64_t nanoseconds);

// Assembly implementation (for full control)
// extern void delay_cycles_asm(uint32_t cycles);

#endif // DELAY_H