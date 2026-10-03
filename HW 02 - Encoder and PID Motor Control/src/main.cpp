#include <Arduino.h>
#include <PID_v1.h>
#include <Wire.h>
#include <esp32-hal-ledc.h>

// Motor Pins
int ch1 = 9;
int ch2 = 10;

// Encoder
#define Pin_Encoder A5

#define EncoderSlots 5

// set sample time
#define sample_time 10

// photoresistor thresholds
#define dark 2000
#define light 3000

//PID variables
double Setpoint, Input, Output;
double Kp=1, Ki=0.5, Kd=0;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

// encoder variables
unsigned long EncoderCount = 0;
unsigned long PreviousTime = 0;

unsigned long LastPulseTime = 0;
unsigned long PulsePeriod = 0;

bool LightState = false;
bool FirstPulse = true;

void setup()
{
  Serial.begin(9600);

  analogReadResolution(12);
  pinMode(Pin_Encoder, INPUT);

  // inital motor speed;
  Output = 0;
  Input = 0;

  // target motor speed
  Setpoint = 100;

  // PID
  myPID.SetOutputLimits(0,255);
  myPID.SetSampleTime(sample_time);
  myPID.SetMode(AUTOMATIC);

  // start timer
  PreviousTime = millis();
}

void loop()
{
  // PID calculates the required PWM
  myPID.Compute();

  analogWrite(ch1, (int)Output);
  analogWrite(ch2, 0);

  // Read photoresistor
  int LightLevel = analogRead(Pin_Encoder);

  // Detect light transition
  if (!LightState && LightLevel >= light) {
    LightState = true;
    EncoderCount++;

    unsigned long CurrentPulseTime = millis();

    // Calculate time between encoder pulses
    if (!FirstPulse) {
      PulsePeriod = CurrentPulseTime - LastPulseTime;

      if (PulsePeriod > 0) {
        Input = 60000.0 / ((double)PulsePeriod * EncoderSlots);
      }
    }

    LastPulseTime = CurrentPulseTime;
    FirstPulse = false;
  }

  // Detect dark transition
  if (LightState && LightLevel <= dark) {
    LightState = false;
  }

  // Print every 200 ms
  static unsigned long LastPrintTime = 0;

  if (millis() - LastPrintTime >= 200) {

    Serial.print("Light: ");
    Serial.print(LightLevel);

    Serial.print(" | Counts: ");
    Serial.print(EncoderCount);

    Serial.print(" | Period: ");
    Serial.print(PulsePeriod);
    Serial.print(" ms");

    Serial.print(" | RPM: ");
    Serial.print(Input);

    Serial.print(" | Target RPM: ");
    Serial.print(Setpoint);

    Serial.print(" | PWM: ");
    Serial.println(Output);

    LastPrintTime = millis();
  }

  delay(10);
}