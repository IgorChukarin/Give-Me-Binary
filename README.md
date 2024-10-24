# Give Me Binary

## About the Game
Give Me Binary is a game designed to challenge player's understanding of binary numbers.  
The game generates a random decimal number between 1 and 15, and the player’s goal is to represent that number using binary, by toggling a set of buttons corresponding to each bit.  
The player’s success is visually indicated by LEDs, which light up when the corresponding bit is set to 1.
The player must toggle 4 buttons, each representing a bit (0 or 1) in the binary number. The LED lights up when the corresponding bit is 1.

## Setup
There are different ways to run the game:

### Wokwi Simulator:
- Copy the content of `schema.txt` into a new `diagram.json` file in your Wokwi project.
- Copy the content of `game.ino` into the `sketch.ino` file in Wokwi.
- Once everything is copied, run the project from the Wokwi platform.

### Physical Circuit:
- Build your own circuit following the diagram (see the picture below). Ensure the buttons and LEDs are correctly connected.
- Upload the `game.ino` code to your Arduino using any Arduino-compatible IDE.
![schema](https://github.com/user-attachments/assets/91a0a092-70a2-438e-bb69-d514eb459502)
*Circuit diagram for setting up the buttons and LEDs for the game.*

## How to Play
1. Choose the difficulty by turning the potentiometer. The game offers four difficulty levels: Easy, Medium, Hard, and Expert, which affect the time limit for your response.
2. Press the leftmost button to start the game.
3. Input your binary answer by pressing the buttons. Each button corresponds to a specific bit (0 or 1). For example:
   - Button 1 (leftmost) represents the least significant bit (LSB).
   - Button 4 (rightmost) represents the most significant bit (MSB).
4. The LEDs will indicate if you set a bit to 1. Correct answers will be acknowledged, and you can move to the next round. Incorrect or slow answers will end the game.


