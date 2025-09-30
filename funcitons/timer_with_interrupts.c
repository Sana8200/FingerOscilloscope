

/* This program blinks LEDR0 on a DE10-Lite board every half-second using the interval 
   timer and ARM processor INTERRUPTS. */

#include <stdint.h>

// --- Hardware Memory Map ---
#define TIMER_BASE_ADDRESS      0x04000020
#define LED_REG_ADDRESS         0xFF200000
#define GIC_CPU_IF_ADDRESS      0xFFFEC100 // ARM GIC CPU Interface
#define GIC_DIST_ADDRESS        0xFFFED000 // ARM GIC Distributor

// --- System-Specific Constants ---
#define PERIPHERAL_CLOCK_HZ     30000000   // System clock is 30 MHz
#define TIMER_IRQ               16         // The timer is connected to IRQ 16

// --- Hardware Register Pointers ---
volatile uint16_t * const timer_status_reg  = (uint16_t *)(TIMER_BASE_ADDRESS + 0x00);
volatile uint16_t * const timer_control_reg = (uint16_t *)(TIMER_BASE_ADDRESS + 0x04);
volatile uint16_t * const timer_period_l    = (uint16_t *)(TIMER_BASE_ADDRESS + 0x08);
volatile uint16_t * const timer_period_h    = (uint16_t *)(TIMER_BASE_ADDRESS + 0x0C);
volatile uint32_t * const led_register      = (uint32_t *)LED_REG_ADDRESS;
volatile uint32_t * const GICC_CTLR         = (uint32_t *)(GIC_CPU_IF_ADDRESS + 0x00);
volatile uint32_t * const GICC_PMR          = (uint32_t *)(GIC_CPU_IF_ADDRESS + 0x04);
volatile uint32_t * const GICD_CTLR         = (uint32_t *)(GIC_DIST_ADDRESS + 0x000);
volatile uint32_t * const GICD_ISENABLER    = (uint32_t *)(GIC_DIST_ADDRESS + 0x100);




// Global variable to track the LED's current state
static volatile uint8_t led_is_on = 0;





// Forward declaration of our Interrupt Service Routine
void timer_isr(void);

/**
 * Configures the interval timer to fire an interrupt periodically.
 */
void setup_timer(uint32_t period_ms) {
    *timer_control_reg = 0; // Stop the timer for safe configuration

    // Calculate the required tick value for our desired period
    uint32_t total_ticks = (PERIPHERAL_CLOCK_HZ / 1000) * period_ms;

    // Set the period registers
    *timer_period_l = total_ticks & 0xFFFF;
    *timer_period_h = (total_ticks >> 16) & 0xFFFF;

    // Start the timer with interrupts (ITO) and continuous mode (CONT) enabled.
    *timer_control_reg = (1 << 2) | (1 << 1) | (1 << 0); // CONT | ITO | START
}





/**
 * Configures the ARM Generic Interrupt Controller (GIC).
 */
void setup_interrupts(void) {
    // Provide the GIC with the location of our Interrupt Service Routine (ISR).
    *(uint32_t *)0x20 = (uint32_t)timer_isr;

    // Enable our specific timer interrupt (IRQ 16).
    GICD_ISENABLER[0] = (1 << TIMER_IRQ);

    // Enable the GIC Distributor and the CPU's GIC Interface.
    *GICD_CTLR = 1;
    *GICC_PMR = 0xFFFF; // Accept all interrupt priorities.
    *GICC_CTLR = 1;
}




/**
 * The Interrupt Service Routine (ISR). This runs automatically on timer timeout.
 */
void timer_isr(void) {
    // Flip the state of our LED.
    led_is_on = !led_is_on;

    // Write the new state to the hardware LED register.
    if (led_is_on) {
        *led_register |= (1 << 0); // Turn LEDR0 on
    } else {
        *led_register &= ~(1 << 0); // Turn LEDR0 off
    }

    // IMPORTANT: Clear the timer's interrupt flag.
    *timer_status_reg = (1 << 0); // Write 1 to the TO bit to clear it.
}





int main(void) {
    // First, set up the ARM Generic Interrupt Controller.
    setup_interrupts();

    // Next, initialize our timer to a 500ms interval.
    setup_timer(500);

    // Finally, enable interrupts globally on the CPU.
    __asm("cpsie i");

    // Enter an infinite loop. Blinking is handled by interrupts in the background.
    while (1) {
        // The CPU is free to do other work here.
    }

    return 0; // This line is never reached.
}