#ifndef ADC_SAMPLING_H
#define ADC_SAMPLING_H

#include <stdint.h>

/**
 * @brief Reads a single 16-bit sample from the AD7705 using SPI and stores it
 * in the global circular waveform buffer.
 * * NOTE: This function is intended to be called by a high-frequency Timer Interrupt
 * Service Routine (ISR).
 */
void sample_adc_and_update_buffer();

#endif // ADC_SAMPLING_H

