#include <Arduino.h>
#include <math.h>

// Joystick inputs
constexpr int JOY_X_PIN  = A0;
constexpr int JOY_Y_PIN  = A1;
constexpr int JOY_SW_PIN = 9;

// UART connection to motor board
constexpr int UART_RX_PIN = 5;
constexpr int UART_TX_PIN = 6;

constexpr uint32_t USB_BAUD  = 115200;
constexpr uint32_t UART_BAUD = 9600;

constexpr uint32_t SEND_INTERVAL_MS  = 50;
constexpr uint32_t DEBUG_INTERVAL_MS = 100;
constexpr uint32_t DEBOUNCE_MS       = 30;

constexpr float DEAD_ZONE = 0.08f;
constexpr float FILTER_ALPHA = 0.25f;

constexpr bool INVERT_X = false;
constexpr bool INVERT_Y = true;

// Measured joystick center values
constexpr float CENTER_X = 1818.0f;
constexpr float CENTER_Y = 1836.0f;

float filteredX = 0.0f;
float filteredY = 0.0f;

int latestRawX = 0;
int latestRawY = 0;

bool buttonStable = false;
bool buttonLastReading = false;
uint32_t buttonChangeTime = 0;

uint32_t lastSendTime = 0;
uint32_t lastDebugTime = 0;

float normalizeAxis(int raw, float center, bool invert) {
    float value;

    if (raw >= center) {
        value = (raw - center) / (4095.0f - center);
    } else {
        value = (raw - center) / center;
    }

    value = constrain(value, -1.0f, 1.0f);

    if (fabsf(value) < DEAD_ZONE) {
        value = 0.0f;
    } else {
        float magnitude =
            (fabsf(value) - DEAD_ZONE) / (1.0f - DEAD_ZONE);

        value = copysignf(magnitude, value);
    }

    if (invert) {
        value = -value;
    }

    return value;
}

void setup() {
    Serial.begin(USB_BAUD);

    pinMode(JOY_SW_PIN, INPUT_PULLUP);
    analogReadResolution(12);

    Serial1.begin(
        UART_BAUD,
        SERIAL_8N1,
        UART_RX_PIN,
        UART_TX_PIN
    );

    delay(1000);

    Serial.println();
    Serial.println("Joystick transmitter ready.");
    Serial.println("Using fixed measured center values.");

    Serial.printf(
        "Center X: %.1f, Center Y: %.1f\n",
        CENTER_X,
        CENTER_Y
    );

    Serial.println("UART: 9600 baud, RX=D5, TX=D6");
    Serial.println("Joystick: X=A0, Y=A1, SW=D9");

    lastSendTime = millis();
    lastDebugTime = millis();
}

void loop() {
    uint32_t now = millis();

    // Debounce the active-low button.
    bool buttonReading = (digitalRead(JOY_SW_PIN) == LOW);

    if (buttonReading != buttonLastReading) {
        buttonLastReading = buttonReading;
        buttonChangeTime = now;
    }

    if (now - buttonChangeTime >= DEBOUNCE_MS) {
        buttonStable = buttonLastReading;
    }

    // Read and send joystick commands at 20 Hz.
    if (now - lastSendTime >= SEND_INTERVAL_MS) {
        lastSendTime = now;

        latestRawX = analogRead(JOY_X_PIN);
        latestRawY = analogRead(JOY_Y_PIN);

        float x = normalizeAxis(
            latestRawX, CENTER_X, INVERT_X
        );

        float y = normalizeAxis(
            latestRawY, CENTER_Y, INVERT_Y
        );

        filteredX += FILTER_ALPHA * (x - filteredX);
        filteredY += FILTER_ALPHA * (y - filteredY);

        // Allow filtered values to settle completely to zero.
        if (x == 0.0f && fabsf(filteredX) < 0.01f) {
            filteredX = 0.0f;
        }

        if (y == 0.0f && fabsf(filteredY) < 0.01f) {
            filteredY = 0.0f;
        }

        // Send X,Y,button to the motor board.
        Serial1.printf(
            "%.3f,%.3f,%d\n",
            filteredX,
            filteredY,
            buttonStable ? 1 : 0
        );
    }

    // USB diagnostics
    if (now - lastDebugTime >= DEBUG_INTERVAL_MS) {
        lastDebugTime = now;

        Serial.printf(
            "Raw X=%d Center X=%.1f | "
            "Raw Y=%d Center Y=%.1f | "
            "X=%.3f Y=%.3f Button=%d\n",
            latestRawX,
            CENTER_X,
            latestRawY,
            CENTER_Y,
            filteredX,
            filteredY,
            buttonStable ? 1 : 0
        );
    }
}
 