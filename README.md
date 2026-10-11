# 👓 Smart Wearable Design and Prototyping – 2025/2026

🎓 Project for the *Smart Wearable Design and Prototyping* course (Politecnico di Milano, A.Y. 2025/26).

## 💡 About the project

This project is a low-power wearable device for monitoring physiological and motion signals. The device continuously measures:

- ❤️ **Heart activity** through photoplethysmography (PPG), from which the heart rate is computed on the device
- 🏃 **Movement** through an accelerometer and a gyroscope
- 🌡️ **Body temperature** through a dedicated temperature sensor

📡 The data is processed by an ultra-low-power microcontroller and sent over Bluetooth Low Energy to a smartphone app, which shows the signals in real time as live charts. The device can also store data in on-board flash memory, and it is designed to save energy 🔋: the microcontroller sleeps between acquisitions and the Bluetooth module has low-power modes, so that the battery lasts longer.

The system has two parts:

1. 🔧 **Wearable firmware** – sensor acquisition, heart-rate algorithm, power management, data storage and Bluetooth communication
2. 📱 **Mobile app** – connection to the device, reception of the data packets and real-time visualization

## 📁 Repository structure

| Folder | Content |
|---|---|
| 🔧 [SWDP_BIO_A6/](SWDP_BIO_A6) | Firmware of the wearable device |
| 📱 [swdp_bio_app/](swdp_bio_app) | Flutter mobile app that connects to the device and plots the data |

## 🔧 Firmware

Written in C for an STM32 microcontroller. It includes drivers for the PPG sensor, the IMU, the temperature sensor, the Bluetooth module, the flash memory and the status LEDs. The heart rate is estimated by filtering the PPG signal and analysing it over a sliding window.

## 📱 Mobile app

A Flutter (Android) app that scans for the device, connects to it and displays the incoming data. It also has a mock-data mode to test the interface without the hardware.

### 📦 BLE data packets

The device sends one packet type per measurement, identified by its first byte:

| Byte | Char | Data |
|---|---|---|
| 65 | `A` | 📈 Acceleration (X, Y, Z) |
| 71 | `G` | 🔄 Gyroscope (X, Y, Z) |
| 80 | `P` | 🔴 Raw PPG |
| 70 | `F` | 🟢 Filtered PPG |
| 72 | `H` | ❤️ Heart rate (BPM) |
| 84 | `T` | 🌡️ Temperature |
