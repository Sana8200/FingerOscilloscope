# FingerOsilloscope

A real-time digital oscilloscope built on the DTEK-V board (de10-lite) and using ADC ad7705. This project captures, displays, and analyzes analog voltage signals as an advanced embedded systems exercise.

##  Key Features

* **Live Waveform Display:** Renders signals on a VGA monitor with a measurement grid.
* **Run/Stop Control:** Freeze the live display for detailed inspection.
* **Zoom & Pan:** Adjust time/voltage scales and shift the wave using onboard buttons and knobs.


## How It Works

The system is built in C for direct hardware control. 

##  Verification


##  Core Challenge

The main technical challenge is managing real-time tasks: generating a stable VGA signal while continuously sampling the ADC and responding to user input without flicker or data loss.

## Contributors

* Sana Monhaseri

### License
This project is under a proprietary license.
