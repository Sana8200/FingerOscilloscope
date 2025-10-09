#include "ad7705_driver.h"
#include "spi_driver.h"     // To call SPI_init() and SPI_transfer_byte()
#include "hardware.h"       // For direct access to GPIO and pin definitions

// This function must be provided by another file (e.g., timetemplate.S)
// for creating short hardware delays.
extern void delay(int cycles);

/**
 * @brief Resets the AD7705 to its default state via the RST pin.
 * Pulls the reset pin low, waits, then brings it high.
 */
void AD7705_reset(void) {
    // Pull the reset pin low to activate the reset
    *pGPIO_DATA &= ~ADC_RST_PIN;
    delay(100); // Hold reset for a short time

    // Release the reset pin
    *pGPIO_DATA |= ADC_RST_PIN;
    delay(100);
}

/**
 * @brief Initializes the AD7705.
 * Resets the chip, configures its registers, and starts the initial self-calibration.
 */
void AD7705_init(void) {
    // 1. Initialize the underlying SPI communication pins
    SPI_init();

    // 2. Perform a hardware reset on the chip
    AD7705_reset();
    // Wait a longer time for the chip's internal oscillator to stabilize
    delay(200000);

    // 3. Synchronize the serial interface. After a reset, sending at least
    //    32 high bits (4 bytes of 0xFF) ensures the ADC is ready for a command.
    *pGPIO_DATA &= ~SPI_CS_N_PIN; // Select the chip
    SPI_transfer_byte(0xFF);
    SPI_transfer_byte(0xFF);
    SPI_transfer_byte(0xFF);
    SPI_transfer_byte(0xFF);
    *pGPIO_DATA |= SPI_CS_N_PIN; // Deselect the chip
    delay(100);

    // 4. Configure the Clock Register
    *pGPIO_DATA &= ~SPI_CS_N_PIN; // Select chip
    // Send command: "The next operation is a WRITE to the CLOCK register"
    SPI_transfer_byte(AD7705_REG_CLOCK | AD7705_WRITE_OP);
    // Send the configuration value for the clock register
    SPI_transfer_byte(CLOCK_REG_CONFIG);
    *pGPIO_DATA |= SPI_CS_N_PIN; // Deselect chip
    delay(100);

    // 5. Configure the Setup Register to start a self-calibration
    *pGPIO_DATA &= ~SPI_CS_N_PIN; // Select chip
    // Send command: "The next operation is a WRITE to the SETUP register"
    SPI_transfer_byte(AD7705_REG_SETUP | AD7705_WRITE_OP);
    // Send the value that triggers a self-calibration
    SPI_transfer_byte(SETUP_REG_CONFIG_CALIBRATE);
    *pGPIO_DATA |= SPI_CS_N_PIN; // Deselect chip

    // 6. Wait for the self-calibration to complete. The DRDY (Data Ready)
    //    pin will go low when the calibration is finished.
    // DRDY is active-low, so we wait while the pin is HIGH.
    while (*pGPIO_DATA & ADC_DRDY_PIN) {
        // Do nothing, just wait.
    }
}

/**
 * @brief Reads the 16-bit data result from the ADC.
 */
uint16_t AD7705_read_data(void) {
    uint8_t high_byte, low_byte;

    // 1. Wait for the next conversion to be ready.
    // The DRDY pin will go low when a new sample is available.
    while (*pGPIO_DATA & ADC_DRDY_PIN) {
        // Do nothing, just wait.
    }

    // 2. Read the 16-bit result from the data register.
    *pGPIO_DATA &= ~SPI_CS_N_PIN; // Select chip

    // Send command: "The next operation is a READ from the DATA register"
    SPI_transfer_byte(AD7705_REG_DATA | AD7705_READ_OP);

    // Clock in the two bytes of data by sending two dummy bytes.
    high_byte = SPI_transfer_byte(0x00); // Read the Most Significant Byte
    low_byte = SPI_transfer_byte(0x00);  // Read the Least Significant Byte

    *pGPIO_DATA |= SPI_CS_N_PIN; // Deselect chip

    // 3. Combine the two 8-bit bytes into a single 16-bit value and return it.
    return ((uint16_t)high_byte << 8) | low_byte;
}