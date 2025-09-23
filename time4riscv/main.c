/* main.c

   This file written 2024 by Artur Podobas and Pedro Antunes

   For copyright and licensing, see file COPYING */


/* Below functions are external and found in other files. */
extern void print(const char*);
extern void print_dec(unsigned int);
extern void display_string(char*);
extern void time2string(char*,int);
extern void tick(int*);
extern void delay(int);
extern int nextprime( int );

// --- New definitions for the 7-Segment Display ---

// IMPORTANT: You must verify this memory address from your lab documentation.
#define HEX_DISPLAY_BASE ((volatile char*)0x04000010)
#define NUM_DIGITS 4

// Lookup table for 7-segment patterns (0-9) for a Common Cathode display.
// A '1' bit turns a segment ON.
const char SEGMENT_MAP[10] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F  // 9
};

// --- End of new definitions ---

int mytime = 0x5957;
char textstring[] = "text, more text, and even more text!";

/* Below is the function that will be called when an interrupt is triggered. */
void handle_interrupt(unsigned cause) 
{}



/* Add your code here for initializing interrupts. */
void labinit(void)
{}


void set_leds(int led_mask){
  volatile int *leds = (int *)0x04000000;
  *leds = led_mask;
}

/**
 * @brief Displays a number on the 7-segment displays.
 * @param value The number to display.
 */
void display_7seg(int value) {
    int i;
    // Turn off all digits first to prevent ghosting
    for (i = 0; i < NUM_DIGITS; i++) {
        HEX_DISPLAY_BASE[i] = 0x00;
    }

    // Handle zero separately
    if (value == 0) {
        HEX_DISPLAY_BASE[0] = SEGMENT_MAP[0];
        return;
    }

    // Display each digit
    for (i = 0; i < NUM_DIGITS && value > 0; i++) {
        int digit = value % 10; // Get the rightmost digit
        HEX_DISPLAY_BASE[i] = SEGMENT_MAP[digit];
        value /= 10; // Move to the next digit
    }
}

void delay_seconds(int seconds){
  // Get the memory address of the hardware timer.
  volatile int *timer = (int *)0x04000008;

  // Set the timer. Assuming the hardware clock is 1MHz,
  // 1,000,000 ticks is equal to 1 second.
  *timer = seconds * 1000000;

  // Wait in a loop until the timer hardware counts down to zero.
  while (*timer > 0);
}

/* Your code goes into main as well as any needed functions. */
int main() {
  int led_value = 0;
  
  while (led_value <= 0xF) { // 0xF is hexadecimal for 15
        
        // Update the physical LEDs with the current counter value.
        set_leds(led_value);

        // *** NEW: Update the 7-segment display with the same value. ***
        display_7seg(led_value);

        // Wait for one second.
        delay_seconds(1);
        
        
        // Increment the counter for the next iteration.
        led_value++;
    }

  // Call labinit()
  labinit();
  
  // Clear displays and LEDs when done.
  set_leds(0);
  display_7seg(0);

  return 0; // The program will end after the loop.
}