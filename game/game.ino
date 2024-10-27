#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <avr/sleep.h>
#include <EnableInterrupt.h>

#define DEBOUNCE_TIME 200
#define LED_BLINK_INTERVAL 500
#define WAKE_UP_TIME 10000

LiquidCrystal_I2C lcd(0x27, 16, 4);

const int BUTTONS[] = {2, 3, 4, 5};
const int LEDS[] = {8, 9, 10, 11};
const int REDLED = 13;

int highScore = 0;
int currentScore = 0;
int difficultyFactor = 0;
int timeLimit = 15;

bool ledStates[] = {LOW, LOW, LOW, LOW};
bool redLedState = LOW;
bool isIntroDisplayed = false;
bool isDifficultySelected = false;
bool isGameStarted = false;
bool isGameOver = false;
bool shouldDisplayNumber = true;

unsigned long previousButtonPressTime[4];
unsigned long previousRedLedBlinkTime = 0;
unsigned long lastActivityTime;
unsigned long timeRoundStart = 0;

int targetNumber = 0;
int currentPotValue = 1;
int currentDelta = 0;

void wakeUp(){}

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  randomSeed(analogRead(0));
  for (int i = 0; i < 4; i++) {
    pinMode(BUTTONS[i], INPUT);
    pinMode(LEDS[i], OUTPUT);
    previousButtonPressTime[i] = 0;
    enableInterrupt(BUTTONS[i], wakeUp, RISING);
  }
  pinMode(REDLED, OUTPUT);
  lastActivityTime = millis();
}

void loop() {
  if (!isIntroDisplayed) displayIntro();
  else if (!isDifficultySelected) selectDifficulty();
  else if (!isGameStarted) startGame();
  else if (!isGameOver) playGame();
  else finishGame();
}

void displayIntro() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Welcome to GMB!");
  lcd.setCursor(0, 1);
  lcd.print("Press B1 to Start");
  lcd.setCursor(0, 3);
  lcd.print("High Score: ");
  lcd.print(highScore);
  isIntroDisplayed = true;
}

void selectDifficulty() {
  checkForIdleTimeout();
  updateDifficultyLevel();
  checkButtonPressForSelection();
  blinkRedLed();
}

void checkForIdleTimeout() {
  if (millis() - lastActivityTime >= WAKE_UP_TIME) goToSleepMode();
}

void updateDifficultyLevel() {
  int newPotValue = analogRead(A1);
  if(newPotValue != currentPotValue){
    currentPotValue = newPotValue;
    lastActivityTime = millis();
    lcd.setCursor(0,2);

    if(currentPotValue <= 256 && difficultyFactor != 1){
      lcd.print("          ");
      lcd.setCursor(0,2);
      lcd.print(" :) EASY");
      difficultyFactor = 1;
    } else if(currentPotValue > 256 && currentPotValue <= 512 && difficultyFactor != 2){
      lcd.print("          ");
      lcd.setCursor(0,2);
      lcd.print(" :| MEDIUM");
      difficultyFactor = 2;
    } else if(currentPotValue > 512 && currentPotValue <= 768 && difficultyFactor != 3){
      lcd.print("          ");
      lcd.setCursor(0,2);
      lcd.print(">:) HARD");
      difficultyFactor = 3;
    }else if(currentPotValue > 768 && difficultyFactor != 4){
      lcd.print("          ");
      lcd.setCursor(0,2);
      lcd.print(">:D EXPERT");
      difficultyFactor = 4;
    }
  }
}

void checkButtonPressForSelection() {
    if (digitalRead(BUTTONS[3])) {
    isDifficultySelected = true;
    redLedState = LOW;
    digitalWrite(REDLED, redLedState);
    lastActivityTime = millis();
  }
}

void goToSleepMode() {
  lcd.clear();
  lcd.print("Sleeping...");
  Serial.flush();
  digitalWrite(REDLED, LOW);
  delay(1000);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_mode();
  sleep_disable();
  lastActivityTime = millis();
  displayIntro();
  delay(1000);
}

void blinkRedLed() {
  if (millis() - previousRedLedBlinkTime >= LED_BLINK_INTERVAL) {
    redLedState = !redLedState;
    digitalWrite(REDLED, redLedState);
    previousRedLedBlinkTime = millis();
  }
}

void startGame() {
  isGameStarted = true;
  lcd.clear();
  lcd.print("Get ready!");
  delay(1000);
}

void playGame() {
  if (shouldDisplayNumber) {
    targetNumber = random(1, 16);
    lcd.clear();
    lcd.print(targetNumber);
    shouldDisplayNumber = false;
    timeRoundStart = millis();
  } 
  else {
    handleRoundTimer();
    handleButtonPresses();
    if (targetNumber == convertButtonsStatesToDecimal()) {
      currentScore++;
      displayScore();
      resetRound();
      
      timeLimit = timeLimit - difficultyFactor;
      if (timeLimit < 1) timeLimit = 1;
      currentDelta = 0;
    }
  }
}

void displayScore() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Correct!");
  lcd.setCursor(0, 1);
  lcd.print("Score: ");
  lcd.print(currentScore);
  delay(1000);
}

void resetRound() {
  shouldDisplayNumber = true;
  turnOffAllLeds();
}

void handleButtonPresses() {
  for (int i = 0; i < 4; i++) {
    int buttonState = digitalRead(BUTTONS[i]);
    long currentTime = millis();
    if (buttonState == HIGH && currentTime - previousButtonPressTime[i] > DEBOUNCE_TIME) {
      ledStates[i] = !ledStates[i];
      digitalWrite(LEDS[i], ledStates[i]);
      previousButtonPressTime[i] = currentTime;
    }
  }
}

int convertButtonsStatesToDecimal() {
  int decimal = 0;
  for (int i = 0; i < 4; i++) {
    decimal += ledStates[i] << (i);
  }
  return decimal;
}

void turnOffAllLeds() {
  for (int i = 0; i < 4; i++) {
    ledStates[i] = LOW;
    digitalWrite(LEDS[i], LOW);
  }
}

void finishGame() {
  turnOffAllLeds();
  lcd.clear();
  digitalWrite(REDLED, HIGH);
  delay(1000);
  digitalWrite(REDLED, LOW);
  lcd.print("Game over!");
  lcd.setCursor(0, 1);
  lcd.print("Final Score: ");
  lcd.print(currentScore);
  if (currentScore > highScore) highScore = currentScore;
  delay(10000);
  resetGame();
}

void resetGame() {
  isIntroDisplayed = false;
  isDifficultySelected = false;
  isGameStarted = false;
  isGameOver = false;
  currentScore = 0;
  difficultyFactor = 0;
  currentPotValue = 0;
  shouldDisplayNumber = true;
  timeLimit = 15;
  turnOffAllLeds();
  lastActivityTime = millis();
  
  timeRoundStart = 0;
  currentDelta = 0;
}

void handleRoundTimer(){
int timeElapsed = (millis() - timeRoundStart) / 1000 ;
 if(currentDelta != timeElapsed){
    lcd.setCursor(8,0);
    lcd.print("Time:");
    lcd.setCursor(13,0);
    lcd.print("  ");
    lcd.setCursor(13,0);
    lcd.print(timeLimit - timeElapsed + 1);
  }
  currentDelta = timeElapsed;
  if(currentDelta > timeLimit) isGameOver = true;
}
