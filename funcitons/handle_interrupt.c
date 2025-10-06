#include "timer.h"
#include "hardware.h"
#include "adc_sampling.h" // NEW: For the oscilloscope data
#include <stdint.h>

// Current Clock Logic Variables
int timeoutcount = 0;
int mytime = 0x5957;

// NEW: Counter to ensure the clock logic runs only once per second (15000 interrupts)
#define CLOCK_TICKS_PER_SECOND 15000 // Must match the hardware timer frequency (15 kHz)
static int clock_tick_counter = 0;

/** * @brief Sets the time on the six 7-segment displays.
 * Helper function
 */
void set_timer_display(int hours, int minutes, int seconds) {
    set_display(0, seconds % 10);      
    set_display(1, (seconds / 10) % 10);
    set_display(2, minutes % 10);      
    set_display(3, (minutes / 10) % 10);
    set_display(4, hours % 10);         
    set_display(5, (hours / 10) % 10);   
}

/**
 * @brief The Interrupt Service Routine (ISR).
 * This function is automatically called by the processor at the Timer frequency (e.g., 15 kHz).
 */
void handle_interrupt(unsigned cause) {
    
    // Acknowledge the interrupt by resetting the timer's timeout flag.
    *TIMER_STATUS = 0;

    // --- 1. OSCILLOSCOPE SAMPLING (Runs on every single interrupt) ---
    sample_adc_and_update_buffer();

    // --- 2. CLOCK LOGIC (Runs only once per second) ---
    
    // We increment a counter for every interrupt
    clock_tick_counter++;

    // Only run the clock update when the counter reaches the full second mark (15000 interrupts)
    if (clock_tick_counter >= CLOCK_TICKS_PER_SECOND) {
        clock_tick_counter = 0;

        // --- Your Original Clock Logic ---
        
        static int hours = 0;
        static int minutes = 0;
        static int seconds = 0;  

        seconds++;
        if(seconds >=60) { 
            seconds = 0; 
            minutes++; 
        }
        if(minutes >= 60) { 
            minutes = 0; 
            hours++; 
        }
        if(hours >= 24) { 
            hours = 0; 
        }

        // Update displays and call tick().
        set_timer_display(hours, minutes, seconds);
        tick( &mytime );

        // This line is from your original code, adjusted for the new tick logic.
        timeoutcount = 0; 
    }
    
    set_leds(timeoutcount); // Keep the LED debugging if needed
}
