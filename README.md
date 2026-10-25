# ESP32 Wi-Fi Security Lab

An educational wireless lab project for the ESP32 development board.

## Features

- ESP32-hosted local web dashboard
- Nearby Wi-Fi network scanning
- Signal strength and security status display
- Device status monitoring
- Authorized wireless network testing

## Hardware

- ESP32 development board
- USB-to-serial interface
- Android phone with Termux for project preparation
- Windows computer with Arduino IDE for compilation and flashing

## Software Requirements

- Arduino IDE
- ESP32 board package by Espressif Systems
- ESP32 Arduino libraries: WiFi and WebServer

## Setup

1. Open `firmware/esp32_wifi_lab/esp32_wifi_lab.ino` in Arduino IDE.
2. Install the ESP32 board package.
3. Select the appropriate board and serial port.
4. Compile and flash the firmware.
5. Connect to the ESP32 lab access point.
6. Open `http://192.168.4.1` in a browser.

## Security Notes

Use this project only on devices and wireless networks you are
authorized to test. The initial lab access-point password is for
setup purposes and should be changed before deployment.

## Project Status

Educational prototype. Hardware compatibility and functionality
must be verified on the actual board.
