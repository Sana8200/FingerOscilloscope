
#include "hardware.h"
#include "lib.h"
#include "timer.h"

// setting displays, 5, 4, 3, 2, 1, 0  : hourse, hours, minutes, minutes, seconds, seconds
void set_timer_display(int hours, int minutes, int seconds) {

    set_display(0, seconds % 10);        
    set_display(1, (seconds / 10) % 10); 

    set_display(2, minutes % 10);        
    set_display(3, (minutes / 10) % 10); 

    set_display(4, hours % 10);          
    set_display(5, (hours / 10) % 10);     
}

#define PAUSE_SWITCH_BIT 6    // SW6 is used to Pause/Resum the program 
#define EXIT_SWITCH_BIT  7    // SW7 is used to exit the program 

#define LED_TICK  0x01
#define LED_EXIT 0x80
#define LED_PAUSE 0x40

/*
 * clock_timer
 * Runs the primary clock loop using the hardware timer.
 * We replaced all delay() calls with a non-blocking check of the timer's hardware flag, makeing the program efficient and responsive. 
 */
void clock_timer(void){
  int timeoutcount = 0;
  int hours = 0;
  int minutes = 0;
  int seconds = 0;

  set_timer_display(0, 0, 0);

  // Main infinite loop 
  while (1) {
    int led_state = 0;
    int switch_state = get_sw();


     // --- Exit Condition ---
    if (( switch_state >> EXIT_SWITCH_BIT) & 0x1) {   
      set_leds(LED_EXIT);      
      break;    
    }
 
    int is_paused = (switch_state >> PAUSE_SWITCH_BIT) & 0x01;
    if(is_paused){
      led_state |= LED_PAUSE;   
    } 

    if(!is_paused){
      // This condition is only true for very brief moment every 100ms 
      if (*TIMER_STATUS & TIMER_STATUS_TO) { 

        // A 100ms timeout event has occurred, Reset the hardware flag immediately.
        *TIMER_STATUS = TIMER_STATUS_TO;

        // --- 10Hz Blink Logic ---
        set_leds(led_state | LED_TICK);
        delay(25); 

        timeoutcount++;

        // Check if a full second has passed (10 * 100ms). (called once in 10 timeouts)
        if(timeoutcount >= 10){
          // Reset for the next second
          timeoutcount = 0; 

          // Now, incrementing the actual time 
          seconds++;
          if (seconds >= 60) {
            seconds = 0;
            minutes++;
          }
          if (minutes >= 60) {
            minutes = 0;
            hours++;
          }
          if (hours >= 24) {
            hours = 0;
          }

          // The display will be updated only once per second
          set_timer_display(hours, minutes, seconds);
        }      
      }
    }
     
    set_leds(led_state);
      
    // --- Handle User Input PUSH BUTTON (runs continuously) ---
    if (get_btn() == 1) {
      int value_to_set = switch_state & 0x3F;
      int selector = (switch_state >> 8) & 0x3;
            
      if (selector == 1 && value_to_set < 60) {        
        seconds = value_to_set;
      } else if (selector == 2 && value_to_set < 60) { 
        minutes = value_to_set;
      } else if (selector == 3 && value_to_set < 24) { 
        hours = value_to_set;
      }   
      set_timer_display(hours, minutes, seconds);
      delay(100);
    }
  }
  display_string("Program Ended.");
  set_timer_display(0,0,0);
}

