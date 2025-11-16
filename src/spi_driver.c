#include "spi_driver.h"
#include "hardware.h" 

static uint32_t pio_output_state;

// Small delay for SPI bit-banging timing.
static void spi_delay() {
    for (volatile int i = 0; i < 10; i++); 
}



void spi_init(){
    uint32_t direction = *pGPIO_DIRECTION;
    // Set CS, SCK, MOSI, RST as Outputs. MISO, DRDY as Inputs.
    direction |= (SPI_CS_PIN | SPI_SCK_PIN | SPI_MOSI_PIN | ADC_RST_PIN);
    direction &= ~(SPI_MISO_PIN | ADC_DRDY_PIN);
    
    *pGPIO_DIRECTION = direction;

    // Initial State: CS High (inactive), RST High (inactive), SCK High (Mode 3 Idle)
    pio_output_state = *pGPIO_DATA;
    pio_output_state |= (SPI_CS_PIN | ADC_RST_PIN | SPI_SCK_PIN); 
    
    *pGPIO_DATA = pio_output_state;
}



// Selects the ADC chip (pulls CS low)
void spi_select_chip() {
    pio_output_state &= ~SPI_CS_PIN;    
    *pGPIO_DATA = pio_output_state; 
    spi_delay();
}


// Deselects the ADC chip (pulls CS high)
void spi_deselect_chip() {
    pio_output_state |= SPI_CS_PIN;    
    *pGPIO_DATA = pio_output_state; 
    spi_delay();
}



void spi_reset_pin(bool high) {
    if(high){
        pio_output_state |= ADC_RST_PIN;
    }else{
        pio_output_state &= ~ADC_RST_PIN;
    }

    *pGPIO_DATA = pio_output_state;
}



void spi_wait_for_ready() {
    // Wait while DRDY pin is High (1)
    while ((*pGPIO_DATA & ADC_DRDY_PIN) != 0);
}


// SPI Mode 3: CPOL=1 (Idle High), CPHA=1 (Sample on Trailing/Rising Edge)
uint8_t spi_transfer_byte(uint8_t byte_out) {
    uint8_t byte_in = 0;

    for (int i = 0; i < 8; i++) {
        // 1. Prepare MOSI Data (MSB first)
        if (byte_out & 0x80) pio_output_state |= SPI_MOSI_PIN;
        else                 pio_output_state &= ~SPI_MOSI_PIN;
        
        // 2. Clock Low (Leading Edge - Setup)
        pio_output_state &= ~SPI_SCK_PIN; 
        *pGPIO_DATA = pio_output_state;
        spi_delay();

        // 3. Clock High (Trailing Edge - Sample)
        pio_output_state |= SPI_SCK_PIN;
        *pGPIO_DATA = pio_output_state;
        
        // 4. Read MISO (Sample Input)
        byte_in <<= 1;
        if (*pGPIO_DATA & SPI_MISO_PIN) {
            byte_in |= 1;
        }
        spi_delay();

        byte_out <<= 1;
    }
    return byte_in;
}


