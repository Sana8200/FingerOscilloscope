# DE10-Lite Digital Oscilloscope

A real-time digital oscilloscope implemented on the DE10-Lite development board using a RISC-V soft processor and external AD7705 16-bit ADC.

![Platform](https://img.shields.io/badge/Platform-DE10--Lite-blue)
![Processor](https://img.shields.io/badge/Processor-RISCV-green)
![ADC](https://img.shields.io/badge/ADC-AD7705%2016--bit-orange)
![Display](https://img.shields.io/badge/Display-VGA%20320x240-purple)

## Overview

This project implements a functional oscilloscope that:
- Samples analog signals (0-3.3V) via an external AD7705 sigma-delta ADC
- Displays real-time waveforms on a VGA monitor (320×240, 8-bit color)
- Provides user controls for gain, sample rate, pause, and freeze functions
- Shows live measurements on both VGA display and 7-segment displays
- Added a BNC connector so you can hook up actual oscilloscope probes, and a potentiometer for testing.


### Signal Acquisition
- **ADC**: AD7705 16-bit sigma-delta converter
- **Interface**: Bit-banged SPI Mode 3 (CPOL=1, CPHA=1)
- **Sample Rate**: Adjustable 50-500 Hz
- **Gain**: Selectable 1×, 2×, 4×, 8× 
- **Voltage Range**: 0-3.3V (unipolar mode)


### Display
- **VGA Output**: 320×240 pixels, RGB332 color encoding
- **Waveform**: Orange trace with sweep-style update
- **Grid**: Dashed grid lines (20×15 divisions)
- **Info Display**: Current voltage, min/max, gain, sample rate
- **7-Segment**: Real-time voltage and gain readout


### Controls
| Switch/Button | What it does |
|---------------|--------------|
| Button | Pause/Resume |
| SW0 | Freeze the screen |
| SW2 | Gain = 2× |
| SW3 | Gain = 4× |
| SW4 | Gain = 8× |
| SW8 | Slower sample rate (-50 Hz) |
| SW9 | Faster sample rate (+50 Hz) |


## Hardware 
- **DTEK-V RISC-V**: Terasic DE10-Lite with DTEK-V RISC-V soft processor
- **ADC**: AD7705 adc board
- **Display**: VGA monitor
- **Power**: 3.3V power for the ADC
- **potentiometer**: Potentiometer (optional, for testing voltage changes)
- **BNC**: BNC to screw terminal adapter (optional, for oscilloscope probes)


### Wiring 

```
DE10-Lite GPIO    AD7705
─────────────────────────
GPIO[0]    ───►   CS (Chip Select)
GPIO[1]    ───►   SCLK (SPI Clock)
GPIO[2]    ───►   DIN (MOSI)
GPIO[3]    ◄───   DOUT (MISO)
GPIO[4]    ◄───   DRDY (Data Ready)
GPIO[5]    ───►   RESET
3.3V       ───►   VCC, REF+
GND        ───►   GND, REF-
```

## Project Structure

```
src/
├── main.c                  # Main loop - everything starts here
├── ad7705_driver.c/.h      # ADC communication
├── spi_driver.c/.h         # SPI bit-banging
├── vga_driver.c/.h         # Drawing stuff on screen
├── vga_text.c              # Text/font rendering
├── timer.c/.h              # Timer for consistent sampling
├── hardware.c/.h           # Switches, LEDs, 7-segment
├── delay.c/.h              # Delay functions
├── boot.S                  # Startup code
├── dtekv-lib.c/.h          # Debug printing via JTAG
├── dtekv-script.lds        # Linker script
└── Makefile
```

## How to Build and Run

### Prerequisites
- RISC-V toolchain (riscv32-unknown-elf-gcc)
- MCB32tools

### Compile
```bash
cd src
make
```

### Run
```bash
jtagd --user-start
dtekv-run main.bin
```

If it doesn't start, try:
```bash
killall jtagd
jtagd --user-start
dtekv-run main.bin
```

### Clean
```bash
make clean
```

## Some Technical Notes

### SPI
Since DTEK-V doesn't have hardware SPI, we had to bit-bang it ourselves. It runs at about 1 MHz and uses Mode 3 (clock idles high, sample on rising edge). The ADC does self-calibration when it starts up and whenever you change the gain.

### VGA
The frame buffer sits at `0x08000000`. We use Bresenham's algorithm for drawing lines and a column-erase approach so the display doesn't flicker when updating.

### Calibration
There's a calibration factor (around 1.535) in the ADC driver to account for the voltage divider and component tolerances. If your readings are off, you might need to tweak this value in `ad7705_driver.c`.


### Memory Map
| Address | Peripheral |
|---------|------------|
| 0x04000000 | LEDs |
| 0x04000010 | Switches |
| 0x04000020 | Timer |
| 0x04000040 | JTAG UART |
| 0x04000050 | 7-Segment Display |
| 0x040000D0 | Push Buttons |
| 0x040000E0 | GPIO |
| 0x08000000 | VGA Frame Buffer |

## Calibration

The voltage reading includes a calibration factor (1.535×) to compensate for:
- Input voltage divider effects
- ADC reference voltage tolerance
- Component variations

To recalibrate, apply known voltages and adjust the factor in `ad7705_driver.c`.

## Course Information

Developed for **IS1200 HT25 Datorteknik** at KTH Royal Institute of Technology.

## License

This project is under a proprietary license.
+ [COPYING](src/COPYING) for license information.
+ [LICENSE](LICENSE.md)

## Authors
- Sana Monhaseri 

---

*DE10-Lite Oscilloscope - IS1200 Project 2025 - TcomK*

