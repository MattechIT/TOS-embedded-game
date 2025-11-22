# TOS-embedded-game
An ATMEGA328P embedded system implementing a game called Turn on the Sequence! (TOS).

**Assignment #01 - Embedded Systems**

This project realizes an embedded game called **"Turn on the Sequence!"** (TOS) on the Arduino platform. It is a memory-based game where the player must reproduce a sequence of numbers displayed on an LCD by pressing the corresponding buttons and lighting up LEDs.

## 📋 Project Overview

* **Architecture:** Super-Loop with Interrupts
* **Programming Style:** Procedural (C-style).
* **Key Features:**
    * Interrupt-based input handling with non-blocking debounce.
    * Deep Sleep mode for power saving.
    * Non-blocking LED fading effects.
    * Adaptive difficulty system.

## 📚 Libraries Used

* LiquidCrystal_I2C - For LCD control.
* EnableInterrupt - For handling pin change interrupts on buttons.
* avr/sleep.h & avr/power.h - For power management.

## 🛠️ Hardware Requirements

* 1x Arduino Uno
* 1x LCD Display (16x2) with I2C module
* 4x Green LEDs (Game sequence)
* 1x Red LED (Standby/Game Over)
* 4x Tactile Buttons
* 1x Potentiometer (10kΩ)
* Resistors (220Ω for LEDs, 10kΩ for Buttons)
* Breadboard and Jumper Wires

## 🔌 Wiring Map

| Component | Arduino Pin | Note |
| :--- | :--- | :--- |
| **Button 1 (L1/Start)** | Digital 5 | Input (Interrupt) |
| **Button 2 (L2)** | Digital 4 | Input (Interrupt) |
| **Button 3 (L3)** | Digital 3 | Input (Interrupt) |
| **Button 4 (L4)** | Digital 2 | Input (Interrupt) |
| **Green LED 1** | Digital 11 | Output |
| **Green LED 2** | Digital 10 | Output |
| **Green LED 3** | Digital 9 | Output |
| **Green LED 4** | Digital 8 | Output |
| **Red LED** | Digital 6 | Supports PWM |
| **Potentiometer** | Analog A3 | Difficulty Selector |
| **LCD SDA** | Analog A4 | I2C Data |
| **LCD SCL** | Analog A5 | I2C Clock |
| **Random Seed** | Analog A0 | Left unconnected |

## 🎮 How to Play

1.  **Setup:** Connect the Arduino to power. The Red LED will start pulsing, you can now click `Button 1` to enter the setup, otherwise the system will go in deep sleep mode after 10 seconds.
2.  **Difficulty:** Adjust the Potentiometer to select a difficulty level (Easy, Medium, Hard, Extreme).
3.  **Start:** Press `Button 1` to confirm and start the game.
4.  **Play:**
    * A sequence of 4 numbers (e.g., `4 1 3 2`) appears on the LCD.
    * Press the corresponding buttons (B4 -> B1 -> B3 -> B2) within the time limit.
5.  **Progression:** If correct, the score increases, and the time limit decreases for the next round.
6.  **Game Over:** If time runs out or the wrong button is pressed, the Red LED turns on. The final score is shown before resetting.

## 📂 Project Structure

```text
assignment-01/
├── src/
│   └──TurnOnTheSequence/
│       └── TurnOnTheSequence.ino   # Main source code
├── doc/
│   ├── scheme.png                  # Breadboard layout
│   ├── schematic.pdf               # Schematic diagram
│   └── demo.md                     # Demonstration video link
└── README.md                       # This documentation