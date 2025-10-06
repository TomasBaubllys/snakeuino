# Snake game on the Arduino Uno R3

**Author:** Tomas Baublys  
**Course:** Introduction to robotics  

---

## Project Overview

This project implements a **Console like snake game** for the Arduino Uno R3 using a 160x128 pixel TFT screen and a joystick:

- **Joystick** - used to control the snakes movement.
- **TFT display** – monitors smoke levels to detect potential fire hazards.
- **Button** - used for switching between different colour modes

The system continously reads the user input, however the game only updates when the timer interrupt sets the tick variable to true, this enables a smoother controller experience. The joystick button is also tied to an interrupt which can either pause/unpause the game if its being played and reset the game once it is over. The extra provided button is (you guessed it) also tied to an interrupt and can change between PCB and light colour modes.

The game also has a highscore system, that uses the arduino`s EEPROM to save it between sessions.

---

## Hardware Setup

- You can view or download the component list here: [partslist.csv](partslist.csv)

## Circuit Diagram
![Circuit photo](real_wirring.jpg)

## Wirring Diagram
![Wirring diagram](wirring.pdf)

---

## Software

- Written in **Arduino C++**.
- **SPI** protocol is used for the display

---

### 

---

### EEPROM layout
- 0x00 - 1 byte of a magic number that is used for checking whether the data has been corrupted
- 0x01 - 1 byte for the current highscore

---

## Usage
You will have to create the provided wiring diagram.

1. Connect the hardware according to the pinout table.
   For more details, refer to the [Wiring Diagram](wirring.jpg)  
2. Upload the code (snakeuino.ino) to your Arduino board.
3. Have fun!.

--- 

# Notes
- If the game feels too fast you can adjust the value in the OCR1A register to change the frame rate


