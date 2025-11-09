#include "spi_driver.h"
#include "hardware.h" 

static uint32_t pio_output_state;

// Small delay for SPI bit-banging timing.
static void spi_delay() {
    for (volatile int i = 0; i < 10; i++); 
}

// Initializes the SPI GPIO pins
void spi_init(){
    uint32_t direction = *pGPIO_DIRECTION;

    direction |= (SPI_CS_PIN | SPI_SCK_PIN | SPI_MOSI_PIN | ADC_RST_PIN);   // CS, SCK, MOSI, RST as outputs
    direction &= ~(SPI_MISO_PIN | ADC_DRDY_PIN);  // MISO, DRDY as inputs
    
    *pGPIO_DIRECTION = direction;
    pio_output_state = *pGPIO_DATA;     // Read initial state (just in case)
    pio_output_state |= (SPI_CS_PIN | ADC_RST_PIN);     // Default state: CS high (deselcted), RST high(inactive)

    // Write the initial state to the hardware
    *pGPIO_DATA = pio_output_state;
}



// Selects the ADC chip (pulls CS low)
void spi_select_chip() {
    pio_output_state &= ~SPI_CS_PIN;    // CS pin low
    *pGPIO_DATA = pio_output_state; 
    spi_delay();
}


// Deselects the ADC chip (pulls CS high)
void spi_deselect_chip() {
    pio_output_state |= SPI_CS_PIN;    // Set CS pin high
    *pGPIO_DATA = pio_output_state; 
    spi_delay();
}



void spi_reset_pin(bool high) {
    if (high) {
        pio_output_state |= ADC_RST_PIN; // High = Inactive
    } else {
        pio_output_state &= ~ADC_RST_PIN; // Low = Active Reset
    }
    *pGPIO_DATA = pio_output_state;
}


// Waits for the DRDY pin to go low, indicating data is ready
void spi_wait_for_ready() {
    while ((*pGPIO_DATA & ADC_DRDY_PIN) != 0) {   // Poll DRDY pin. Low means data is ready
        // Wait...
    }
}



// Sends one byte and receives one byte (MSB first).
uint8_t spi_transfer_byte(uint8_t byte_out) {
    uint8_t byte_in = 0;

    for (int i = 0; i < 8; i++) {
        // 1. Set MOSI (output bit) in the shadow register
        if (byte_out & 0x80) {
            pio_output_state |= SPI_MOSI_PIN;    // Set MOSI high
        } else {
            pio_output_state &= ~SPI_MOSI_PIN;   // Set MOSI low
        }
        *pGPIO_DATA = pio_output_state; // Write change
        
        // 2. Pulse SCK HIGH
        spi_delay();
        pio_output_state |= SPI_SCK_PIN;  // Set SCK high
        *pGPIO_DATA = pio_output_state;   
        spi_delay();
        
        // 3. Pulse SCK LOW
        pio_output_state &= ~SPI_SCK_PIN;   // Set SCK low
        *pGPIO_DATA = pio_output_state;
        spi_delay(); // Give time for data to be valid (t5)
        
        // 4. Read MISO (input bit) - this is a direct read
        byte_in <<= 1;      
        if (*pGPIO_DATA & SPI_MISO_PIN) {     // Read MISO input bit
            byte_in |= 1;
        }
        
        // 5. Shift to the next bit
        byte_out <<= 1;
    }
    return byte_in;     // Return the received byte
}