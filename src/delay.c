#include "delay.h"

void delay_cycles(uint32_t cycles) {
    volatile uint32_t count = cycles / 9;  
    while (count > 0) {
        count--;
    }
}


void delay_us(uint32_t microseconds) {
    uint32_t cycles = microseconds * 30;
    delay_cycles(cycles);
}


void delay_ms(uint32_t milliseconds) {
    while (milliseconds > 0) {
        delay_us(1000);
        milliseconds--;
    }
}


void delay_ns(uint32_t nanoseconds) {
    uint32_t cycles = (nanoseconds * 3) / 100;
    if (cycles < 9) {
        cycles = 9;
    } 
    delay_cycles(cycles);
}

