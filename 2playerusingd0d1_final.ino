#include <MD_Parola.h>
#include <MD_MAX72XX.h>
#include <SPI.h>

// ----- MAX7219 LED Matrix Settings -----
#define MAX_DEVICES 4
#define DATA_PIN 11
#define CS_PIN 10
#define CLK_PIN 13

MD_Parola matrix = MD_Parola(MD_MAX72XX::FC16_HW, CS_PIN, MAX_DEVICES);

// ----- Pin Definitions for LEDs & Buttons -----
const int leds[] = {2, 3, 4, 5, A0, A1, A2, A3};       // LEDs: indices 0–3 for P1, 4–7 for P2
const int buttons[] = {6, 7, 8, 9, 0, 1, A4, A5};     // Buttons: indices 0–3 for P1, 4–7 for P2
const int resetButton = 12;                             // Reset button pin

// ----- Game Mode & Score Variables -----
bool twoPlayerMode = true;
bool gameOver = false;

int p1Score = 0, p2Score = 0;
int singleScore = 0;

// ----- Timing Constants -----
const long singleLedDuration = 700;
const long twoPlayerLedDuration = 500;
const long gapDurationSingle = 200;
const long gapDurationTwo = 200;
const long longPressDuration = 1500;

// ----- Debouncing Settings -----
const int debounceDelay = 50;  // Debounce delay in milliseconds
unsigned long lastButtonPressTime[8];  // Array for last press times

// ----- Function Prototypes -----
void resetGame(bool toggleMode);
void displaySingleScore();
void displayTwoPlayerScores();
void declareSingleWinner();
void declareTwoPlayerWinner();
bool isButtonPressed(int buttonIndex);

void setup() {
  matrix.begin();
  matrix.setIntensity(5);
  matrix.displayClear();

  for (int i = 0; i < 8; i++) {
    pinMode(leds[i], OUTPUT);
    digitalWrite(leds[i], LOW);
    pinMode(buttons[i], INPUT_PULLUP);
    lastButtonPressTime[i] = 0;  // Initialize the debounce timers
  }
  pinMode(resetButton, INPUT_PULLUP);

  resetGame(false);
}

void loop() {
  // ----- Reset Button Logic -----
  if (digitalRead(resetButton) == LOW) {
    unsigned long pressStart = millis();
    while (digitalRead(resetButton) == LOW) {
      if (millis() - pressStart >= longPressDuration) break;
    }
    unsigned long pressDuration = millis() - pressStart;
    if (pressDuration >= longPressDuration) {
      resetGame(true);
    } else {
      resetGame(false);
    }
    delay(200);
    return;
  }

  if (gameOver) return;

  if (twoPlayerMode) {
    // ----- TWO-PLAYER MODE -----
    int p1Led = random(0, 4);
    int p2Led = random(4, 8);
    digitalWrite(leds[p1Led], HIGH);
    digitalWrite(leds[p2Led], HIGH);

    unsigned long startTime = millis();
    bool p1Pressed = false, p2Pressed = false;

    while (millis() - startTime < twoPlayerLedDuration) {
      if (!p1Pressed && isButtonPressed(p1Led)) {
        p1Pressed = true;
        digitalWrite(leds[p1Led], LOW);
        p1Score++;
      }
      if (!p2Pressed && isButtonPressed(p2Led)) {
        p2Pressed = true;
        digitalWrite(leds[p2Led], LOW);
        p2Score++;
      }
    }

    digitalWrite(leds[p1Led], LOW);
    digitalWrite(leds[p2Led], LOW);

    displayTwoPlayerScores();

    if (p1Score >= 10 || p2Score >= 10) {
      declareTwoPlayerWinner();
    }

    delay(gapDurationTwo);

  } else {
    // ----- SINGLE-PLAYER MODE -----
    int ledIndex1 = random(0, 8);
    int ledIndex2;
    do {
      ledIndex2 = random(0, 8);
    } while (ledIndex2 == ledIndex1);

    digitalWrite(leds[ledIndex1], HIGH);
    digitalWrite(leds[ledIndex2], HIGH);

    unsigned long startTime = millis();
    bool pressed1 = false, pressed2 = false;

    while (millis() - startTime < singleLedDuration) {
      if (!pressed1 && isButtonPressed(ledIndex1)) {
        pressed1 = true;
        digitalWrite(leds[ledIndex1], LOW);
      }
      if (!pressed2 && isButtonPressed(ledIndex2)) {
        pressed2 = true;
        digitalWrite(leds[ledIndex2], LOW);
      }
      if (pressed1 && pressed2) {
        singleScore++;
        displaySingleScore();
        break;
      }
    }

    digitalWrite(leds[ledIndex1], LOW);
    digitalWrite(leds[ledIndex2], LOW);

    if (singleScore >= 10) {
      declareSingleWinner();
    }

    delay(gapDurationSingle);
  }
}

void resetGame(bool toggleMode) {
  if (toggleMode) {
    twoPlayerMode = !twoPlayerMode;
  }

  if (twoPlayerMode) {
    p1Score = 0;
    p2Score = 0;
  } else {
    singleScore = 0;
  }
  gameOver = false;

  for (int i = 0; i < 8; i++) {
    digitalWrite(leds[i], LOW);
  }
  matrix.displayClear();

  if (twoPlayerMode) {
    displayTwoPlayerScores();
  } else {
    displaySingleScore();
  }
}

void displayTwoPlayerScores() {
  char disp[10];
  sprintf(disp, "%d  %d", p2Score, p1Score);  // P2 on left, P1 on right
  matrix.displayText(disp, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
  matrix.displayAnimate();
}

void displaySingleScore() {
  matrix.displayClear();
  matrix.print(singleScore);
}

void declareTwoPlayerWinner() {
  gameOver = true;
  matrix.displayClear();
  if (p1Score >= 10) {
    matrix.displayText("P1WINS", PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
  } else {
    matrix.displayText("P2WINS", PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
  }
  matrix.displayAnimate();
}

void declareSingleWinner() {
  gameOver = true;
  matrix.displayClear();
  matrix.displayText("1PWINS", PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
  matrix.displayAnimate();
}

// ----- Button Debouncing Function -----
bool isButtonPressed(int buttonIndex) {
  static unsigned long lastDebounceTime[8] = {0};  // Store last debounce time for each button
  unsigned long currentTime = millis();

  // If the button is pressed and enough time has passed since the last press
  if (digitalRead(buttons[buttonIndex]) == LOW && (currentTime - lastDebounceTime[buttonIndex]) > debounceDelay) {
    lastDebounceTime[buttonIndex] = currentTime;  // Update the last debounce time
    return true;
  }
  return false;
}
