#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

// --- Timer Memory-Mapped Register Pointers ---
// Base address for the timer hardware
#define TIMER_BASE_ADDR      0x04000020
// Pointers to the status, control, and period registers
#define TIMER_STATUS    ((volatile int *)(TIMER_BASE_ADDR + 0x0))
#define TIMER_CTRL      ((volatile int *)(TIMER_BASE_ADDR + 0x4))
#define TIMER_PERIODL   ((volatile int *)(TIMER_BASE_ADDR + 0x8))
#define TIMER_PERIODH   ((volatile int *)(TIMER_BASE_ADDR + 0xC))

// --- Timer Control Register Bits ---
#define TIMER_CTRL_ITO     0x1 // Bit 0: Enable Interrupt
#define TIMER_CTRL_CONT    0x2 // Bit 1: Continuous mode
#define TIMER_CTRL_START   0x4 // Bit 2: Start timer
#define TIMER_CTRL_STOP    0x8 // Bit 3: Stop timer

// --- Timer Status Register Bits ---
#define TIMER_STATUS_TO    0x1 // Bit 0: Timeout Flag (Clear this in the ISR)
#define TIMER_STATUS_RUN   0x2 // Bit 1: Timer is running


#endif // TIMER_H






