#ifndef ADC_SAMPLING_H
#define ADC_SAMPLING_H
#include <stdint.h> // Includes standard integer types like uint16_t for fixed-width integers.
  
/**
 * @brief Reads a single 16-bit sample from the AD7705 using SPI and stores it
 * in the global circular waveform buffer.
 * This function is intended to be called by a high-frequency Timer Interrupt
 * Service Routine (ISR).
 */

// Declares a function that:
// -> Reads a single 16-bit sample from the AD7705 ADC via SPI.
// -> Stores the sample in a global circular waveform buffer.
// -> Is designed to be called from a high-frequency Timer Interrupt Service Routine (ISR) to maintain a steady sampling rate.
 void sample_adc_and_update_buffer();

#endif // ADC_SAMPLING_H

/* This header file does not contain the actual implementation, but the declaration.
   It allows main.c to call sample_adc_and_update_buffer() without knowing 
   the details of how SPI communication or the buffer works.*/