#include <Arduino.h>
#include <PID_v1.h>
#include <Wire.h>
#include <esp32-hal-ledc.h>

// Motor Pins
  //int ch1 = 0;
  //int ch2 = 1;

  //#define MotorPin1 9
  //#define MotorPin2 10

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
double Kp=2, Ki=5, Kd=1;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

// encoder variables
unsigned long EncoderCount = 0;
// unsigned long PreviousEncoderCount = 0;
unsigned long PreviousTime = 0;
unsigned long LastPulseTime = 0;
unsigned long PulsePeriod = 0;

// double MeasuredRPM = 0;

bool LightState = false;
bool FirstPulse = true;

void setup()
{
  Serial.begin(9600);

  //ledcAttachPin(MotorPin1, ch1);
  //ledcAttachPin(MotorPin2, ch2);

  analogReadResolution(12);
  pinMode(Pin_Encoder, INPUT);

  // inital motor speed;
  Output = 0;
  Input = 0;

  // target motor speed
  Setpoint = 25;

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

    unsigned long CurrentPulseTime = millis();

    if (!FirstPulse) {
      PulsePeriod = CurrentPulseTime - LastPulseTime;
        if (PulsePeriod > 0) {
      Input = 60000.0 / (PulsePeriod*EncoderSlots);
      }
    }
    LastPulseTime = CurrentPulseTime;
    FirstPulse = false;
  }

  

    if (LightState && LightLevel <= dark) {
      LightState = false;
    }

    /*
    if (!FirstPulse && (millis() - LastPulseTime > 3000)) {
      Input = 0;
    }
    */

  // calculate RPM
  unsigned long CurrentTime = millis();

  if (CurrentTime - PreviousTime >= sample_time) {

    // encoder pulses
    // unsigned long NewCounts = EncoderCount - PreviousEncoderCount;

    /*
      // convert pulses to revs
      double Revolutions = (double)NewCounts / EncoderSlots;

      // revs per 10ms to revs per min
      Input = Revolutions*(60000/sample_time);
    */

    // save encoder count
    // PreviousEncoderCount = EncoderCount;

    // PID
    myPID.Compute();

    analogWrite(ch1, (int)Output);
      // analogWrite(ch1, Setpoint);
    analogWrite(ch2, 0);

    PreviousTime = CurrentTime;

    static unsigned long LastPrintTime = 0;

    // display info
    if (CurrentTime - LastPrintTime >= 200) {

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

      LastPrintTime = CurrentTime;
    }
  }
}