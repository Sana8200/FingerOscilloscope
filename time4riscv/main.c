// main.c

// Declare the external assembly function.
extern void led_test(unsigned int led_value);
extern void switch_test(void);
extern void led_mirror(void);
extern void counter(void);
extern void adder(void);




void handle_interrupt() {}


int main() {
    // Defining the pattern we want to display on the 10 LEDs.
    // 0x07c is 0b0001111100 in binary.
    unsigned int pattern = 0x07c;

    // Calling the assembly functions 
    //led_test(pattern);
    //switch_test();
    //led_mirror();
    //counter();
    adder();

    return 0;
}


