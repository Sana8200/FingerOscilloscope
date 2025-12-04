# DE10-Lite Digital Oscilloscope

A real-time digital oscilloscope implemented on the DE10-Lite FPGA development board using a RISC-V soft processor and external AD7705 16-bit ADC.

![Platform](https://img.shields.io/badge/Platform-DE10--Lite-blue)
![Processor](https://img.shields.io/badge/Processor-RISC--V%20-green)
![ADC](https://img.shields.io/badge/ADC-AD7705%2016--bit-orange)
![Display](https://img.shields.io/badge/Display-VGA%20320x240-purple)

## Overview

This project implements a functional oscilloscope that:
- Samples analog signals (0-3.3V) via an external AD7705 sigma-delta ADC
- Displays real-time waveforms on a VGA monitor (320×240, 8-bit color)
- Provides user controls for gain, sample rate, pause, and freeze functions
- Shows live measurements on both VGA display and 7-segment displays

## Features

### Signal Acquisition
- **ADC**: AD7705 16-bit sigma-delta converter
- **Interface**: Bit-banged SPI Mode 3 (CPOL=1, CPHA=1)
- **Sample Rate**: Adjustable 50-500 Hz
- **Gain**: Selectable 1×, 2×, 4×, 8× (internal PGA)
- **Voltage Range**: 0-3.3V (unipolar mode)

### Display
- **VGA Output**: 320×240 pixels, RGB332 color encoding
- **Waveform**: Orange trace with sweep-style update
- **Grid**: Dashed grid lines (20×15 divisions)
- **Info Display**: Current voltage, min/max, gain, sample rate
- **7-Segment**: Real-time voltage and gain readout

### User Controls
| Control | Function |
|---------|----------|
| Button | Pause/Resume |
| SW0 | Freeze display |
| SW2 | Gain = 2× |
| SW3 | Gain = 4× |
| SW4 | Gain = 8× |
| SW8 | Sample rate -50 Hz |
| SW9 | Sample rate +50 Hz |

## Hardware Requirements

- **FPGA Board**: Terasic DE10-Lite with DTEK-V RISC-V soft processor
- **ADC**: AD7705 breakout board
- **Display**: VGA monitor
- **Power**: 3.3V reference for ADC

### Wiring Diagram

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
├── main.c                  # Main application loop
├── ad7705_driver.c/.h      # AD7705 ADC driver
├── spi_driver.c/.h         # Bit-banged SPI implementation
├── vga_driver.c/.h         # VGA graphics driver
├── vga_text.c              # Font rendering (5×7 bitmap)
├── timer.c/.h              # Hardware timer for sampling
├── hardware.c/.h           # LEDs, switches, 7-segment
├── delay.c/.h              # Software timing delays
├── boot.S                  # RISC-V boot code
├── dtekv-lib.c/.h          # JTAG UART printing
├── dtekv-script.lds        # Linker script
├── Makefile                # Build configuration
└── tools/                  # Programming utilities
```

## Building

### Prerequisites
- RISC-V toolchain (`riscv32-unknown-elf-gcc`)
- MCB32tools environment
- Make

### Compile
```bash
cd src
make build          # Build with -O3 optimization
```

### Program and Run
```bash
make run            # Upload and run on DE10-Lite
```

### Clean
```bash
make clean          # Remove build artifacts
```

## Technical Details

### SPI Implementation
The AD7705 is interfaced using software bit-banging since the DTEK-V lacks hardware SPI:
- Clock frequency: ~1 MHz (500ns half-period)
- Mode 3: Clock idles high, data sampled on rising edge
- Self-calibration performed at startup and on gain changes

### VGA Driver
- Frame buffer at `0x08000000`
- RGB332 format (3-3-2 bits for R-G-B)
- Bresenham's algorithm for line drawing
- Efficient row-by-row rectangle filling
- Column-erase technique for flicker-free sweep updates

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

Developed for **IS1500 Computer Organization and Components** at KTH Royal Institute of Technology.

## License

See [COPYING](src/COPYING) for license information.

## Authors

- Farshid [Surname]
- [Partner Name]

---

*DE10-Lite Oscilloscope - IS1500 Project 2025*

### License
This project is under a proprietary license.