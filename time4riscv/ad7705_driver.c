/**
 * @file ad7705_driver.c
 * @brief AD7705 specific configuration and data reading routines.
 * This file handles the logic for talking to the AD7705 ADC.
 */

#include "ad7705_driver.h"

// =============================================================================
// 1. AD7705 COMMUNICATION PRIMITIVES
// =============================================================================

/**
 * @brief Performs a hardware reset on the AD7705 using the RST pin.
 */
void ad7705_hardware_reset(void) {
    // 1. Pull RST low to initiate the reset sequence.
    RST_LOW();
    delay(1000); 
    
    // 2. Raise RST high to return to normal operating mode.
    RST_HIGH();
    delay(100000); // Wait for chip to stabilize before configuration.
}

/**
 * @brief Helper function to write a configuration byte to an AD7705 register.
 * This function handles the Chip Select and sequential SPI transfers for writing.
 * @param comm_byte Communication byte (register address + WRITE flag).
 * @param data_byte The 8-bit data to write (the actual setting).
 */
static void ad7705_write_reg(unsigned char comm_byte, unsigned char data_byte) {
    CS_LOW(); // Start communication
    spi_transfer(comm_byte); // Send the target register address and 'Write' command.
    spi_transfer(data_byte); // Send the actual configuration data.
    CS_HIGH(); // End communication
}

/**
 * @brief Reads the contents of a specific AD7705 register.
 * @param comm_byte Communication byte (register address + READ flag).
 * @return The 8-bit data read from the register.
 */
static unsigned char ad7705_read_reg(unsigned char comm_byte) {
    unsigned char received_byte;
    CS_LOW(); // Start communication
    spi_transfer(comm_byte); // Send the target register address and 'Read' command.
    // Send a dummy byte (0xFF) to clock in the register data.
    received_byte = spi_transfer(0xFF); 
    CS_HIGH(); // End communication
    return received_byte;
}

/**
 * @brief Waits until the DRDY pin signals data is ready (Active Low).
 * This is a blocking function, ensuring we read the data only after conversion completes.
 */
void ad7705_wait_for_data_ready(void) {
    volatile unsigned int timeout = 1000000; 
    
    // Loop while the DRDY pin is HIGH (data not ready) AND we haven't timed out.
    while (DRDY_READ() && timeout-- > 0) { 
        // Spin lock (waiting)
    }
    
    if (timeout <= 0) {
        print_string("AD7705 Timeout Error! Data Ready pin not asserted.\n");
    }
}


// =============================================================================
// 2. AD7705 INITIALIZATION AND DATA READING
// =============================================================================

/**
 * @brief Initializes and configures the AD7705 for operation.
 * Configures the clock source, update rate, and forces a self-calibration.
 */
void ad7705_init(void) {
    // 1. Hardware Reset: Ensure the chip is in a known good state.
    ad7705_hardware_reset();

    // 2. Software Reset: Send 32 '1's (4 bytes of 0xFF) to resync SPI interface.
    CS_LOW();
    for (int i = 0; i < 4; i++) {
        spi_transfer(0xFF); 
    }
    CS_HIGH();
    delay(1000); 

    // 3. Write Clock Register: Set 1kHz update rate.
    ad7705_write_reg(REG_CLOCK_WRITE, AD7705_CLOCK_INIT);

    // --- DIAGNOSTIC: Read back Clock Register to verify write ---
    // Clock Read Command is 0x28 (Clock Register: 010, Read: 1, Channel 1: 0).
    unsigned char clock_read_val = ad7705_read_reg(0x28); 
    print_string("Clock Register Readback: ");
    print_integer(clock_read_val);
    print_string(" (Expected 12)\n"); 
    // --- END DIAGNOSTIC ---


    // 4. Write Setup Register: Set Normal mode, Gain=1, Unipolar, AND force SELF-CALIBRATION.
    ad7705_write_reg(REG_SETUP_WRITE, AD7705_SETUP_INIT);
    
    // 5. Wait for the calibration to complete before continuing to main loop.
    print_string("Waiting for AD7705 Self-Calibration...\n");
    ad7705_wait_for_data_ready();
    ad7705_wait_for_data_ready(); // Wait two full cycles to ensure stability.
    print_string("Calibration complete.\n");
}

/**
 * @brief Reads the 16-bit ADC conversion result from the Data Register.
 * @return The 16-bit raw ADC value (0 to 65535).
 */
unsigned int ad7705_read_data(void) {
    unsigned char high_byte, low_byte;
    unsigned int result; 
    
    // 1. Wait for a new measurement to be completed.
    ad7705_wait_for_data_ready();
    
    // 2. Send Data Register Read command.
    CS_LOW();
    spi_transfer(REG_DATA_READ); 

    // 3. Read 16 bits (High byte first, Low byte second).
    high_byte = spi_transfer(0xFF); 
    low_byte = spi_transfer(0xFF);  

    CS_HIGH();

    // 4. Combine bytes: Shift high byte left by 8 and combine with the low byte.
    result = ((unsigned int)high_byte << 8) | low_byte;

    return result;
}
