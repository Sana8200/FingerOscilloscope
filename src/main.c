
#include "ad7705_driver.h"     
#include <stdint.h>



int main() {
    unsigned char high_byte, low_byte;
    unsigned int adc_value;

    // First, configuring the GPIO pins for SPI
    spi_setup();


    // Printing a startup message to the console
    print_string("DTEK-V ADC Reader Initialized.\n");
    delay(10000); // Small delay to ensure message prints



    // --- 1. Configure the AD7705 ---
    print_string("Configuring AD7705...\n");
    
    *GPIO_DATA_ADDR &= ~CS_PIN;       // Set Chip Select LOW to start talking
    spi_send_byte(CMD_SETUP_WRITE);   // Send command: "prepare to receive setup data"
    spi_send_byte(SETUP_CONFIG);      // Send the actual setup configuration
    *GPIO_DATA_ADDR |= CS_PIN;        // Set Chip Select HIGH to finish
    
    print_string("Configuration complete. Entering main loop.\n");

    // --- 2. Main Loop: Continuously Read Data ---
    while (1) {
        delay(100000); // Wait a bit between readings to control sample rate

        // Tell the ADC we want to read the 16-bit data
        *GPIO_DATA_ADDR &= ~CS_PIN;       // Chip Select LOW
        spi_send_byte(CMD_DATA_READ);     // Send command: "prepare to send data"
        
        // Receive the two bytes of data
        high_byte = spi_receive_byte(); // Read the most significant 8 bits
        low_byte = spi_receive_byte();  // Read the least significant 8 bits
        
        *GPIO_DATA_ADDR |= CS_PIN;        // Chip Select HIGH

        // --- 3. Process and Display the Result ---
        
        // Combine the two 8-bit bytes into a single 16-bit unsigned integer
        adc_value = (high_byte << 8) | low_byte;

        // Display the 10 most significant bits on the LEDs (The baord has 10 LEDs at LEDS_ADDR)
        *LEDD_ADDR = adc_value >> 6;

        // Print the full numeric value to the console
        print_string("ADC Value: ");
        print_integer(adc_value);
        print_string("\n");
     
    }

    return 0; // This line will never be reached
}



