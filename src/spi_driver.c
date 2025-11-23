#include "spi_driver.h"
#include "hardware.h" 

static uint32_t pio_output_state;

// Simple delay to satisfy AD7705 timing requirements ($t_{14}, t_{15} > 100ns$).
// On a 30MHz CPU, 1 cycle is ~33ns. A loop of 10 is approx 300ns 
static void spi_delay() {
    for (volatile int i = 0; i < 10; i++) {
        __asm("nop"); // Prevents compiler optimization
    }
}



void spi_init() {
    // Read current direction register
    uint32_t direction = *pGPIO_DIRECTION;

    // Set Directions:
    // Outputs: CS (0), SCK (1), MOSI (2), RST (5)
    // Inputs:  MISO (3), DRDY (4)
    direction |= (SPI_CS_PIN | SPI_SCK_PIN | SPI_MOSI_PIN | ADC_RST_PIN);
    direction &= ~(SPI_MISO_PIN | ADC_DRDY_PIN);
    *pGPIO_DIRECTION = direction;

    // Set Initial Pin States
    // SPI Mode 3 Idle state: SCK High
    // Chip Select: High (Inactive)
    // Reset: High (Not resetting)
    pio_output_state = *pGPIO_DATA;
    pio_output_state |= (SPI_CS_PIN | ADC_RST_PIN | SPI_SCK_PIN);
    *pGPIO_DATA = pio_output_state;
}



// Selects the ADC chip (pulls CS low)
void spi_select_chip() {
    pio_output_state &= ~SPI_CS_PIN; // Pull CS Low
    *pGPIO_DATA = pio_output_state;
    spi_delay(); 
}


// Deselects the ADC chip (pulls CS high)
void spi_deselect_chip() {
    pio_output_state |= SPI_CS_PIN; // Pull CS High
    *pGPIO_DATA = pio_output_state;
    spi_delay();
}



void spi_reset_pin(bool high) {
    if (high) {
        pio_output_state |= ADC_RST_PIN;
    } else {
        pio_output_state &= ~ADC_RST_PIN;
    }
    *pGPIO_DATA = pio_output_state;
}



void spi_wait_for_ready() {
    // Poll DRDY pin. Wait while it is HIGH (1).
    // DRDY goes LOW (0) when data is available.
    while ((*pGPIO_DATA & ADC_DRDY_PIN) != 0);
}



uint8_t spi_transfer_byte(uint8_t byte_out) {
    uint8_t byte_in = 0;

    // 1. Ensure Clock starts HIGH (Idle for Mode 3)
    pio_output_state |= SPI_SCK_PIN;
    *pGPIO_DATA = pio_output_state;

    for (int i = 0; i < 8; i++) {
        // 2. Setup MOSI (Output)
        if (byte_out & 0x80) pio_output_state |= SPI_MOSI_PIN;
        else                 pio_output_state &= ~SPI_MOSI_PIN;
        
        // 3. Drop SCK LOW (Leading Edge - Slave puts data on MISO here)
        pio_output_state &= ~SPI_SCK_PIN;
        *pGPIO_DATA = pio_output_state;
        
        spi_delay(); 

        // 4. Read MISO (Input) - Sample BEFORE raising the clock
        byte_in <<= 1;
        if (*pGPIO_DATA & SPI_MISO_PIN) {
            byte_in |= 1;
        }

        // 5. Raise SCK HIGH (Trailing Edge - Latch data)
        pio_output_state |= SPI_SCK_PIN;
        *pGPIO_DATA = pio_output_state;
        
        spi_delay();

        byte_out <<= 1;
    }
    return byte_in;
}


    
    