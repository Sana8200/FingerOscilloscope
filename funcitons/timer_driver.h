

#ifndef TIMER_DRIVER_H
#define TIMER_DRIVER_H

#define TIMER_CONTROL_ADDR   ((volatile unsigned int *) 0x04000020)   // Control Register address 
#define TIMER_STATUS_ADDR    ((volatile unsigned int *) 0x04000024)   // Timer status register address 
#define TIMER_PERIOD_L_ADDR  ((volatile unsigned int *) 0x04000028)   // Timer period Low address
#define TIMER_PERIOD_H_ADDR  ((volatile unsigned int *) 0x0400002C)   // Timer period high address 


#define TIMER_CONTROL_START   (1 << 0)   // Bit 0 starts the timer
#define TIMER_CONTROL_STOP    (1 << 1)   // Bit 1 stops the timer
#define TIMER_STATUS_TIMEOUT  (1 << 0)   // Bit 0 is the "timeout" flag



#define CLOCK_FREQUENCY 30000000


void timer_init_1s();
void timer_wait();


#endif // TIMER_DRIVER_H

 