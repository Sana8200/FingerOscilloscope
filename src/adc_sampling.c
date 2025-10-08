#include "adc_sampling.h"
#include "framebuffer.h"    // Gives access to waveform_data[] and waveform_write_index
#include "ad7705_driver.h"  // Gives access to spi_send_byte(), spi_receive_byte(), etc.
#include <stdint.h>

// --- HARDWARE DEFINITIONS ---
// CRITICAL: These are external definitions that must be defined elsewhere (e.g., in main.c)
// Using the address from your DTEK-V documentation
extern volatile unsigned int * const GPIO_DATA_REG; 

// Define the Chip Select Pin mask (example: bit 5)
// CRITICAL: Verify this mask matches your hardware connection.
#define CS_PIN        (1 << 5) 

// Define the AD7705 command for reading data (adjust if necessary)
#define CMD_DATA_READ 0x10 

/**
 * @brief Reads a single 16-bit sample from the AD7705 using SPI and stores it
 * in the global circular waveform buffer.
 * * NOTE: This function is designed to be executed very quickly inside the 
 * Timer Interrupt Service Routine (ISR) to maintain a steady sample rate.
 */
void sample_adc_and_update_buffer() {
    unsigned char high_byte, low_byte;
    uint16_t adc_value;

    // 1. SPI Communication to read the 16-bit data from AD7705 
    // Set Chip Select LOW to initiate communication
    *GPIO_DATA_REG &= ~CS_PIN;       
    
    // Send command to the ADC to prepare to transmit data
    spi_send_byte(CMD_DATA_READ);     
    
    // Read the two bytes of the 16-bit result
    high_byte = spi_receive_byte(); 
    low_byte = spi_receive_byte();  
    
    // Set Chip Select HIGH to finish transaction
    *GPIO_DATA_REG |= CS_PIN;        

    // 2. Process the result
    // Combine the two 8-bit bytes into a single 16-bit value
    adc_value = (high_byte << 8) | low_byte;

    // 3. Store the new sample in the global circular buffer
    // The 'volatile' keyword ensures the compiler handles this shared memory correctly.
    waveform_data[waveform_write_index] = adc_value;

    // 4. Advance the index (using the modulo operator to ensure it wraps around)
    // This is the "circular" part; it overwrites the oldest data point.
    waveform_write_index = (waveform_write_index + 1) % WAVEFORM_BUFFER_SIZE;
}

