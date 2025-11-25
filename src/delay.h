/**
 * delay.h - Timing delay functions
 */

#ifndef DELAY_H
#define DELAY_H

#include <stdint.h>

/**
 * Delay for approximately the given number of CPU cycles
 */
void delay_cycles(uint32_t cycles);

/**
 * Delay for approximately the given number of microseconds
 */
void delay_us(uint32_t microseconds);

/**
 * Delay for approximately the given number of milliseconds
 */
void delay_ms(uint32_t milliseconds);

/**
 * Delay for approximately the given number of nanoseconds
 * Note: Minimum practical delay is ~130ns at 30 MHz
 */
void delay_ns(uint64_t nanoseconds);

#endif // DELAY_H