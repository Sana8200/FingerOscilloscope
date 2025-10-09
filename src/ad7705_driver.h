#ifndef AD7705_DRIVER_H
#define AD7705_DRIVER_H

#include <stdint.h>

//=============================================================================
//           AD7705 Command Byte Definitions (for Communication Register)
//=============================================================================
// The AD7705 is controlled by writing to its Communication Register.
// A command byte is constructed by OR-ing together a register selection
// and an operation (Read or Write).
//
// Example: To prepare for reading the Data Register, the command would be:
//          AD7705_SELECT_DATA_REG | AD7705_READ_OP

// --- Register Selection Bits (RS2, RS1, RS0) ---
#define AD7705_REG_COMMUNICATION   (0b000 << 2) // The Communication Register itself
#define AD7705_REG_SETUP           (0b001 << 2) // The Setup Register (configures gain, mode, etc.)
#define AD7705_REG_CLOCK           (0b010 << 2) // The Clock Control Register
#define AD7705_REG_DATA            (0b011 << 2) // The 16-bit Data Register (holds the result)
#define AD7705_REG_TEST            (0b100 << 2) // Used for factory testing
#define AD7705_REG_OFFSET          (0b101 << 2) // The Offset (zero-scale) Calibration Register
#define AD7705_REG_GAIN            (0b110 << 2) // The Gain (full-scale) Calibration Register


// --- Operation Bits (R/W) ---
#define AD7705_WRITE_OP            (0b0 << 1)
#define AD7705_READ_OP             (0b1 << 1)


//=============================================================================
//                AD7705 Configuration Values
//=============================================================================

// --- Clock Register Configuration (0x0C) ---
// We choose: Master Clock Disabled (use internal), 2.4576MHz clock, 500 Hz update rate.
#define CLOCK_REG_CONFIG           (0x0C)

// --- Setup Register Configuration (0x10) ---
// We choose: Self-calibration, Gain = 1, Unipolar mode, buffer enabled.
#define SETUP_REG_CONFIG_CALIBRATE (0x10)

// --- Setup Register Read Command ---
// Command to read the Setup register to check calibration status.
#define SETUP_REG_READ_CMD         (AD7705_SELECT_SETUP_REG | AD7705_READ_OP)


//=============================================================================
//                     AD7705 Driver Function Prototypes
//=============================================================================

/**
 * @brief Resets the AD7705 to its default state via the RST pin.
 */
void AD7705_reset(void);

/**
 * @brief Initializes the AD7705.
 * Resets the chip, configures the clock and setup registers, and
 * starts the initial self-calibration.
 */
void AD7705_init(void);

/**
 * @brief Reads the 16-bit data result from the ADC.
 * This function waits for the DRDY (Data Ready) pin to go low, indicating the
 * conversion is complete, and then reads the 2-byte result.
 * @return uint16_t The 16-bit unsigned conversion result.
 */
uint16_t AD7705_read_data(void);


#endif // AD7705_DRIVER_H