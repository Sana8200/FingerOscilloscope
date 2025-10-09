#include "hardware.h" // Includes GetBtn(), print functions, and hardware addresses
#include "framebuffer.h" // Includes AppState_t, STATE_LIVE, etc.
#include <stdint.h>
#include <stdbool.h>

// External function for precise, processor-specific delay
extern void delay(int cycles); 

// --- System Variables ---
// A simple millisecond counter updated by the Timer ISR
volatile int ms_counter = 0;


/* Software delay loop that executes a fixed number of CPU cycles. 
   This function is typically implemented in Assembly for accuracy. */
void delay_ms(int ms) {
    // This is a simple approximation. For actual time-accurate delays, 
    // you should rely on the Timer hardware.

    // For simplicity, we calculate a large number of cycles to delay:
    // This assumes a high clock frequency (e.g., 50,000 cycles is approx 1ms at 50MHz).
    for (int i = 0; i < ms; i++) {
        // Delay 1ms (adjust 50000 based on your actual CPU clock speed)
        delay(50000); 
    }
}


/* --- Button Handling and State Machine --- */

/* Checks for button presses and updates the main application state. */
void HandleButtonPress(void) {
    static bool button_pressed_last = false;
    
    // GetBtn() returns 1 if pressed (non-zero), 0 if not (based on hardware.c)
    bool button_current = (GetBtn() != 0); 
    
    // Check for a transition from released (false) to pressed (true) (rising edge)
    if (button_current && !button_pressed_last) {
        
        // Cycle through states: LIVE -> PAUSED -> MENU -> LIVE
        AppState++;
        if (AppState > STATE_MENU) { 
            AppState = STATE_LIVE; // Wrap back to LIVE_VIEW (0)
        }
    }
    
    // Store the current state for the next check (debounce logic)
    button_pressed_last = button_current;
}