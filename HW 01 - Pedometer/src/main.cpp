// Creates a pedometer with a state machine approach
// Screens
  // Main Screen: Step Counter
  // Second Screen: Approximate Distance Based on Stride Length
  // Third Screen: Adjust Stride Length
  // Fourth Screen: Raw Accleration Values (ax, ay, az)


#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BNO08x.h"
#include <Arduino.h>
#include <Adafruit_ST7789.h>

// Display
Adafruit_ST7789 display = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
GFXcanvas16 canvas(240, 135);

// Create Object
Adafruit_BNO08x accelerometer;

// States
enum screenState {
  MainScreen,
  DistanceScreen,
  StrideScreen,
  AccelerationScreen,
  sCount
};

screenState currentScreen = MainScreen;

// Pedometer Variables
int stepCount = 0;
float strideLength = 0.75;
float distance = 0.0;
float accelX = 0.0; // raw X-direction acceleration
float accelY = 0.0; // raw Y-direction acceleration
float accelZ = 0.0; // raw Z-direction acceleration
float X = 0.0; // linear X-direction acceleration
float Y = 0.0; // linear Y-direction acceleration
float Z = 0.0; // linear Z-direction acceleration


float linearAccelMagnitude = 0.0;
sh2_SensorValue_t sensorValue;
bool stepDetect = false;
unsigned long lastStepTime = 0;

// Buttons
# define Button_D0 0
# define Button_D1 1
# define Button_D2 2

// Setup
void setup() {
  Serial.begin(9600);

  // Display
  display.init(135, 240);
  display.setRotation(3);
  canvas.setTextColor(ST77XX_GREEN);
  pinMode(TFT_BACKLITE, OUTPUT);
  digitalWrite(TFT_BACKLITE, 1);
  digitalWrite(TFT_I2C_POWER, HIGH);
  delay(10);

  // Buttons
  pinMode(Button_D0, INPUT_PULLUP);
  pinMode(Button_D1, INPUT_PULLDOWN);
  pinMode(Button_D2, INPUT_PULLDOWN);

  // Initialize Accelerometer
  if (!accelerometer.begin_I2C()) {
    Serial.println("BNO085 not found!");
    while(1);
  }
  Serial.println("BNO085 found!");

  accelerometer.enableReport(SH2_ACCELEROMETER, 20000);
  accelerometer.enableReport(SH2_LINEAR_ACCELERATION, 20000);
  accelerometer.enableReport(SH2_RAW_ACCELEROMETER, 20000);
}

// loop
void loop () {
  // Read accelerometer
  if (accelerometer.wasReset()) {
    accelerometer.enableReport(SH2_ACCELEROMETER, 20000);
    accelerometer.enableReport(SH2_LINEAR_ACCELERATION, 20000);
    accelerometer.enableReport(SH2_RAW_ACCELEROMETER, 20000);
  }

  if (accelerometer.getSensorEvent(&sensorValue)) {
    if (sensorValue.sensorId == SH2_RAW_ACCELEROMETER) {

      accelX = sensorValue.un.rawAccelerometer.x;
      accelY = sensorValue.un.rawAccelerometer.y;
      accelZ = sensorValue.un.rawAccelerometer.z;

    }

    if (sensorValue.sensorId == SH2_LINEAR_ACCELERATION) {

      X = sensorValue.un.linearAcceleration.x;

      Y = sensorValue.un.linearAcceleration.y;

      Z = sensorValue.un.linearAcceleration.z;
      
      linearAccelMagnitude = sqrt(X*X + Y*Y + Z*Z);

    }

  }

  // Detect Steps
  float stepThreshold = .70;

  if (linearAccelMagnitude > stepThreshold && stepDetect == false && millis() - lastStepTime > 300) {
    stepCount++;

    stepDetect = true;
    lastStepTime = millis();
  }

  if (linearAccelMagnitude < 1.0) {
    stepDetect = false;
  }

  // Calculate Distance
  distance = stepCount * strideLength;

  // Handle Buttons

  if (digitalRead(Button_D2)) {
    currentScreen = (screenState)(((int)currentScreen + 1) % int(sCount));
    delay(150);
  }


  if (!digitalRead(Button_D0)) {

    if (currentScreen == StrideScreen) {
      strideLength -= 0.05;
      if (strideLength < 0.20) {
        strideLength = 0.20;
      }
    }
    delay(150);
  }

  if (digitalRead(Button_D1)) {
    if (currentScreen == StrideScreen) {
      strideLength += 0.05;
      if (strideLength > 2.00) {
        strideLength = 2.00;
      }
    }
    delay(150);
  }

  // Screen Display
  canvas.fillScreen(ST77XX_ORANGE);
  canvas.setCursor(0, 20);
  canvas.setTextColor(ST77XX_GREEN);
      // canvas.setTextSize(2)

  if (currentScreen == MainScreen) {

    canvas.println("Pedometer");
    canvas.print("Steps = ");
    canvas.println(stepCount);

  } else if (currentScreen == DistanceScreen) {

    canvas.println("Distance");
    canvas.print("Distance = ");
    canvas.print(distance);
    canvas.println(" m");

  } else if (currentScreen == StrideScreen) {

    canvas.println("Stride Length");
    canvas.print("Stride = ");
    canvas.print(strideLength);
    canvas.println(" m");

    canvas.println("D0 = -");
    canvas.println("D1 = +");

  } else if (currentScreen == AccelerationScreen) {

    canvas.println("Raw Acceleration");

    canvas.print("X = ");
    canvas.println(accelX);

    canvas.print("Y = ");
    canvas.println(accelY);

    canvas.print("Z = ");
    canvas.println(accelZ);

  }
  display.drawRGBBitmap(0,0, canvas.getBuffer(), 240, 135);
  
  Serial.println(linearAccelMagnitude);

  delay(30);
  
}
