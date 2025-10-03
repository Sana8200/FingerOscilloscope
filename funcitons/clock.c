// Need to call and include the other functions from other files to compile it 


/*
 * clock_timer
 * Runs the primary clock loop. It handles timekeeping, display updates, and user input for setting the time or exiting.
 */
void clock(void){
    
  // Initialize time variables
  int hours = 0;
  int minutes = 0;
  int seconds = 0;


  // Clearing LEDs and displays before starting for ensuring 
  set_leds(0);
  set_timer_display(0, 0, 0);


  // Entering the main infinite loop for the clock.
  while (1) {

    // --- Time Keeping ---
    // This section increments the time every second.
    delay(3000); // Wait for approximately one second.
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
      hours = 0; // Clock resets after 24 hours.
    }

    // --- Display Update ---
    // Update the 7-segment displays with the new time.
    set_timer_display(hours, minutes, seconds);

    // --- Button and Switch Input Handling ---
    // Check if a button is pressed to modify the time.
    if (get_btn() == 1) {
      set_leds(0x200); // Turn on LED9 for visual feedback on button press

      int switch_state = get_sw();

      // Isolate the value from the 6 right-most switches (SW0-SW5).
      int value_to_set = switch_state & 0x3F;

      // Isolate the selector from the 2 left-most switches (SW9 and SW8).
      // 01 = seconds, 10 = minutes, 11 = hours.
      int selector = (switch_state >> 8) & 0x3;

      if (selector == 1) { // Binary 01: Modify seconds
        if (value_to_set < 60) seconds = value_to_set;
      } else if (selector == 2) { // Binary 10: Modify minutes
        if (value_to_set < 60) minutes = value_to_set;
      } else if (selector == 3) { // Binary 11: Modify hours
        if (value_to_set < 24) hours = value_to_set;
      }

      // Update the display immediately after setting a new time.
      set_timer_display(hours, minutes, seconds);

      // Add a small delay and turn off the feedback LED.
      delay(500);
      set_leds(0);
    }


    // --- Exit Condition ---
    // SW7 is used to exit the program 
    if ((get_sw() >> 7) & 0x1) {
      break;  // Exit the loop if SW7 is high (up).
    }
  }

  // --- Program End ---
  // Clear displays and LEDs to indicate the program has ended.
  display_string("Program End.");
  set_timer_display(0,0,0);
  set_leds(0);

}