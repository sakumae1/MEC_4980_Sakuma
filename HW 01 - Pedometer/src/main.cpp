// Creates a pedometer with a state machine approach
// Screens
  // Main Screen: Step Counter
  // Second Screen: Distance Traveled Based on Stride Length
  // Third Screen: Adjusts Stride Length
  // Fourth Screen: Raw Accleration Values (accelX, accelY, accelZ)


#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO08x.h>
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

sh2_SensorValue_t sensorValue;

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

  accelerometer.enableReport(SH2_RAW_ACCELEROMETER, 20000);
  accelerometer.enableReport(SH2_STEP_COUNTER, 20000);
}

// loop
void loop () {
  // *Read accelerometer*
  if (accelerometer.wasReset()) {
    // read raw accelerometer data from the sensor
    accelerometer.enableReport(SH2_RAW_ACCELEROMETER, 20000);

    // read steps data from the sensor
    accelerometer.enableReport(SH2_STEP_COUNTER, 20000);
  }

  if (accelerometer.getSensorEvent(&sensorValue)) {
    if (sensorValue.sensorId == SH2_RAW_ACCELEROMETER) {

      accelX = sensorValue.un.rawAccelerometer.x;
      accelY = sensorValue.un.rawAccelerometer.y;
      accelZ = sensorValue.un.rawAccelerometer.z;

    }


    // *Detect Steps*
    if (sensorValue.sensorId == SH2_STEP_COUNTER) {
      stepCount = sensorValue.un.stepCounter.steps;
    }
  }


  // *Calculate Distance*
  distance = stepCount * strideLength;


  // *Handle Buttons*
  // Change Screen
  if (digitalRead(Button_D2)) {
    currentScreen = (screenState)(((int)currentScreen + 1) % int(sCount));
    delay(150);
  }

  // decrease stride length
  if (!digitalRead(Button_D0)) {

    if (currentScreen == StrideScreen) {
      strideLength -= 0.05;
      if (strideLength < 0.50) {
        strideLength = 0.50;
      }
    }
    delay(150);
  }

  // increase stride length
  if (digitalRead(Button_D1)) {
    if (currentScreen == StrideScreen) {
      strideLength += 0.05;
      if (strideLength > 1.00) {
        strideLength = 1.00;
      }
    }
    delay(150);
  }

  // *Screen Display*
  canvas.fillScreen(ST77XX_ORANGE);
  canvas.setCursor(0, 20);
  canvas.setTextColor(ST77XX_GREEN);
  canvas.setTextSize(2);

  if (currentScreen == MainScreen) {

    canvas.println("Pedometer");
    canvas.println(" ");
    canvas.print("Steps = ");
    canvas.println(stepCount);

  } else if (currentScreen == DistanceScreen) {

    canvas.println("Distance Traveled");
    canvas.println(" ");
    canvas.print("Distance = ");
    canvas.print(distance);
    canvas.println(" m");

  } else if (currentScreen == StrideScreen) {

    canvas.println("Stride Length"); // Heel-to-Heel Distance Between Opposite Feet
    canvas.println(" ");
    canvas.print("Stride = ");
    canvas.print(strideLength);
    canvas.println(" m");

    canvas.println("D0 = -");
    canvas.println("D1 = +");

  } else if (currentScreen == AccelerationScreen) {

    canvas.println("Raw Acceleration");
    canvas.println("(Unscaled Direct ADC Readings)");
    canvas.println(" ");

    canvas.print("X = ");
    canvas.println(accelX);

    canvas.print("Y = ");
    canvas.println(accelY);

    canvas.print("Z = ");
    canvas.println(accelZ);

  }
  display.drawRGBBitmap(0,0, canvas.getBuffer(), 240, 135);

  delay(10);
  
}