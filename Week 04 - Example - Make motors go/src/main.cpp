#include <Arduino.h>
#include <esp32-hal-ledc.h>

int ch1 = 0;
int ch2 = 1;

void setup() {
  ledcAttachPin(9, ch1);
  ledcAttachPin(10, ch2);
  // ledcChangeFrequency()
}

void loop() {
  ledcWrite(ch1, 255);
  delay(1000);
  ledcWrite(ch1, 170);
  delay(1000);
  ledcWrite(ch1, 0);
  delay(1000);
  ledcWrite(ch2, 170);
  delay(1000);
  ledcWrite(ch2, 255);
  delay(1000);
  ledcWrite(ch2, 0);

}

