#ifndef TIMER_H
#define TIMER_H

/*
 * timer.h
 * Contains definitions for the memory-mapped timer
 */

// --- Timer Memory-Mapped Register Pointers ---
// Registers are 16-bit wide. We use int pointers and adjust access accordingly.

#define TIMER_BASE_ADDR      0x04000020
#define TIMER_STATUS    ((volatile int *)(TIMER_BASE_ADDR + 0x0))  // Offset 0, 0x04000020 + 0x0 = 0x4000020   
#define TIMER_CTRL      ((volatile int *)(TIMER_BASE_ADDR + 0x4))  // Offset 4 (16-bit offset 1 * 4 bytes) = 0x04000024
#define TIMER_PERIODL   ((volatile int *)(TIMER_BASE_ADDR + 0x8))  // Offset 8 (16-bit offset 2 * 4 bytes) = 0x04000028
#define TIMER_PERIODH   ((volatile int *)(TIMER_BASE_ADDR + 0xC))  // Offset 12 (16-bit offset 3 * 4 bytes) = 0x0400002C

// --- Timer Control Register Bits (also from Table 258) ---
// Control register 4 least significant bits, shows the different funciton,  001000.....0010 
#define TIMER_CTRL_ITO     0x1 // Bit 0: Interrupt On 
#define TIMER_CTRL_CONT    0x2 // Bit 1: Continuous mode
#define TIMER_CTRL_START   0x4 // Bit 2: START (Write 1 to start)
#define TIMER_CTRL_STOP    0x8 // Bit 3: STOP (Write 1 to stop)


// --- Timer Status Register Bits (from Table 257) ---
#define TIMER_STATUS_TO    0x1 // Bit 0: Timeout Flag
#define TIMER_STATUS_RUN   0x2 // Bit 1: Timer is running

#endif /* TIMER_H */