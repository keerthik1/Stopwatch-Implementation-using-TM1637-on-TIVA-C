# Stopwatch Implementation Using TM1637 on TIVA C LaunchPad

## Overview
This project implements a stopwatch using the TM1637 4-digit 7-segment display driver on the TIVA C LaunchPad (EK-TM4C123GXL, TM4C123GH6PM microcontroller). The stopwatch counts time in MM:SS format (minutes and seconds), displayed on the TM1637-driven 7-segment display, with start, stop, and reset functionality controlled via Keypad inputs. The project demonstrates embedded systems skills, including timer interrupts, GPIO handling, and a 2-wire communication protocol for the TM1637.

### Features
- Displays time in MM:SS format (e.g., 12:34 for 12 minutes, 34 seconds).
- Supports start, stop, and reset functions using Keypad interfaced to the TIVA C LaunchPad.
- Implements a software-based 2-wire protocol (CLK and DIO) to communicate with the TM1637.
- Uses the TIVA C’s SysTick timer for precise 1-second increments.
- Configures the system clock to 16 MHz for accurate timing.

## Hardware Requirements
- **TIVA C LaunchPad**: EK-TM4C123GXL with TM4C123GH6PM microcontroller.
- **TM1637 4-Digit 7-Segment Display Module**: Pre-assembled module with a 4-digit display.
- **Keypad**: 3 buttons(A,B,C) for start,stop and reset.
- **Jumper Wires**: For connecting the TM1637 and buttons to the LaunchPad.
- **Connections**:
  - TM1637 CLK → TIVA GPIO (e.g., PB0)
  - TM1637 DIO → TIVA GPIO (e.g., PB1)
  - TM1637 VCC → 3.3V (or 5V, depending on module)
  - TM1637 GND → GND
  - Start → A button on Keypad
  - Stop → B Button on keypad
  - Reset → C Button on keypad

## Software Requirements
- **Code Composer Studio (CCS)**: Version 12.4.0 or later.
- **TivaWare**: Version 2.2.0.295 or later.
- **Compiler**: TI ARM Compiler (included with CCS).

## Setup Instructions
1. **Hardware Setup**:
   - Connect the TM1637 module to the TIVA C LaunchPad as per the pin assignments above.
   - Ensure the Keypad is connected as mentioned above.
   - Power the LaunchPad via USB.
2. **Software Setup**:
   - Clone or download this repository.
