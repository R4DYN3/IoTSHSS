# Arduino Sensor Monitoring and Alarm System

## Arduino uno - smart house code: 
### Description
This Arduino project monitors various sensors (gas, light, water, soil) and triggers alarms based on certain thresholds.
The system processes sensor data, sets warning flags, and outputs alarms without blocking the rest of the system’s operation.

### Features
- **Gas Sensor**: if the gas warning bit is set, the fan on the house will automatically turn on and [TODO]: an alarm will play
- **Light Sensor**: Activates an light when light levels drop below a threshold.
- **Water Sensor**: Serial Streams warning data over to to Arduino uno wifi to show a high water level.
- **Soil Sensor**: erial Streams warning data over to to Arduino uno wifi to show Hydropenia.

[todo]:
- **Non-blocking alarm**: The alarm is triggered without interrupting the rest of the program using `millis()` for non-blocking delays.

### Hardware Requirements
- **Arduino Board**: Any Arduino compatible board (e.g., Arduino Uno).
- **Gas Sensor** (e.g., MQ series)
- **Light Sensor** (e.g., LDR)
- **Water Sensor** (e.g., capacitive moisture sensor)
- **Soil Sensor** (e.g., resistive moisture sensor)
- **Buzzer or Speaker** (connected to pin 3)
- **Infrared LED** (connected to pin 13, if applicable)

### Software Requirements
- **Arduino IDE**: The development environment for uploading the code to the Arduino board.
- code: SMARTHOUSE_ARDUINO_UNO_IOT_2025.ino

## Pin Configuration
- **Pin 3**: Buzzer/Speaker for the alarm
- **Pin 13**: Infrared LED (for low light alarm)
- **Analog Pin A0**: Gas sensor input
- **Analog Pin A1**: Light sensor input
- **Analog Pin A2**: Soil sensor input
- **Analog Pin A3**: Water sensor input

### Setup
### 1. Wiring
- **Gas Sensor**: Connect the gas sensor to `A0` (Analog Pin 0).
- **Light Sensor**: Connect the light sensor to `A1` (Analog Pin 1).
- **Soil Sensor**: Connect the soil sensor to `A2` (Analog Pin 2).
- **Water Sensor**: Connect the water sensor to `A3` (Analog Pin 3).
- **Infrared LED**: Connect the infrared LED to pin `13`.
- **Buzzer/Speaker**: Connect the buzzer or speaker to pin `3`.

## Code Explanation
This code reads sensors data from sensors across the house, and based on the data will set a warning bit field to indicate any warnings.
The data is then streamed over serial to an Arduino uno wifi board.
Based on the warning bit field, the house will run certain mitigations to self preserve itself:
1. If the gas level is HIGH the fan will turn on
2. If the light level is low a light will turn on 
