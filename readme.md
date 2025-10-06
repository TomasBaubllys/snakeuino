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

### Interrupts 

## Interrupts
The game uses two non timer interrupts on both of the supported pins (D2 and D3):
1. The big yellow button thats connected to D3 triggers the interrupt to call the function void color_button_pressed() which sets the flag to change the games theme color.

2. The joysticks "press down" button connected to D2 pin triggers the interrupt to call the function void button_pressed() which sets the flag to either pause/continue the game or reset the game if the game was lost.

## Timer interrupt
The timer interrupt is used to update and render the game, this allows the main loop to be non-blocking and allow for multiple user inputs between frames, which makes the game feel more responsive. The timer interrupt is set as shown below:

Since, Arduino Uno R3 CPU operates on 16Mhz frequency, we can use that to limit the fps, so we can use a 64 prescaler, set the OCR1A register to 17500, and set the timer1 to CTC mode.

The math behind it is something like this:
We want to get arround 14fps, so thats a frame every 1/14 = 0.07 seconds or 70ms. Since we use a 64 prescaler, that means that the timer now runs at 16000000 / 64 = 250000 ticks/second. So if we set the OCR1A value to 17500, that means that the timer interrupt will happen every 17500 / 250000 = 0.07 seconds. Of course we do not have to guess the OCR1A value, we can simply solve the equation: OCR1A_value / (16000000 / prescaler) = 1/(desired_fps)

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


