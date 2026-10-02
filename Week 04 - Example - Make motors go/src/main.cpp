#include <Arduino.h>
#include <esp32-hal-ledc.h>

int ch1 = 9;
int ch2 = 10;

void setup() {
  // ledcAttachPin(9, ch1);
  // ledcAttachPin(10, ch2);
  // ledcChangeFrequency()
}

void loop() {
  analogWrite(ch1, 255);
  delay(1000);
  analogWrite(ch1, 170);
  delay(1000);
  analogWrite(ch1, 0);
  delay(1000);
  analogWrite(ch2, 170);
  delay(1000);
  analogWrite(ch2, 255);
  delay(1000);
  analogWrite(ch2, 0);

}

