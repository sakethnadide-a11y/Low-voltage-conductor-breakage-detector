# Low-Voltage Conductor Breakage Detector using IoT and Edge AI

An ESP32-based prototype for monitoring electrical current and voltage, identifying abnormal operating conditions, and initiating a local fault response. The project combines sensor acquisition, an Autoencoder-based anomaly-detection workflow, and embedded inference using TensorFlow Lite for Microcontrollers.

> **Safety notice:** This is an educational prototype, not a certified protection device. Do not connect the circuit directly to live mains or overhead distribution conductors. Use appropriately rated isolated sensors, power supplies, fusing, enclosures, and supervision by a qualified electrical professional. Test with safe, isolated low-voltage sources only.

## Overview

Conductor breakage and related electrical faults can cause abnormal voltage and current behaviour. This project explores a low-cost monitoring approach in which an ESP32 reads sensor signals and evaluates current measurements using a lightweight Autoencoder model. The firmware also checks a voltage threshold.

When the configured fault condition is met, the ESP32 changes the relay output and activates a buzzer. The firmware prints sensor readings and status messages over the serial monitor.

## Features

- Continuous analogue sampling of current and voltage inputs
- Autoencoder-based reconstruction-error calculation on the ESP32
- Voltage-threshold fault check
- Relay output for a prototype isolation mechanism
- Buzzer alert for local notification
- Serial output for monitoring and debugging
- Python training and TensorFlow Lite model-conversion scripts

## System workflow

1. **Sensing:** The ESP32 reads the current-sensor signal and voltage-measurement signal through its ADC pins.
2. **Preprocessing:** The current ADC reading is scaled to a 0–1 range in the firmware.
3. **Inference:** The scaled value is passed to the embedded TensorFlow Lite model.
4. **Anomaly scoring:** The firmware calculates the absolute difference between the model output and input (reconstruction error).
5. **Fault decision:** The firmware compares the reconstruction error and measured voltage against configured thresholds.
6. **Response:** When the voltage condition or configured anomaly condition is met, the relay output is changed and the buzzer is activated. The firmware then waits for a reset.

## Hardware

The current firmware defines the following ESP32 connections:

| Component / signal | ESP32 pin | Purpose |
|---|---:|---|
| Current sensor output | GPIO 35 | Current-related analogue input |
| Voltage measurement output | GPIO 34 | Voltage-related analogue input |
| Relay control | GPIO 26 | Prototype relay output |
| Buzzer control | GPIO 27 | Audible alert |

The original project notes refer to an ACS712 current sensor, a voltage-sensing circuit, a relay module, and a buzzer. The firmware currently calculates voltage using a resistor-divider formula (`R1 = 30 kΩ`, `R2 = 10 kΩ`). Confirm the actual sensor and circuit design before wiring; the README should match the hardware you physically built.

### Important hardware considerations

- ESP32 ADC pins are not 5 V tolerant. Ensure every analogue input remains within the board's permitted voltage range.
- A resistor divider alone does not make an AC mains measurement safe or provide galvanic isolation.
- Verify relay contact ratings, isolation, default state, and driver requirements for the intended load.
- The relay logic in the sketch assumes `LOW` activates the fault/isolation state and `HIGH` is the normal state. Confirm this for your relay module.
- Do not use this prototype as the sole means of protecting people or electrical infrastructure.

## Software and tools

- **Firmware:** Arduino C++ / Arduino IDE
- **Microcontroller:** ESP32
- **Model development:** Python, TensorFlow / Keras
- **Data processing:** pandas, NumPy, scikit-learn
- **Embedded inference:** TensorFlow Lite for Microcontrollers (through the ESP32 TensorFlow Lite library)

## Repository structure

```text
low-voltage-conductor-breakage-detector/
├── sketch_sep17a.ino    # ESP32 firmware: sensing, inference and fault response
├── train_model.py      # Trains the Autoencoder and exports a .tflite model
├── convert_model.py    # Converts the .tflite model into a C/C++ header
├── model_data.h        # Embedded model byte array used by the firmware
└── README.md
```

## Machine-learning workflow

The `train_model.py` script:

1. Loads `current_reading.csv`.
2. Scales the training data using `MinMaxScaler`.
3. Trains a fully connected Autoencoder to reconstruct normal current readings.
4. Calculates reconstruction errors and derives a percentile-based threshold.
5. Exports the trained model as `anomaly_detector.tflite`.

