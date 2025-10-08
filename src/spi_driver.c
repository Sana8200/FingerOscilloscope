/**
 * @file spi_driver.c
 * @brief Low-level SPI bit-banging routines. 
 * Implements SPI Mode 3 (CPOL=1, CPHA=1) required by the AD7705.
 */

#include "ad7705_driver.h" // Includes pin macros and delay() prototype

// =============================================================================
// 1. SPI PIN INITIALIZATION
// =============================================================================

/**
 * @brief Sets up the GPIO pins for the SPI interface (Input/Output).
 * Configures the direction register and sets the initial high-impedance state 
 * required by the AD7705 (SPI Mode 3).
 */
void spi_setup(void) {
    volatile unsigned int direction_mask = 0;

    // --- Configure Outputs ---
    // SCK, DIN, CS, and RST are controlled by the MCU.
    direction_mask = SPI_SCK_PIN | SPI_DIN_PIN | ADC_CS_PIN | ADC_RST_PIN;
    *GPIO_PORT_DIRECTION |= direction_mask; // Set bits to '1' for Output

    // --- Configure Inputs ---
    // DOUT (MISO) and DRDY are signals coming from the ADC.
    direction_mask = SPI_DOUT_READ | ADC_DRDY_PIN;
    *GPIO_PORT_DIRECTION &= ~direction_mask; // Clear bits to '0' for Input

    // --- Set Initial Idle State (SPI Mode 3: SCK High, CS High) ---
    CS_HIGH();
    SCK_HIGH();
}

// =============================================================================
// 2. CORE SPI COMMUNICATION
// =============================================================================

/**
 * @brief Transfers one byte (8 bits) over the SPI bus.
 * Implements SPI Mode 3: Clock Idles HIGH, Data is latched on the Falling edge.
 * @param tx_byte The byte to send (Master Out).
 * @return The byte received during the transfer (Master In).
 */
unsigned char spi_transfer(unsigned char tx_byte) {
    unsigned char rx_byte = 0;

    // Loop through 8 bits, Most Significant Bit (MSB) first
    for (int i = 0; i < 8; i++) {
        
        // 1. Prepare Data (DIN)
        // Check if the MSB of tx_byte is set.
        if (tx_byte & 0x80) { 
            DIN_HIGH();
        } else {
            DIN_LOW();
        }
        tx_byte <<= 1; // Prepare the next bit for transmission

        // 2. SCK drops LOW (Falling Edge Latch)
        // The AD7705 reads the data on this falling edge.
        SCK_LOW();
        delay(10); 
        
        // 3. Read Data (DOUT)
        rx_byte <<= 1; // Shift existing received bits left
        if (*GPIO_PORT_DATA & SPI_DOUT_READ) {
            rx_byte |= 1; // Set the new LSB to 1
        }

        // 4. SCK rises HIGH (Clock returns to idle state)
        SCK_HIGH();
        delay(10); 
    }
    return rx_byte;
}
