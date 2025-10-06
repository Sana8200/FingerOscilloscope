
#include "ad7705_driver.h"


/* ========================================================================= */
/* --- MAIN APPLICATION --- */
/* This file contains the high-level application logic. It initializes     */
/* the hardware and the ADC, then enters a loop to read and display data.  */
/* ========================================================================= */

int main() {
    unsigned int adc_value;
    spi_setup();
    ad7705_init();

    print_string("AD7705 Initialized. Starting main loop.\n");

    while (1) {

        ad7705_wait_for_data_ready();

        adc_value = ad7705_read_data();
        
        *LED_REG = adc_value >> 6;

        print_string("ADC Value: ");
        print_integer(adc_value);
        print_string("\n");
    }

    return 0; 
}