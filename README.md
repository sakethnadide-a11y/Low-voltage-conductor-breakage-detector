# low-voltage-conductor-breakage-detector

# Low Voltage Conductor Breakage Detector using IoT and Edge AI

## Overview

This project presents a cost-effective and intelligent solution for detecting breakages in Low Voltage AC Distribution Over Head conductors. It leverages a combination of IoT hardware and an on-device AI model to provide real-time anomaly detection, ensuring a swift and automated response to potential faults.

The system continuously monitors current and voltage, using a lightweight machine learning model deployed on an ESP32 microcontroller to identify deviations from normal operating conditions. Upon detecting a fault, it automatically isolates the power supply, activates a local buzzer alarm, and sends an SMS alert to concerned authorities.

## Key Features

- **Real-time Monitoring:** Continuous sensing of current and voltage parameters.
- **Edge AI:** Anomaly detection is performed on the device (ESP32) using a pre-trained TensorFlow Lite model, eliminating the need for constant cloud connectivity.
- **Automated Response:** The system triggers a relay to cut power and activates an audible alarm (buzzer).
- **GSM/SMS Alerting:** Utilizes a **SIM800L** module to send an immediate text message to alert personnel, ensuring a rapid response.
- **Cost-Effective:** Utilizes widely available and low-cost components.

## Technologies Used

- **Hardware:**
    - **Microcontroller:** ESP32
    - **Sensors:** ACS712 (Current Sensor), ZMPT101B (Voltage Sensor)
    - **Actuators:** Relay Module, Buzzer
    - **Connectivity:** **GSM (SIM800L)**
- **Software:**
    - **IDE:** Arduino IDE
    - **AI/ML:** TensorFlow, TensorFlow Lite Micro
    - **Libraries:** TinGSM (or SoftwareSerial for SMS)

## Methodology

### 1. Data Sensing & Collection
Current and voltage are measured continuously from the overhead conductor lines using the ACS712 and ZMPT101B sensors.

### 2. AI Model Training (Anomaly Detection)
An Autoencoder model was trained on a dataset of "normal" current readings. This model learns to compress and reconstruct the data. The reconstruction error serves as the primary metric for anomaly detection:
- A **low** reconstruction error indicates a **normal** state.
- A **high** reconstruction error (above a predefined threshold) signals an **anomaly** or a potential fault.

The training process was conducted in Python using `pandas`, `scikit-learn`, and `tensorflow`. The model was then converted to a highly optimized TensorFlow Lite (`.tflite`) format for deployment on the ESP32.

### 3. On-Device Inference & Alerting
The converted `.tflite` model, compiled into a C-extension (`.h` file), is integrated into the ESP32's firmware. The microcontroller:
1.  Takes sensor readings.
2.  Feeds the scaled data into the AI model.
3.  Calculates the reconstruction error.
4.  Compares the error against the pre-calibrated threshold.
5.  If the error exceeds the threshold or the voltage drops below a safe level, a fault is declared.
6.  The relay is triggered to cut the power, the buzzer is activated, and an **SMS message is sent via the SIM800L module** to notify the relevant personnel.

## Code and Implementation

### AI Model Training (Python)
The `train_model.py` script contains the full pipeline for training the Autoencoder and converting it to a TensorFlow Lite model.

### Embedded C++ Code (Arduino)
The `main.ino` file contains the complete firmware for the ESP32, including the sensor reading logic, AI model inference, and the fault-handling routines with the integrated SIM800L code. The `model_data.h` file contains the binary data of the converted TFLite model.

## Installation and Setup

1.  **Hardware Assembly:** Connect the components, including the SIM800L module, as per the circuit diagram.
2.  **Arduino IDE Setup:** Install the ESP32 board manager and required libraries.
3.  **Model Conversion:** Run the Python script to generate `model_data.h`.
4.  **Upload Code:** Upload the `main.ino` and `model_data.h` files to your ESP32.

✍️ Author: Rohan Pydipalli
