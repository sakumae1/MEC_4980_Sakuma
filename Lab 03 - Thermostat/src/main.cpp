/***************************************************************************
  This is a library for the BME680 gas, humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BME680 Breakout
  ----> http://www.adafruit.com/products/3660

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#include <Wire.h> 
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <Adafruit_ST7789.h>
Adafruit_ST7789 display = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
GFXcanvas16 canvas(240, 135);
long startTime = 0;


#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25) // standard pressure at sea level

enum hvacState {
  Heating, // 0
  Cooling, // 1
  hCount // 2
};

enum menuState {
  MainMenu, // 0
  TemperatureMenu, // 1
  OperationMenu, // 2
  UnitMenu, // 3
  mCount // 4
};

enum tempState {
  C,
  F,
  tcount
};

hvacState opMode = Heating;
menuState menuMode = MainMenu;
tempState tempMode = C; 
float targetTemp = 24.0; // floating point (target) temperature
float targetDisplayTemp;
volatile long prevChangeTime = 0;
volatile long prevChangeTimeTwo = 0;
long debounceTime = 50;
volatile bool changeButtonFlag = false;
volatile bool menuButtonFlag = false;

void IRAM_ATTR buttonToChangeThings() {
  long now = millis();
  if (now > prevChangeTime + debounceTime) {
    changeButtonFlag = true;
    prevChangeTime = now;
  }
}

void IRAM_ATTR buttonToChangeMenu() {
  long now = millis();
  if (now > prevChangeTimeTwo + debounceTime) {
    menuButtonFlag = true;
    prevChangeTimeTwo = now;
  }
}

Adafruit_BME680 bme(&Wire); // I2C
//Adafruit_BME680 bme(&Wire1); // example of I2C on another bus
//Adafruit_BME680 bme(BME_CS); // hardware SPI
//Adafruit_BME680 bme(BME_CS, BME_MOSI, BME_MISO,  BME_SCK);

float getCurrentTemp() {
  if (tempMode == tempState::C) {
    return bme.temperature;
  }
  if (tempMode == tempState::F) {
    return bme.temperature * 9. / 5. + 32.;
  }
  return -1100.;
}

void setup() {
  display.init(135, 240);
  display.setRotation(3);
  canvas.setTextColor(ST77XX_GREEN);
  pinMode(TFT_BACKLITE, OUTPUT);
  digitalWrite(TFT_BACKLITE, 1);
      startTime = millis();
      Serial.begin(9600);
  while (!Serial);
  Serial.println(F("BME680 test"));

  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }

pinMode(1, INPUT_PULLDOWN);
attachInterrupt(digitalPinToInterrupt(1), buttonToChangeThings, RISING);

pinMode(2, INPUT_PULLDOWN);
attachInterrupt(digitalPinToInterrupt(2), buttonToChangeMenu, RISING);
  
  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_2X);
  //bme.setHumidityOversampling(BME680_OS_2X);
  //bme.setPressureOversampling(BME680_OS_4X);
  //bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  //bme.setGasHeater(320, 150); // 320*C for 150 ms
}

void loop() {
  if (! bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }

  // breat into seperate "if statements" to have different menus
  float currentTemp = getCurrentTemp();

  if (tempMode == tempState::C) {
    targetDisplayTemp = targetTemp;
  } else {
    targetDisplayTemp = targetTemp * 9. /5. + 32.; // converting target temp based on mode
  }

  canvas.fillScreen(ST77XX_ORANGE);
  canvas.setCursor(0,20);

  // Menu Commands
  if (menuMode == MainMenu) {
    canvas.println("Main Menu");
    canvas.print("Temperature = ");
    canvas.print(currentTemp);

    if (tempMode == tempState::C) {
      canvas.print(" *C");
    } else {
      canvas.print(" *F");
    }
    
    canvas.println(" with Target ");
    // canvas.print(targetTemp);
    canvas.print(targetDisplayTemp);

    if (tempMode == tempState::C) {
      canvas.println(" *C");
    } else {
      canvas.println(" *F");
    }

    canvas.print("Operating in Mode: ");

    if (opMode == Heating) {
      canvas.print("Heating");
    } else if (opMode == Cooling) {
      canvas.print("Cooling");
    }

    // if (opMode == Heating) {
      // if (bme.temperature < targetTemp) {
      // canvas.println("Heater is on now!");
      // }
    // } else if (opMode == Cooling) {
    // if (bme.temperature > targetTemp) {
      // canvas.println("AC is on now!");
    // }

  // Menu 1  
  } else if (menuMode == TemperatureMenu) {
    canvas.println("Menu 1");
    canvas.print("Temperature = ");
    canvas.print(currentTemp);
    if (tempMode == tempState::C) {
      canvas.print(" *C");
    } else {
      canvas.print(" *F");
    }
    
    canvas.println(" with Target ");
    // canvas.print(targetTemp);
    canvas.print(targetDisplayTemp);

    if (tempMode == tempState::C) {
      canvas.println(" *C");
    } else {
      canvas.println(" *F");
    }

    canvas.print("Operating in Mode: ");

    if (opMode == Heating) {
      canvas.print("Heating");
    } else if (opMode == Cooling) {
      canvas.print("Cooling");
    }
  // Menu 2
  } else if (menuMode == OperationMenu) {
      canvas.println("Menu 2");
      canvas.print("Operation Mode = ");
      if (opMode == Heating) {
        canvas.print("Heating");
      } else if (opMode == Cooling) {
        canvas.print("Cooling");
    }
  // Menu 3
  } else if (menuMode == UnitMenu) {
      canvas.println("Menu 3");
      canvas.print("Temperature Unit Is ");
      if (tempMode == tempState::C) {
        canvas.println("Celsius");
      } else {
        canvas.println("Fahrenheit");
      }
  }


  // canvas.print(" in menu ");
  // if (menuMode == MainMenu) {
    // canvas.println("1");
  // } else if(menuMode == TemperatureMenu) {
    // canvas.println("2");
  // } else if (menuMode == OperationMenu) {
    // canvas.println("3");
  // } else if (menuMode == UnitMenu) {
    // canvas.println("4");
  // }

if (menuButtonFlag) {
  menuButtonFlag = false;
  menuMode = (menuState)(((int)menuMode + 1) % (int)menuState::mCount);
  Serial.print("!!!!!!Moving to menu: ");
  Serial.println(menuMode);
}

  if (changeButtonFlag) {
    if (menuMode == TemperatureMenu) {
      targetTemp += 1.0;
      if (targetTemp > 30.) {
        targetTemp = targetTemp -10.;
      }
    }
    if (menuMode == OperationMenu) {
      opMode = (hvacState)(((int)opMode + 1) % (int)hvacState::hCount);
    }
    if (menuMode == UnitMenu) {
      // change from F to C or C to F
      tempMode = (tempState)(((int)tempMode + 1) % (int)tempState::tcount);
    }
    changeButtonFlag = false;
    
  }



/*
  Serial.print("Pressure = ");
  Serial.print(bme.pressure / 100.0);
  Serial.println(" hPa");
/*
  Serial.print("Humidity = ");
  Serial.print(bme.humidity);
  Serial.println(" %");
*/
  // Serial.println();
  display.drawRGBBitmap(0,0, canvas.getBuffer(), 240, 135);
  delay(100);
}

long getTime() {
  return (millis() - startTime);
}