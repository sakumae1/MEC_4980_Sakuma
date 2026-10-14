#include <Arduino.h>

int pinStep = 9;
int pinDir = 10;

volatile bool shouldStep = false;
bool doneStepping = true;

long stepFreq = 200*2; // *2 for including off portion of signal

hw_timer_t *stepTimer = NULL;

void ARDUINO_ISR_ATTR toggleStep() {
  shouldStep = !shouldStep;
  digitalWrite(pinStep, shouldStep);
}

void setup() {
  pinMode(pinStep, OUTPUT);
  pinMode(pinDir, OUTPUT);

  stepTimer = timerBegin(stepFreq);
  timerAttachInterrupt(stepTimerm, &toggleStep);
  timerAlarm(stepTimer, 1, true, 0);
}

void loop() {
  
}

