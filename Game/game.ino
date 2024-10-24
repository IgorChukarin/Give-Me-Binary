#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <avr/sleep.h>
#include <EnableInterrupt.h>

#define DEBOUNCE_TIME 200
#define LED_BLINK_INTERVAL 500
#define WAKE_UP_TIME 10000

LiquidCrystal_I2C lcd(0x27, 16, 4);

const int REDLED = 13;
bool redLedState = LOW;
long previousRedLedBlinkTime = 0;

const int BUTTONS[] = {2, 3, 4, 5};
const int LEDS[] = {8, 9, 10, 11};
bool ledStates[] = {LOW, LOW, LOW, LOW};
long previousButtonPressTime[4];

int currentPotValue = 1;
int difficulty = 0;

bool isIntroDisplayed = false;
bool isDifficultySelected = false;
bool isGameStarted = false;
bool isGameOver = false;

bool shouldDisplayNumber = true;
int targetNumber = 0;

unsigned long timeRoundStart = 0;
int deltaT = 0;
int currentDelta = 0;
int time = 15;
int highScore = 0;  
int currentScore = 0; 

int F = 0;
long lastActivityTime;

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
    enableInterrupt (BUTTONS[i], wakeUp, RISING);
  }
  pinMode(REDLED, OUTPUT);
  enableInterrupt(digitalPinToInterrupt(2), wakeUp, RISING);
  lastActivityTime = millis();
}

void loop() {
  if (!isIntroDisplayed) {
    displayIntro();
  } else if (!isDifficultySelected) {
    checkInactivity();
    selectDifficulty();
  } else if (!isGameStarted) {
    startGame();
  } else if (!isGameOver) {
    playGame();
  } else {
    finishGame();
  }
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


void checkInactivity() {
  if (millis() - lastActivityTime >= WAKE_UP_TIME) {
    goToSleepMode();
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

  Serial.println("WAKE UP");
  sleep_disable();
  lastActivityTime = millis();
  displayIntro();
  delay(1000);
}


void selectDifficulty() {
  blinkRedLed();

  int newPotValue = analogRead(A1);
  if(currentPotValue != newPotValue){
    currentPotValue = newPotValue;
    if(currentPotValue >= 0 && currentPotValue <= 256 && difficulty != 1){
      lastActivityTime = millis();
      lcd.setCursor(0,2);
      lcd.print("          ");
      lcd.setCursor(0,2);
      lcd.print(" :) EASY");
      difficulty = 1;
      F = 10;
    }else if(currentPotValue > 256 && currentPotValue <= 512 && difficulty != 2){
      lastActivityTime = millis();
      lcd.setCursor(0,2);
      lcd.print("          ");
      lcd.setCursor(0,2);
      lcd.print(" :| MEDIUM");
      difficulty = 2;
      F = 15;
    }else if(currentPotValue > 512 && currentPotValue <= 768 && difficulty != 3){
      lastActivityTime = millis();
      lcd.setCursor(0,2);
      lcd.print("          ");
      lcd.setCursor(0,2);
      lcd.print(">:) HARD");
      difficulty = 3;
      F = 27;
    }else if(currentPotValue > 768 && difficulty != 4){
      lastActivityTime = millis();
      lcd.setCursor(0,2);
      lcd.print("          ");
      lcd.setCursor(0,2);
      lcd.print(">:D EXPERT");
      difficulty = 4;
      F = 50;
    }
  }
  if (digitalRead(BUTTONS[3])) {
    isDifficultySelected = true;
    redLedState = LOW;
    digitalWrite(REDLED, redLedState);
    lastActivityTime = millis();
  }
}


void blinkRedLed() {
  long currentRedLedTime = millis();
  if (currentRedLedTime - previousRedLedBlinkTime >= LED_BLINK_INTERVAL) {
    previousRedLedBlinkTime = currentRedLedTime;
    redLedState = !redLedState;
    digitalWrite(REDLED, redLedState);
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
    lcd.print(targetNumber, DEC);
    shouldDisplayNumber = false;
    timeRoundStart = millis();
  } else {
    no_more_time();
    handleButtonPresses();
    if (targetNumber == convertButtonsStatesToDecimal()) {
      currentScore++;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Correct!");
      lcd.setCursor(0, 1);
      lcd.print("Score: ");
      lcd.print(currentScore);
      delay(1000);
      shouldDisplayNumber = true;
      turnOffAllLeds();
      time = time - (time*F/100);
      currentDelta = 0;
    }
  }
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
  lcd.clear();
  digitalWrite(REDLED, HIGH);
  delay(1000);
  digitalWrite(REDLED, LOW);
  lcd.print("Game over!");
  lcd.setCursor(0, 1);
  lcd.print("Final Score: ");
  lcd.print(currentScore);
  if (currentScore > highScore) {
    highScore = currentScore;  
  }
  delay(10000);
  resetGame();
}


void resetGame() {
  lcd.clear();
  isIntroDisplayed = false;
  isDifficultySelected = false;
  isGameStarted = false;
  isGameOver = false;
  timeRoundStart = 0;
  deltaT = 0;
  currentDelta = 0;
  currentScore = 0;
  difficulty = 0;
  currentPotValue = 0;
  shouldDisplayNumber = true;
  time =15;
  turnOffAllLeds();
  lastActivityTime = millis();
}


void no_more_time(){
int deltaT = (millis() - timeRoundStart)/1000 ;
 if(currentDelta != deltaT){
    lcd.setCursor(8,0);
    lcd.print("Time:");
    lcd.setCursor(13,0);
    lcd.print("  ");
    lcd.setCursor(13,0);
    lcd.print(time - currentDelta);
  }
  currentDelta = deltaT;
  if(currentDelta > time){
    finishGame();
  }
}