The `convert_model.py` script reads `anomaly_detector.tflite` and writes its bytes into `model_data.h`, which is included by the ESP32 sketch.

### Dataset required for training

The training script expects a file named `current_reading.csv` in the same directory. That dataset is not included in this repository. Add your own appropriately collected and documented dataset before running the training script. Use representative normal operating data and keep separate fault/test data for evaluation.

## Setup

### 1. Clone the repository

```bash
git clone https://github.com/<YOUR_GITHUB_USERNAME>/low-voltage-conductor-breakage-detector.git
cd low-voltage-conductor-breakage-detector
```

Replace `<YOUR_GITHUB_USERNAME>` with your GitHub username.

### 2. Set up the Python environment

Use a compatible Python environment, then install the training dependencies:

```bash
python -m venv .venv
```

Activate it:

**Windows**
```powershell
.venv\Scripts\activate
```

**macOS / Linux**
```bash
source .venv/bin/activate
```

Install dependencies:

```bash
pip install pandas numpy scikit-learn tensorflow
```

### 3. Train and convert the model

Place `current_reading.csv` in the project directory, then run:

```bash
python train_model.py
python convert_model.py
```

This generates `anomaly_detector.tflite` and updates `model_data.h`.

### 4. Configure Arduino IDE

1. Install the ESP32 board support package in Arduino IDE.
2. Install a TensorFlow Lite for Microcontrollers library compatible with the included headers.
3. Open `sketch_sep17a.ino`.
4. Keep `model_data.h` in the sketch's source directory (or in the same Arduino sketch folder).
5. Select the correct ESP32 board and serial port.
6. Verify the sensor wiring and thresholds for your specific prototype before uploading.

### 5. Monitor the device

Open the Arduino Serial Monitor at **115200 baud**. The firmware prints current ADC readings, calculated voltage, and fault status messages.

## Configuration and calibration

The firmware contains configurable values near the top of `sketch_sep17a.ino`, including:

- `ANOMALY_THRESHOLD`
- `ERROR_DROP_THRESHOLD`
- `VOLTAGE_THRESHOLD`
- `R1` and `R2`
- Sensor and actuator GPIO assignments

**Calibration is required before testing.** The current sketch sets `ANOMALY_THRESHOLD` to `3500`, while the model input/output are treated as floating-point values in the 0–1 range. This value is therefore not consistent with the stated reconstruction-error scale and should not be treated as a valid calibrated threshold. Derive and apply a threshold using representative training/validation data.

Also ensure that the preprocessing used on the ESP32 matches the preprocessing used during training. The current Python script fits a `MinMaxScaler`, while the firmware divides the ADC reading by `4095`. These are not necessarily equivalent. For a reliable implementation, save the training scaler parameters and apply the same transformation in firmware.

## Current implementation scope and limitations

- The checked-in firmware implements sensor reading, model inference, voltage-threshold checking, relay output, buzzer output, and serial logging.
- GSM/SIM800L SMS notification is described in some earlier project notes, but GSM code is not present in the checked-in firmware. SMS alerts should be considered a future integration unless implemented separately.
- The training dataset (`current_reading.csv`) and generated `.tflite` file are not included.
- The firmware's sudden-drop condition uses a change in reconstruction error; validate whether this logic corresponds to the intended fault signature.
- The current code halts after a detected event and requires a reset. A production design would need robust fault-state handling, diagnostics, watchdog strategy, and fail-safe validation.

## Future improvements

- Collect and document a larger dataset containing normal and fault-condition samples.
- Validate detection performance using a separate test set and report precision, recall, false-alarm rate, and detection latency.
- Align training and on-device preprocessing and calibrate the anomaly threshold.
- Add voltage/current signal conditioning and safer isolated measurement hardware.
- Integrate and test GSM/SMS or another remote notification method.
- Add event logging, watchdog handling, and a clearly defined fail-safe state.
- Validate the complete system against applicable electrical safety standards before any real-world deployment.

## Contributing

Suggestions, bug reports, and improvements are welcome. Please open an issue to discuss substantial changes before submitting a pull request.

## License

No license is currently specified. Add a license file if you want to define how others may use, modify, and distribute this project.
