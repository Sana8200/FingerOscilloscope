
/* This program blinks LEDR0 on a DE10-Lite board every half-second  using the interval 
   timer and a simple POLLING technique. */
#include <stdint.h>



// --- Hardware Memory Map ---
#define TIMER_BASE_ADDRESS      0x04000020
#define LED_REG_ADDRESS         0xFF200000
#define PERIPHERAL_CLOCK_HZ     30000000 // System clock is 30 MHz


// --- Hardware Register Pointers ---
volatile uint16_t * const timer_status_reg  = (uint16_t *)(TIMER_BASE_ADDRESS + 0x00);
volatile uint16_t * const timer_control_reg = (uint16_t *)(TIMER_BASE_ADDRESS + 0x04);
volatile uint16_t * const timer_period_l    = (uint16_t *)(TIMER_BASE_ADDRESS + 0x08);
volatile uint16_t * const timer_period_h    = (uint16_t *)(TIMER_BASE_ADDRESS + 0x0C);
volatile uint32_t * const led_register      = (uint32_t *)LED_REG_ADDRESS;


/**
 * Configures the interval timer to run continuously.
 */
void setup_timer(uint32_t period_ms) {
    *timer_control_reg = 0; // Stop the timer for safe configuration

    // Calculate the required tick value for our desired period.
    uint32_t total_ticks = (PERIPHERAL_CLOCK_HZ / 1000) * period_ms;

    // Set the period registers.
    *timer_period_l = total_ticks & 0xFFFF;
    *timer_period_h = (total_ticks >> 16) & 0xFFFF;

    // Start the timer with continuous mode (CONT) enabled.
    // NOTE: We do NOT enable the ITO (Interrupt on Timeout) bit.
    *timer_control_reg = (1 << 2) | (1 << 0); // CONT | START
}





int main(void) {
    // A simple variable to keep track of the LED state.
    uint8_t led_is_on = 0;

    // Initialize our timer to a 500ms interval.
    setup_timer(500);

    // This is the main polling loop. The program spends all its time here.
    while (1) {
        // Checking if the Timeout bit ('TO') in the timer's status register is 1.
        if (*timer_status_reg & (1 << 0)) {


            // --- The timer has finished, so we take action ---


            // 1. Flip the state of our LED.
            led_is_on = !led_is_on;

            // 2. Write the new state to the hardware LED register.
            if (led_is_on) {
                *led_register |= (1 << 0); // Turn LEDR0 on
            } else {
                *led_register &= ~(1 << 0); // Turn LEDR0 off
            }

            // 3. Clearing the timer's timeout flag by writing 1 to it.
            *timer_status_reg = (1 << 0);
        }
    }

    return 0; // This line is never reached.
}