# Snake game on the arduino uno R3

**Author:** Tomas Baublys  
**Course:** Introduction to robotics  

---

## Project Overview

This project implements a **Console like snake game** for the arduino uno R3 using a 160x128 pixel TFT screen and a joystick:

- **Joystick** - used to control the snakes movement.
- **TFT display** – monitors smoke levels to detect potential fire hazards.
- **PIR motion sensor** – detects motion in the environment.
- **Piezo buzzer** – provides audible alarms.
- **NeoPixel LED ring** – provides visual feedback (alarm and status).

The system continuously monitors the sensors and responds as follows:

1. **Smoke detected**:  
   - Red light on the NeoPixel ring.  
   - Piezo buzzer sounds an alarm.

2. **Low light & motion detected**:  
   - White-yellow light on the NeoPixel ring is turned on.

3. **No smoke or motion**:  
   - All LEDs are turned off.

---

## Hardware Setup

- You can view or download the component list here: [partslist.csv](partslist.csv)

## Circuit Diagram
![Circuit photo](real_wirring.png)

---

## Software

- Written in **Arduino C++**.
- Uses the **Adafruit NeoPixel library** to control the LED ring.  

## Parameters Explanation

- `PHOTO_RES_THRESHOLD_VAL` – (**arbitrary**) threshold value for ambient light detection.
- `SMOKE_DET_THRESHOLD_VAL` – (**arbitrary**) threshold value for smoke detection.
- `NUM_OF_LEDS` – number of LEDs in NeoPixel ring.
- `LED_ALARM_*` – RGB color for alarm state.
- `LED_ON_*` – RGB color for normal operation.
- `DELAY` – loop delay in milliseconds.
- `*_PIN` - component pin numbers.

### Key Functions

- `setup()` – Initializes serial communication, sensor pins, and NeoPixel ring.  
- `loop()` – Reads sensor values and updates LEDs and buzzer based on conditions.

---

## Usage
You can either create the provided wiring diagram or explore the live Tinkercad project.

1. Connect the hardware according to the pinout table.
   For more details, refer to the [Wiring Diagram (PDF)](wirring.pdf)  
   or view the project on [Tinkercad](https://www.tinkercad.com/things/inF08CLtjdp-corridor-automatic-lamp-with-smoke-detector?sharecode=2ZnDAHdU0eUikj-b9LfzPR57jI1qgf9tkYdIz54yzns).
2. Install the [Adafruit NeoPixel library](https://github.com/adafruit/Adafruit_NeoPixel) in Arduino IDE.
3. Upload the code to your Arduino board.
4. Observe the LED ring and buzzer respond to smoke, motion, and light conditions.

---

## Notes

- Adjust `PHOTO_RES_THRESHOLD_VAL` and `SMOKE_DET_THRESHOLD_VAL` to match your sensor calibration.
- The buzzer uses a 523 Hz tone for 500 ms when smoke is detected.
- The system can be extended with additional sensors or network alerts for more advanced safety systems.
