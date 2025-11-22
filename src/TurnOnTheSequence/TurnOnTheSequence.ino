/*
 * Assignment:  #01 - Turn on the Sequence! (TOS)
 * Author:      Todeschi Matteo
 * Date:        November 2025
 * Dependencies:
 * - LiquidCrystal_I2C.h
 * - EnableInterrupt.h
 * - avr/sleep.h & avr/power.h
 *
 * WIRING & PIN MAPPING
 * 
 * [INPUTS]
 * Button 1 (B1)   : Pin 5  (Input 1 / Start)
 * Button 2 (B2)   : Pin 4  (Input 2)
 * Button 3 (B3)   : Pin 3  (Input 3)
 * Button 4 (B4)   : Pin 2  (Input 4)
 * Potentiometer   : Pin A3 (Difficulty Selection)
 * Unconnected     : Pin A0 (Random Seed Noise)
 * [OUTPUTS]
 * Green LED 1 (L1): Pin 11
 * Green LED 2 (L2): Pin 10
 * Green LED 3 (L3): Pin 9
 * Green LED 4 (L4): Pin 8
 * Red LED (LS)    : Pin 6 (PWM)
 * [I2C LCD DISPLAY]
 * SDA             : Pin A4
 * SCL             : Pin A5
 * Address         : 0x27 (Column: 16, Rows: 2)
 */

#include <EnableInterrupt.h>
#include <LiquidCrystal_I2C.h>
#include <avr/sleep.h>
#include <avr/power.h>

#define BUTTON1_PIN 5
#define BUTTON2_PIN 4
#define BUTTON3_PIN 3
#define BUTTON4_PIN 2
#define LED1_PIN 11
#define LED2_PIN 10
#define LED3_PIN 9
#define LED4_PIN 8
#define LEDR_PIN 6
#define POT_PIN A3

#define MIN_TIME_FACTOR 0.5
#define MAX_TIME_FACTOR 0.8
#define MAX_DIFFICULTY_LEVELS 4

#define WELCOME 1
#define WAIT 2
#define SELECT_DIFF 3
#define INIT_GAME 4
#define INPUT_GAME 5
#define SHOW_RESULT 6
#define NONE -1

// General
int fase;
int score = 0;
int attempt = 0;
bool result;
bool timedOut;
unsigned long int begin;
unsigned long int maxTime = 10000;

// Sequence
int sequence[4];
float reduceTimeFactor;
int volatile difficulty;

// Button click
const long debounceDelay = 150;
const int ledPins[5] = {6, 11, 10, 9, 8};
volatile int inputIndex = 0;
volatile int buttonPressed = 0;
volatile unsigned long lastInterrupt1 = 0;
volatile unsigned long lastInterrupt2 = 0;
volatile unsigned long lastInterrupt3 = 0;
volatile unsigned long lastInterrupt4 = 0;

// Fading led
int currentBrightness = 0;
bool fadingUp = true;
unsigned long lastFadeTime = 0;
const int fadeInterval = 5;

// LCD
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27,16,2);

/* - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Interrupt handling */
void clicked1(){
  buttonClicked(1, lastInterrupt1);
}

void clicked2(){
  buttonClicked(2, lastInterrupt2);
}

void clicked3(){
  buttonClicked(3, lastInterrupt3);
}

void clicked4(){
  buttonClicked(4, lastInterrupt4);
}

void buttonClicked(int button, volatile unsigned long &lastTime){
  unsigned long now = millis();
  if (now - lastTime > debounceDelay) {
    lastTime = now;
    buttonPressed = button;
  }
}

void startGameInt(){
  unsigned long now = millis();
  if (now - lastInterrupt1 < debounceDelay) {
    return;
  }
  lastInterrupt1 = now;
  resetFadeState();
  disableInterrupt(BUTTON1_PIN);
  enableInterrupt(BUTTON1_PIN, confirmDifficulty, RISING);
  fase = SELECT_DIFF;
}

void confirmDifficulty(){
  unsigned long now = millis();
  if (now - lastInterrupt1 < debounceDelay) {
    return;
  }
  lastInterrupt1 = now;
  disableInterrupt(BUTTON1_PIN);
  reduceTimeFactor = MIN_TIME_FACTOR + (difficulty - 1) * (MAX_TIME_FACTOR - MIN_TIME_FACTOR) / (MAX_DIFFICULTY_LEVELS - 1);
  fase = INIT_GAME;
}

void wakeUp(){ }

/* - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Setup and Loop */
void setup(){
  Serial.begin(9600);
  pinMode(POT_PIN, INPUT);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(LED4_PIN, OUTPUT);
  pinMode(LEDR_PIN, OUTPUT);
  randomSeed(analogRead(A0));
  lcd.init();
  lcd.backlight();
  lcd.clear();
  fase = WELCOME;
  result = false;
  difficulty = NONE;
  score = 0;
  timedOut = false;
}

void enterSleepMode() {
  resetFadeState();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.noBacklight();
  Serial.println("Sleeping");
  while(digitalRead(BUTTON1_PIN) == LOW) delay(10);

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  enableInterrupt(BUTTON1_PIN, wakeUp, FALLING);
  sleep_cpu();
  sleep_disable();
  disableInterrupt(BUTTON1_PIN);

  pinMode(BUTTON1_PIN, INPUT);
  lcd.backlight();
  Serial.println("Waking");
  fase = WELCOME;
}

