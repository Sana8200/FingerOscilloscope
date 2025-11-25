/**
 * ad7705_driver.h - Driver for AD7705 16-bit Sigma-Delta ADC
 * 
 * The AD7705 is a 2-channel, 16-bit sigma-delta ADC with:
 * - 2 fully differential analog input channels
 * - On-chip programmable gain amplifier (1-128)
 * - On-chip digital filter
 * - SPI-compatible serial interface (Mode 3)
 * - Self-calibration and system calibration modes
 */

#ifndef AD7705_DRIVER_H
#define AD7705_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// AD7705 Register Addresses (for Communication Register RS2-RS0 bits)
// ============================================================================
#define REG_CMM     0x0    // Communication Register (8-bit)
#define REG_SETUP   0x1    // Setup Register (8-bit)
#define REG_CLOCK   0x2    // Clock Register (8-bit)
#define REG_DATA    0x3    // Data Register (16-bit, read-only)
#define REG_TEST    0x4    // Test Register (8-bit) - do not use
#define REG_NOP     0x5    // No operation
#define REG_OFFSET  0x6    // Offset Register (24-bit)
#define REG_GAIN    0x7    // Gain Register (24-bit)

// ============================================================================
// Channel Selection
// ============================================================================
#define CHN_AIN1    0x0    // AIN1(+)/AIN1(-), calibration pair 0
#define CHN_AIN2    0x1    // AIN2(+)/AIN2(-), calibration pair 1

// ============================================================================
// Output Update Rates (for Clock Register FS1-FS0 bits)
// These depend on MCLK frequency and CLKDIV setting
// Values below are for MCLK = 4.9152 MHz, CLKDIV = 0
// ============================================================================
#define UPDATE_RATE_20    0x0    // 20 Hz  (-3dB @ 5.24 Hz)
#define UPDATE_RATE_25    0x1    // 25 Hz  (-3dB @ 6.55 Hz)
#define UPDATE_RATE_100   0x2    // 100 Hz (-3dB @ 26.2 Hz)
#define UPDATE_RATE_200   0x3    // 200 Hz (-3dB @ 52.4 Hz)
// For MCLK = 2.4576 MHz or MCLK = 4.9152 MHz with CLKDIV = 1:
#define UPDATE_RATE_50    0x0    // 50 Hz
#define UPDATE_RATE_60    0x1    // 60 Hz
#define UPDATE_RATE_250   0x2    // 250 Hz
#define UPDATE_RATE_500   0x3    // 500 Hz

// ============================================================================
// Operating Modes (for Setup Register MD1-MD0 bits)
// ============================================================================
#define MODE_NORMAL          0x0    // Normal operation
#define MODE_SELF_CAL        0x1    // Self-calibration
#define MODE_ZERO_SCALE_CAL  0x2    // Zero-scale system calibration
#define MODE_FULL_SCALE_CAL  0x3    // Full-scale system calibration

// ============================================================================
// Gain Settings (for Setup Register G2-G0 bits)
// ============================================================================
#define GAIN_1      0x0    // Gain = 1   (Input range: 0 to Vref)
#define GAIN_2      0x1    // Gain = 2   (Input range: 0 to Vref/2)
#define GAIN_4      0x2    // Gain = 4   (Input range: 0 to Vref/4)
#define GAIN_8      0x3    // Gain = 8   (Input range: 0 to Vref/8)
#define GAIN_16     0x4    // Gain = 16  (Input range: 0 to Vref/16)
#define GAIN_32     0x5    // Gain = 32  (Input range: 0 to Vref/32)
#define GAIN_64     0x6    // Gain = 64  (Input range: 0 to Vref/64)
#define GAIN_128    0x7    // Gain = 128 (Input range: 0 to Vref/128)

// ============================================================================
// Polarity (for Setup Register B/U bit)
// ============================================================================
#define BIPOLAR     0x0    // Bipolar operation: -Vref to +Vref
#define UNIPOLAR    0x1    // Unipolar operation: 0 to +Vref

// ============================================================================
// Reference Voltage
// ============================================================================
#define VREF        3.3f   // Reference voltage in volts

// ============================================================================
// Legacy Compatibility Defines (from original code)
// ============================================================================
#define WRITE_SETUP_REG   0x10    // Write to setup register, channel 0
#define WRITE_CLOCK_REG   0x20    // Write to clock register, channel 0
#define CLOCK_CONFIG      0x0C    // CLK=1, CLKDIV=0, update rate=0

// ============================================================================
// Public API Functions
// ============================================================================

/**
 * Initialize the AD7705 ADC
 * Performs hardware reset, SPI reset, and self-calibration
 * @param channel: Channel to initialize (CHN_AIN1 or CHN_AIN2)
 */
void ad7705_init(uint8_t channel);

/**
 * Read raw 16-bit ADC data (blocking)
 * @param channel: Channel to read
 * @return: 16-bit ADC value (0-65535 for unipolar mode)
 */
uint16_t ad7705_read_data(uint8_t channel);

/**
 * Read raw ADC data with timeout
 * @param channel: Channel to read
 * @param data: Pointer to store result
 * @return: true if successful, false if timeout
 */
bool ad7705_read_data_timeout(uint8_t channel, uint16_t *data);

/**
 * Read ADC and convert to voltage
 * @param channel: Channel to read
 * @return: Voltage in volts
 */
float ad7705_read_voltage(uint8_t channel);

/**
 * Check if new data is ready (non-blocking)
 * @param channel: Channel to check
 * @return: true if data ready
 */
bool ad7705_data_ready(uint8_t channel);

#endif // AD7705_DRIVER_H