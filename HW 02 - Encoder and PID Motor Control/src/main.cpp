#include <Arduino.h>
#include <PID_v1.h>
#include <Wire.h>
#include <esp32-hal-ledc.h>

// Motor Pins
int ch1 = 0;
int ch2 = 1;

#define MotorPin1 9
#define MotorPin2 10

// Encoder
#define Pin_Encoder A5
      // #define PIN_OUTPUT 3

#define EncoderSlots 10

// set sample time
#define sample_time 100

// photoresistor thresholds
#define dark 1200
#define light 1800

//PID variables
double Setpoint, Input, Output;
double Kp=2, Ki=5, Kd=1;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

// encoder variables
unsigned long EncoderCount = 0;
unsigned long PreviousEncoderCount = 0;
unsigned long PreviousTime = 0;

bool LightState = false;

void setup()
{
  Serial.begin(9600);

  ledcAttachPin(MotorPin1, ch1);
  ledcAttachPin(MotorPin2, ch2);

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
  // read photoresitor
  int LightLevel = analogRead(Pin_Encoder);

  // detect encoder slots
  if (!LightState && LightLevel >= light){
    LightState = true;

    // one slot passed
    EncoderCount++;
  }

  if (LightState && LightLevel <= dark) {
    LightState = false;
  }

  // calculate RPM
  unsigned long CurrentTime = millis();

  if (CurrentTime - PreviousTime >= sample_time) {

    // encoder pulses
    unsigned long NewCounts = EncoderCount - PreviousEncoderCount;

    // convert pulses to revs
    double Revolutions = (double)NewCounts / EncoderSlots;

    // revs per 100ms to revs per min
    Input = Revolutions*(60000/sample_time);

    // save encoder count
    PreviousEncoderCount = EncoderCount;

    // PID
    myPID.Compute();

    ledcWrite(ch1, (int)Output);
    ledcWrite(ch2, 0);

    // display info
    Serial.print("Light: ");
    Serial.print(LightLevel);

    Serial.print(" | Counts: ");
    Serial.print(LightLevel);

    Serial.print(" | RPM: ");
    Serial.print(Setpoint);

    Serial.print(" | Target RPM: ");
    Serial.print(Setpoint);

    Serial.print(" | PWM: ");
    Serial.println(Output);

    PreviousTime = CurrentTime;
  }
}