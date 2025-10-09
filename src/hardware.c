#include "hardware.h"
#include "timer.h" // Includes timer.h for register access
#include "ad7705_driver.h" // Needed to call AdcReadSample()
#include <stdint.h>
#include <stdbool.h>

// --- Global variables for the simple clock display ---
static int g_hours = 0;
static int g_minutes = 0;
static int g_seconds = 0;
static int g_timeout_counter = 0; // Counts up to 10 for 1-second updates (100ms interval)


/* Reads the status of the push button (1 if pressed, 0 otherwise). */
int GetBtn(void) {
    volatile int *button_pointer = (volatile int *)PUSH_BUTTON_BASE;
    return (*button_pointer) & 0x01; 
}

/* Writes a single value (0-9) to one of the six 7-segment displays. */
void SetDisplay(int display_number, int value) {
    // This function implements the complex logic to map numbers (0-9) to 
    // the correct 7-segment bit patterns (omitted for brevity).
    // The pointer calculation and assignment is done here.
}


/* Sets the time across the six 7-segment displays. */
void SetTimerDisplay(int hours, int minutes, int seconds) {
    // Logic to set individual digits using SetDisplay()
    // ... (omitted for brevity)
}


/*
 * The Interrupt Service Routine (ISR) is called by the timer (e.g., every 100ms).
 * This is where high-frequency tasks like ADC sampling occur.
 */
void HandleInterrupt(unsigned cause) {
    // 1. Clear the Timer interrupt flag immediately
    *TIMER_STATUS = 0; 

    // 2. Sample the ADC and store the data in the buffer
    AdcReadSample(); 

    // 3. Simple clock update logic 
    g_timeout_counter++;

    if (g_timeout_counter >= 10) {
        g_timeout_counter = 0;
        g_seconds++;
        
        // Logic to update minutes/hours
        if (g_seconds >= 60) { g_seconds = 0; g_minutes++; }
        if (g_minutes >= 60) { g_minutes = 0; g_hours++; }
        if (g_hours >= 24) { g_hours = 0; }

        SetTimerDisplay(g_hours, g_minutes, g_seconds);
    }
}


/* Initializes all hardware components. */
void LabInit(void) {
    // 1. Set the timer period for 100ms
    const int period_value = 3000000; 
    *TIMER_PERIODL = period_value & 0xFFFF;
    // Set TIMER_PERIODH here...

    // 2. Start timer: Continuous mode | Start | Interrupt On
    *TIMER_CTRL = TIMER_CTRL_CONT | TIMER_CTRL_START | TIMER_CTRL_ITO;
    *TIMER_STATUS = 0; // Clear initial status

    // 3. Initialize the ADC (sets up SPI GPIO, performs reset/calibration)
    AdcInit();
    
    // 4. Enable system interrupts
    EnableInterrupts();
}



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
    
    // Store the current state for the next check (debounce logic)
    button_pressed_last = button_current;
}
