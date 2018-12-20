# Final Project: Keypad lock

## Description
In this project, a keypad lock was created on an ATTiny88. At ATTiny88 was selected because it has more than enough program space, ram, and enough I/O for a very low price.
This keypad lock has several unique functions:
* 4x3 keypad input
* separate display module
* 4x7 segment display for output multiplexed through shift registers
* speaker for user feedback (forrect/incorrect codes), key presses, and alarm.
* battery backup and charging
* 3.3v - 4.2v @ 500 mA output for correct passwords

The code utilizes a polling method of reading user input at 3.9 KHz, utilizing sleep modes inbetween. There are two separate PCBs: The control board and the display board. The 7-segment display is multiplexed through two 74HC595 shift registers. This is done so the display board can use as few wires as possible for simplicity. There are 5 wires going from the control board to the display board: Vcc, Gnd, Clock, Data, and Latch. 

This keypad has an 18650 battery as a power source, as well as a TP4056 as battery regulation. 

## Pinout
There are several required connections, these are listed below:

* PD3 - Output lock
* PD4 - LEDG
* VCC - VCC
* GND - GND
* PB6 - 6 
* PB7 - 7 
* PD5 - LEDR
* PD6 - Siren
* PD7 - LATCH
* PB0 - 0 
* PC5 - SCK	
* PC4 - DAT	
* PC3 - # 
* PC2 - * 
* PC1 - 9 
* PC0 - 8 
* GND - GND
* Aref - VCC
* AVCC - VCC
* PB5 - 5
* PB4 - 4
* PB3 - 3
* PB2 - 2
* PB1 - 1

It should be noted that:
* PD6 (Siren) is a low-side switch that drives an 8-ohm speaker.
* LEDR drives the red LED on the keypad
* LEDG drives the green LED on the keypad
* SCK is the clock for the display board
* DAT is the data line for the display board
* LATCH is the latch pin for the display board
## Display board
The display board uses 2 74HC595 shift registers connected in series for a total of 16 outputs. 11 of these outputs are used for the display. The decimal point is not used. The OE pin is always held low, and the reset pin is always held high. The first 4 putputs drive the current number being multiplexed, the next 4 are unused, and the next 7 drive the segments. The last bit is also unused.

## Operation
When initially powered on or after a correct password is entered, the user has 3 password attempts. When a key is pressed, the button is debounced with a short delay, and a tone() function is called. After 4 key presses, the code is compared with the correct code. If the code is incorrect, the number of password attempts is decremented, two low-pitch tones are emitted, the keypad displays "nope", and "A= n" where n is the number of password attempts left . If the correct password is entered, a low pitched tone and a high pitched tone are emitted, and the keypad says "ello", and a 3.3v-4.2v signal is emitted. If 4 incorrect consecutive passwords are entered, a siren is emitted, which can only be stopped by entering the correct password.




