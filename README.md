# PPG-Based Low-Cost Cuffless Blood Pressure Monitoring System

This repository contains the ESP8266 firmware and project notes for a low-cost cuffless continuous blood pressure monitoring prototype based on Blood Pressure Trending (BPT). 
The prototype uses a MAX32664D biometric sensor hub with a MAX30102 PPG sensor to estimate systolic and diastolic blood pressure trends after calibration. The full research design also includes heart rate, SpO2, mean arterial pressure (MAP), and skin temperature monitoring.

> Research prototype only. This project is not a certified medical device and should not be used for clinical diagnosis or treatment decisions.

## Project Highlights

- Cuffless BPT-based blood pressure trend estimation
- PPG-based heart rate and SpO2 acquisition
- ESP8266-based wireless monitoring
- Browser dashboard served directly from the device
- Portable, low-cost hardware design, estimated around 50 USD in the paper
- Validation described for 20 participants: 10 young adults and 10 elderly participants

## Research Summary

The paper proposes a non-invasive continuous monitoring system that reduces the discomfort of cuff-based blood pressure devices. Instead of repeatedly inflating a cuff, the device uses PPG morphology through the MAX32664D sensor hub and requires reference calibration from a conventional cuff device.

Reported accuracy from the paper:

| Group | SYS Accuracy | DIA Accuracy | MAP Accuracy | HR Accuracy | SpO2 Accuracy |
| --- | ---: | ---: | ---: | ---: | ---: |
| Age 20-28 | 96.53% | 94.99% | 96.53% | 97.52% | 99.17% |
| Age 45-64 | 95.29% | 94.91% | 96.01% | 97.81% | 99.30% |

## Hardware

Main components described in the paper:

| Component | Quantity | Purpose |
| --- | ---: | --- |
| ESP8266 NodeMCU | 1 | Microcontroller and Wi-Fi web server |
| MAX32664D + MAX30102 module | 1 | BPT, heart rate, and SpO2 sensing |
| MAX30205 temperature sensor | 1 | Skin temperature measurement |
| 1.8-inch RGB TFT LCD | 1 | Local display in the full prototype |
| Li-ion battery | 1 | Portable power |
| TP4056 charger module | 1 | Battery charging and protection |
| Switch, LEDs, casing, wiring | As needed | Power control, status, enclosure |

The firmware currently included in this repository implements the ESP8266, MAX32664/MAX30102, serial output, and browser dashboard path. The paper also describes TFT LCD and MAX30205 temperature features that can be added as firmware extensions.

## Wiring Used By Firmware

| ESP8266 Pin | Connected To |
| --- | --- |
| D2 | I2C SDA |
| D1 | I2C SCL |
| D6 | MAX32664 RESET |
| D0 | MAX32664 MFIO |

## Firmware Files

- `Bpt_base_bp.cpp` - ESP8266 firmware for MAX32664 BPT readings and a web dashboard
- `secrets.h.example` - Wi-Fi credential template
- `platformio.ini` - PlatformIO build target for NodeMCU ESP8266
- `becithcon_2025_ieee_submission.pdf` - project paper with full methodology, validation, and cost analysis

## Setup

1. Install PlatformIO or configure the Arduino IDE for ESP8266.
2. Install the ESP8266 board support package.
3. Install the MAX32664 Arduino library that provides `max32664.h`.
4. Copy `secrets.h.example` to `secrets.h`.
5. Edit `secrets.h` with your Wi-Fi name and password.
6. Build and upload the firmware to the ESP8266.

Example using PlatformIO:

```powershell
Copy-Item secrets.h.example secrets.h
pio run
pio run --target upload
```

After boot, open the ESP8266 serial monitor at `115200` baud. The device prints the assigned IP address. Visit that IP address in a browser to view the live vitals dashboard.

## Calibration

The BPT method requires baseline cuff measurements before estimation. The included firmware uses example calibration values:

```cpp
calibValSys = {120, 122, 125}
calibValDia = {80, 81, 82}
```

Replace these with user-specific calibration readings from a validated cuff-based device. The paper describes three reference blood pressure measurements for calibration.

## Web Dashboard

The ESP8266 serves a simple page at `/` showing:

- Systolic pressure in mmHg
- Diastolic pressure in mmHg
- Heart rate in bpm
- SpO2 in percent

The page refreshes manually using the on-page refresh button.

## Cost Analysis From Paper

The paper estimates the prototype cost at approximately `6060 BDT`, or about `49.64 USD`, using the listed hardware components.

## Citation

If you use this project, cite the included paper:

`Development of a PPG-based Low-cost Cuffless Continuous Blood Pressure Monitoring with Vital Sign Detection System`

