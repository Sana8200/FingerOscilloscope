
// main.c



// Declare the external assembly function

extern void led_test(unsigned int led_value);
// extern void switch_test(void);
// extern void led_mirror(void);
// extern void counter(void);
// extern void adder(void);
// extern void sevenSegDisplay(void);

void handle_interrupt() {

}


int main() {


    // Calling the functions 

    led_test(0x2A);
    //switch_test();
    //led_mirror();
    //counter();
    //adder();

    // sevenSegDisplay();


    return 0;
}