void loop(){
  switch ( fase ){
    case WELCOME:
      score = 0;
      attempt = 0;
      difficulty = NONE;
      maxTime = 10000;
      screenPrintUp("Welcome to TOS!");
      screenPrintDw("Press B1 to Play");
      enableInterrupt(BUTTON1_PIN, startGameInt, RISING);
      begin = millis();
      fase = WAIT;
      break;
    case WAIT:
      fadeLed(LEDR_PIN);
      if (millis() - begin > 10000) {
        disableInterrupt(BUTTON1_PIN);
        pinMode(BUTTON1_PIN, INPUT_PULLUP);
        enterSleepMode();
        pinMode(BUTTON1_PIN, INPUT);  
      }
      break;
    case SELECT_DIFF:
      readDifficulty();
      break;
    case INIT_GAME:
      inputIndex = 0;
      enableInterrupt(BUTTON1_PIN, clicked1, RISING);
      enableInterrupt(BUTTON2_PIN, clicked2, RISING);
      enableInterrupt(BUTTON3_PIN, clicked3, RISING);
      enableInterrupt(BUTTON4_PIN, clicked4, RISING);
      generateArray(sequence);
      Serial.println("Time to answer: " + String(maxTime));
      screenPrintUp("Go!");
      screenPrintDw(arrayToString(sequence));
      fase = INPUT_GAME;
      begin = millis();
      break;
    case INPUT_GAME:
      if (buttonPressed != 0) {
        verifyClick(buttonPressed);
        buttonPressed = 0;
      }
      if(millis() - begin > maxTime){
        result = false;
        timedOut = true;
        fase = SHOW_RESULT;
      }
      break;
    case SHOW_RESULT:
      disableInterrupt(BUTTON1_PIN);
      disableInterrupt(BUTTON2_PIN);
      disableInterrupt(BUTTON3_PIN);
      disableInterrupt(BUTTON4_PIN);
      if (result){
        score = score + 10 + (5 * attempt);
        attempt++;
        screenPrintUp("GOOD!");
        screenPrintDw("Score: " + String(score));
        delay(2000);
        maxTime = (unsigned long int) (maxTime*reduceTimeFactor);
        fase = INIT_GAME;
        resetLeds();
      } else {
        digitalWrite(ledPins[0], HIGH);
        delay(2000);
        resetLeds();
        if(timedOut){
          screenPrintUp("Game Over - Time");
          Serial.println("Game Over - Time");
          timedOut = false;
        } else {
          screenPrintUp("Game Over - Miss");
          Serial.println("Game Over - Miss");
        }
        screenPrintDw("Final Score: " + String(score));
        delay(10000);
        fase = WELCOME;
      }
      break;
    default:
      break;
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Utils */
void turnOnLed(int led){
  resetLeds();
  digitalWrite(ledPins[led], HIGH);
}

void resetLeds(){
  for (int i = 0; i < 5; i++){
    digitalWrite(ledPins[i], LOW);
  }
}

void generateArray(int array[4]) {
  for (int i = 0; i < 4; i++) {
    array[i] = random(1, 5);
  }
}

String arrayToString(int array[4]) {
  String result = "";
  for (int i = 0; i < 4; i++) {
    result += String(array[i]);
    if (i < 3) {
      result += ", ";
    }
  }
  return result;
}

void screenPrintUp(String message){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(message);
}

void screenPrintDw(String message){
  lcd.setCursor(0, 1);
  lcd.print(message);
}

void verifyClick(int buttonId) {
  turnOnLed(buttonId);
  Serial.println("Clicked " + String(buttonId));
  if (buttonId != sequence[inputIndex]) {
    result = false;
    fase = SHOW_RESULT;
    return;
  }
  inputIndex++;
  if (inputIndex >= 4) {
    result = true;
    fase = SHOW_RESULT;
  }
}

void readDifficulty() {
  int potRead = analogRead(POT_PIN);
  int newDiff = map(potRead, 0, 1023, 1, 4);
  if (newDiff != difficulty) {
    difficulty = newDiff;
    screenPrintDw("                ");
    screenPrintDw(getDifficultyLabel());
  }
}

String getDifficultyLabel(){
  switch ( difficulty ){
    case 4:
      return "Easy";
    case 3:
      return "Medium";
    case 2:
      return "Hard";
    case 1:
      return "Extreme";
    default:
      return "Error!";
  }
}

void fadeLed(int pin) {
  unsigned long now = millis();
  if (now - lastFadeTime >= fadeInterval) {
    lastFadeTime = now;
    if (fadingUp) {
      currentBrightness += 1;
      if (currentBrightness >= 255) {
        currentBrightness = 255;
        fadingUp = false;
      }
    } else {
      currentBrightness -= 1;
      if (currentBrightness <= 0) {
        currentBrightness = 0;
        fadingUp = true;
      }
    }
    analogWrite(pin, currentBrightness);
  }
}

void resetFadeState() {
  currentBrightness = 0;
  fadingUp = true;
  digitalWrite(LEDR_PIN, LOW);
}