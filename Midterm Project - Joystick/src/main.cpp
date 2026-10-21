#include <Arduino.h>
#include <math.h>

// Joystick inputs
constexpr int JOY_X_PIN  = A0;
constexpr int JOY_Y_PIN  = A1;
constexpr int JOY_SW_PIN = 9;   // Button moved from D5 to D9

// UART pins for communication with the motor Feather
constexpr int UART_RX_PIN = 5;  // D5
constexpr int UART_TX_PIN = 6;  // D6

constexpr uint32_t USB_BAUD  = 115200;
constexpr uint32_t UART_BAUD = 9600;

constexpr uint32_t SEND_INTERVAL_MS = 20;  // 50 Hz
constexpr uint32_t DEBUG_INTERVAL_MS = 100;

constexpr float DEAD_ZONE = 0.08f;
constexpr float FILTER_ALPHA = 0.25f;

constexpr bool INVERT_X = false;
constexpr bool INVERT_Y = true;

constexpr uint32_t DEBOUNCE_MS = 30;

float centerX = 2048.0f;
float centerY = 2048.0f;

float filteredX = 0.0f;
float filteredY = 0.0f;

bool buttonStable = false;
bool buttonLastReading = false;
uint32_t buttonChangeTime = 0;

uint32_t lastSendTime = 0;
uint32_t lastDebugTime = 0;

float readAxis(int pin, float center, bool invert) {
    int raw = analogRead(pin);

    float value = (raw - center) / 2048.0f;
    value = constrain(value, -1.0f, 1.0f);

    if (fabsf(value) < DEAD_ZONE) {
        value = 0.0f;
    } else {
        // Rescale the remaining range so output still reaches full scale.
        float magnitude = (fabsf(value) - DEAD_ZONE)
                        / (1.0f - DEAD_ZONE);
        value = copysignf(magnitude, value);
    }

    if (invert) {
        value = -value;
    }

    return value;
}

void calibrateJoystick() {
    // Leave the joystick centered during startup.
    constexpr int samples = 100;
    uint32_t sumX = 0;
    uint32_t sumY = 0;

    delay(500);

    for (int i = 0; i < samples; i++) {
        sumX += analogRead(JOY_X_PIN);
        sumY += analogRead(JOY_Y_PIN);
        delay(5);
    }

    centerX = static_cast<float>(sumX) / samples;
    centerY = static_cast<float>(sumY) / samples;
}

void setup() {
    Serial.begin(USB_BAUD);

    pinMode(JOY_SW_PIN, INPUT_PULLUP);
    analogReadResolution(12);

    // Explicitly assign Serial1's RX and TX pins.
    Serial1.setPins(UART_RX_PIN, UART_TX_PIN);
    Serial1.begin(UART_BAUD);

    delay(200);

    Serial.println();
    Serial.println("Joystick transmitter starting...");
    Serial.println("Keep the joystick centered for calibration.");

    calibrateJoystick();

    Serial.printf("Center X: %.1f, Center Y: %.1f\n",
                  centerX, centerY);
    Serial.println("UART: 9600 baud, RX=D5, TX=D6");
}

void loop() {
    uint32_t now = millis();

    // Debounce the active-low joystick button.
    bool buttonReading = (digitalRead(JOY_SW_PIN) == LOW);

    if (buttonReading != buttonLastReading) {
        buttonLastReading = buttonReading;
        buttonChangeTime = now;
    }

    if (now - buttonChangeTime >= DEBOUNCE_MS) {
        buttonStable = buttonLastReading;
    }

    // Send commands at 50 Hz.
    if (now - lastSendTime >= SEND_INTERVAL_MS) {
        lastSendTime = now;

        float x = readAxis(JOY_X_PIN, centerX, INVERT_X);
        float y = readAxis(JOY_Y_PIN, centerY, INVERT_Y);

        filteredX += FILTER_ALPHA * (x - filteredX);
        filteredY += FILTER_ALPHA * (y - filteredY);

        // CSV format: X,Y,button
        Serial1.printf("%.3f,%.3f,%d\n",
                       filteredX,
                       filteredY,
                       buttonStable ? 1 : 0);
    }

    // USB diagnostics at 10 Hz.
    if (now - lastDebugTime >= DEBUG_INTERVAL_MS) {
        lastDebugTime = now;

        Serial.printf("X: %.2f  Y: %.2f  Button: %d\n",
                      filteredX,
                      filteredY,
                      buttonStable ? 1 : 0);
    }
}
 