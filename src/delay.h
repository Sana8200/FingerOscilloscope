#ifndef DELAY_H
#define DELAY_H

#include <stdint.h>

void delay_cycles(uint32_t cycles);
void delay_us(uint32_t microseconds);
void delay_ms(uint32_t milliseconds);
void delay_ns(uint32_t nanoseconds);


#endif  /* DELAY_H */