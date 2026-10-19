#include <Arduino.h>
#include <math.h>

// =====================================================
// PIN CONFIGURATION
// Adafruit ESP32-S3 Reverse TFT Feather
// =====================================================
constexpr int JOY_X_PIN  = A0;
constexpr int JOY_Y_PIN  = A1;
constexpr int JOY_SW_PIN = 5;  // D5

// =====================================================
// CONFIGURATION
// =====================================================
constexpr uint32_t SAMPLE_INTERVAL_MS = 20;  // 50 Hz
constexpr uint32_t PRINT_INTERVAL_MS  = 100; // 10 Hz
constexpr uint32_t DEBOUNCE_MS        = 30;

constexpr int ADC_MAX_VALUE = 4095; // 12-bit ADC

// Joystick center measured during startup.
int centerX = 2048;
int centerY = 2048;

// Ignore small movements around joystick center.
constexpr float DEAD_ZONE = 0.08f;

// Analog smoothing: larger values respond faster.
constexpr float FILTER_ALPHA = 0.25f;

// Change these if an axis moves in the wrong direction.
constexpr bool INVERT_X = false;
constexpr bool INVERT_Y = true;

// =====================================================
// STATE VARIABLES
// =====================================================
float filteredX = 0.0f;
float filteredY = 0.0f;

uint32_t lastSampleMs = 0;
uint32_t lastPrintMs = 0;

bool buttonStable = false;
bool buttonLastReading = false;
uint32_t buttonChangeMs = 0;

// =====================================================
// CALIBRATE JOYSTICK CENTER
// Keep the joystick untouched during startup.
// =====================================================
void calibrateJoystick()
{
    constexpr int NUM_SAMPLES = 100;

    long sumX = 0;
    long sumY = 0;

    Serial.println(
        "Calibrating joystick. Leave it centered..."
    );

    for (int i = 0; i < NUM_SAMPLES; ++i)
    {
        sumX += analogRead(JOY_X_PIN);
        sumY += analogRead(JOY_Y_PIN);
        delay(5);
    }

    centerX = sumX / NUM_SAMPLES;
    centerY = sumY / NUM_SAMPLES;

    Serial.printf(
        "Center X: %d, Center Y: %d\n",
        centerX, centerY
    );
}

// =====================================================
// NORMALIZE ANALOG INPUT TO -1.0 ... +1.0
// =====================================================
float normalizeAxis(int raw, int center, bool invert)
{
    float value;

    if (raw >= center)
    {
        value = (float)(raw - center) / (4095 - center);
    }
    else
    {
        value = (float)(raw - center) / center;
    }

    if (invert)
    {
        value = -value;
    }

    // Clamp to the valid output range.
    if (value > 1.0f) value = 1.0f;
    if (value < -1.0f) value = -1.0f;

    // Ignore small movements around the center.
    const float deadZone = 0.08f;

    if (fabs(value) < deadZone)
    {
        value = 0.0f;
    }
    else
    {
        // Rescale so output smoothly spans 0 to 1 outside the dead zone.
        value = (value > 0)
            ? (value - deadZone) / (1.0f - deadZone)
            : (value + deadZone) / (1.0f - deadZone);
    }

    return value;
}

// =====================================================
// READ AND DEBOUNCE JOYSTICK BUTTON
// INPUT_PULLUP means pressed = LOW.
// =====================================================
void updateButton()
{
    bool reading = (digitalRead(JOY_SW_PIN) == LOW);
    uint32_t now = millis();

    if (reading != buttonLastReading)
    {
        buttonLastReading = reading;
        buttonChangeMs = now;
    }

    if ((now - buttonChangeMs) >= DEBOUNCE_MS)
    {
        buttonStable = reading;
    }
}

// =====================================================
// SETUP
// =====================================================
void setup()
{
    Serial.begin(115200);

    // Allow time for USB serial to initialize.
    delay(500);

    analogReadResolution(12);

    pinMode(JOY_SW_PIN, INPUT_PULLUP);

    delay(300);

    calibrateJoystick();

    lastSampleMs = millis();
    lastPrintMs = millis();

    Serial.println("Joystick controller ready.");
    Serial.println("Output: x,y,button");
}

// =====================================================
// MAIN LOOP
// =====================================================
void loop()
{
    uint32_t now = millis();

    // Sample joystick at approximately 50 Hz.
    if (now - lastSampleMs >= SAMPLE_INTERVAL_MS)
    {
        lastSampleMs = now;

        int rawX = analogRead(JOY_X_PIN);
        int rawY = analogRead(JOY_Y_PIN);

        float targetX =
            normalizeAxis(rawX, centerX, INVERT_X);

        float targetY =
            normalizeAxis(rawY, centerY, INVERT_Y);

        // Smooth the readings.
        filteredX +=
            FILTER_ALPHA * (targetX - filteredX);

        filteredY +=
            FILTER_ALPHA * (targetY - filteredY);

        updateButton();

        // Save the latest raw readings for printing.
        static int latestRawX = 0;
        static int latestRawY = 0;

        latestRawX = rawX;
        latestRawY = rawY;
    }

    // Print at approximately 10 Hz.
    if (now - lastPrintMs >= PRINT_INTERVAL_MS)
    {
        lastPrintMs = now;

        // Read current raw values for diagnostic output.
        int rawX = analogRead(JOY_X_PIN);
        int rawY = analogRead(JOY_Y_PIN);

        Serial.printf(
            "Raw X: %d, Raw Y: %d | x=%.3f, y=%.3f, button=%d\n",
            rawX,
            rawY,
            filteredX,
            filteredY,
            buttonStable ? 1 : 0
        );
    }
}