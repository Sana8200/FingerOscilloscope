/*
 * hardware.c
 * set leds, get push botton status, get switch status, setting 7 segment display, handle interrupts, labinit, 
 *
 * 
 */

#include "hardware.h"
#include "timer.h"


// Takes an integer and writes it to the LED base address to control the 10 LEDs.
void set_leds(int led_mask)
{
    volatile int *led_pointer = (volatile int *)LED_BASE_ADDR;
    *led_pointer = led_mask;
}


// reads the status of teh push button
int get_btn(void)
{
    volatile int *push_button_pointer = (volatile int *)PUSH_BUTTON_BASE_ADDR;
    return (*push_button_pointer) & 0x01;
}


// Reads the status of the 10 toggle switches on the board, no parameter
int get_sw(void)
{
    volatile int *switch_pointer = (volatile int *)SWITCH_BASE_ADDR;
    return (*switch_pointer) & 0x3FF;
}



// writes a value to one of the six 7-segment displays
void set_display(int display_number, int value)
{
    static const int sev_seg_map[] = {
        0x40, // 0
        0x79, // 1
        0x24, // 2
        0x30, // 3
        0x19, // 4
        0x12, // 5
        0x02, // 6
        0x78, // 7
        0x00, // 8
        0x10, // 9
    };
    int bit_pattern;

    if (value >= 0 && value <= 9)
    {
        bit_pattern = sev_seg_map[value];
    }
    else
    {
        // Turning off all the segments for invalid numbers
        // bit_pattern = 0x7F;

        // for invalid numbers bigger than 9 or negative numbers show 0 on the display
        bit_pattern = sev_seg_map[0];
    }
    unsigned int displayer_address = SEV_SEG_DISPLAY_BASE_ADDR + (display_number * 0x10);

    volatile int *display_pointer = (volatile int *)displayer_address;
    *display_pointer = bit_pattern;
}




// Sets the time on the six 7-segment displays.
void set_timer_display(int hours, int minutes, int seconds)
{
    set_display(0, seconds % 10);
    set_display(1, (seconds / 10) % 10);
    set_display(2, minutes % 10);
    set_display(3, (minutes / 10) % 10);
    set_display(4, hours % 10);
    set_display(5, (hours / 10) % 10);
}



/*
 * The Interrupt Service Routine (ISR).
 * This function is automatically called by the processor every time the
 * hardware timer generates an interrupt (every 100ms). It must be very fast.
 * The code inside handle interrupt, the code that's gonna interrupt can change and modify
 * to something else to interrupt 
 */
void handle_interrupt(unsigned cause)
{
    int timeoutcount = 0;
    *TIMER_STATUS = 0;

    set_leds(timeoutcount);
    static int hours = 0;
    static int minutes = 0;
    static int seconds = 0;

    // --- Clock Logic ---
    timeoutcount++;

    if (timeoutcount >= 10)
    {
        timeoutcount = 0;
        seconds++;
        if (seconds >= 60){
            seconds = 0;
            minutes++;
        }
        if (minutes >= 60)
        {
            minutes = 0;
            hours++;
        }
        if (hours >= 24)
        {
            hours = 0;
        }

        set_timer_display(hours, minutes, seconds);
    }
}





// Initializes hardware, including the timer for interrupt generation.
void labinit(void)
{
    // Set the timer period for 100ms (3,000,000 cycles at 30MHz).
    int period_value = 3000000;
    *TIMER_PERIODL = period_value & 0xFFFF;
    *TIMER_PERIODH = (period_value >> 16) & 0xFFFF;

    *TIMER_STATUS = 0;

    *TIMER_CTRL = TIMER_CTRL_CONT | TIMER_CTRL_START | TIMER_CTRL_ITO;

    enable_interrupt();
}
