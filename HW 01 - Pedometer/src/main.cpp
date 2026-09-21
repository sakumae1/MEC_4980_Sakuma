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
float accelX = 0.0;
float accelY = 0.0;
float accelZ = 0.0;

// Buttons
# define Button_D0 0
# define Button_D1 1
# define Button_D2 2

// Setup
void setup() {
  Serial.begin();

  // Display
  display.init(135, 240);
  display.setRotation(3);
  canvas.setTextColor(ST77XX_GREEN);
  digitalWrite(TFT_BACKLITE, HIGH);

  // Buttons
  pinMode(Button_D0, INPUT);
  pinMode(Button_D1, INPUT);
  pinMode(Button_D2, INPUT);

  // Initialize Accelerometer
  if (!accelerometer.begin_I2C()) {
    Serial.println("BNO085 not found!");
    while(1);
  }
  Serial.println("BNO085 found!");
}

// loop
void loop () {
  // Read accelerometer


  // Detect Steps


  // Handle Buttons


  // Screen Display


  delay(10);
}