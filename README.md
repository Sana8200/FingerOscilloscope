# FingerOsilloscope

A real-time digital oscilloscope built on the DTEK-V board. This project captures, displays, and analyzes analog voltage signals as an advanced embedded systems exercise.

##  Key Features

* **Live Waveform Display:** Renders signals on a VGA monitor with a measurement grid.
* **Run/Stop Control:** Freeze the live display for detailed inspection.
* **Save & Replay:** Record waveform snapshots to memory and view them later from a menu.
* **Zoom & Pan:** Adjust time/voltage scales and shift the wave using onboard buttons and knobs.
* **Dual-Mode Operation:** Switch seamlessly between live capture and replaying saved signals.

## 🛠️ How It Works

The system is built in C for direct hardware control. It uses the board's **ADC** to continuously sample input voltage into a **circular buffer**. A **framebuffer** in memory is used to draw the UI and waveform, which is then sent to a VGA monitor via a simple resistor-based DAC. The entire user interface is managed by a **state machine** to handle different modes like `LIVE`, `PAUSED`, and `MENU`.

##  Verification

The oscilloscope's accuracy is verified using a function generator to confirm that waveform shapes, voltage levels, and time measurements are displayed correctly. All buttons and features are tested to ensure they work as expected in every mode.

##  Core Challenge

The main technical challenge is managing real-time tasks: generating a stable VGA signal while continuously sampling the ADC and responding to user input without flicker or data loss.

## 👥 Contributors

* Sana Monhaseri
* Rachel Lachiani

## 📜 License

This project is licensed under the MIT License.
