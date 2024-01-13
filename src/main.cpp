#include <Arduino.h>
#include <Button.h>

const uint8_t BRIGHTNESS_BTN_PIN = 2;
const uint8_t COLOR_BTN_PIN = 3;
const uint8_t WARM_LEDS_PIN = 5;
const uint8_t WHITE_LEDS_PIN = 6;

const uint8_t COLOR_MODE_0_FULL_WARM = 0b0010;
const uint8_t COLOR_MODE_1_FULL_WARM_HALF_WHITE = 0b0110;
const uint8_t COLOR_MODE_2_FULL_WARM_FULL_WHITE = 0b1010;
const uint8_t COLOR_MODE_3_HALF_WARM_FULL_WHITE = 0b1001;
const uint8_t COLOR_MODE_4_FULL_WHITE = 0b1000;
const uint8_t MAX_COLOR_MODE = 5;
const uint8_t COLOR_MODES[MAX_COLOR_MODE] = {COLOR_MODE_0_FULL_WARM, COLOR_MODE_1_FULL_WARM_HALF_WHITE,
                                             COLOR_MODE_2_FULL_WARM_FULL_WHITE, COLOR_MODE_3_HALF_WARM_FULL_WHITE,
                                             COLOR_MODE_4_FULL_WHITE};

Button brightnessBtn(BRIGHTNESS_BTN_PIN);
Button colorBtn(COLOR_BTN_PIN);

const uint16_t BRIGHTNESS_STEP = 255 / 5;
const uint16_t BRIGHTNESS_MAX = 255;
uint16_t ledBrightnessTarget = 0;

uint8_t colorModeIndex = 2;
float warmLedsMultiplier = 1;
float whiteLedsMultiplier = 1;

uint16_t warmLedsTarget = 0;
uint16_t warmLedsCurr = 0;
uint16_t whiteLedsTarget = 0;
uint16_t whiteLedsCurr = 0;

extern const uint8_t gamma8[];

void updateLEDs();
int16_t calculateChange(uint16_t current, uint16_t target);

void setup() {
  Serial.begin(9600);
  Serial.println("Nano lamp controller");

  pinMode(WARM_LEDS_PIN, OUTPUT);
  analogWrite(WARM_LEDS_PIN, 0);
  pinMode(WHITE_LEDS_PIN, OUTPUT);
  analogWrite(WHITE_LEDS_PIN, 0);

  brightnessBtn.begin();
  colorBtn.begin();
}

void loop() {
  if (brightnessBtn.pressed()) {
    if (ledBrightnessTarget >= BRIGHTNESS_MAX) {
      ledBrightnessTarget = 0;
    } else {
      ledBrightnessTarget += BRIGHTNESS_STEP;
    }

    warmLedsTarget = ledBrightnessTarget;
    whiteLedsTarget = ledBrightnessTarget;

    Serial.print("Change brightness to ");
    Serial.println(ledBrightnessTarget);
  }

  if (colorBtn.pressed()) {
    if (colorModeIndex >= MAX_COLOR_MODE - 1) {
      colorModeIndex = 0;
    } else {
      colorModeIndex++;
    }

    Serial.print("Change color mode to ");
    Serial.println(colorModeIndex);
  }

  updateLEDs();
  delay(10);
}

void updateLEDs() {
  if (COLOR_MODES[colorModeIndex] & 0b0010) {
    warmLedsMultiplier = 1;
  } else if (COLOR_MODES[colorModeIndex] & 0b0001) {
    warmLedsMultiplier = 0.5;
  } else {
    warmLedsMultiplier = 0;
  }
  if (COLOR_MODES[colorModeIndex] & 0b1000) {
    whiteLedsMultiplier = 1;
  } else if (COLOR_MODES[colorModeIndex] & 0b0100) {
    whiteLedsMultiplier = 0.5;
  } else {
    whiteLedsMultiplier = 0;
  }
  const uint16_t realWarmLedsTarget = warmLedsTarget * warmLedsMultiplier;
  if (warmLedsCurr != realWarmLedsTarget) {
    // Serial.print("Change warm LEDs to ");
    warmLedsCurr += calculateChange(warmLedsCurr, realWarmLedsTarget);
    // Serial.println(warmLedsCurr);
    analogWrite(WARM_LEDS_PIN, pgm_read_byte(&gamma8[warmLedsCurr]));
  }
  const uint16_t realWhiteLedsTarget = whiteLedsTarget * whiteLedsMultiplier;
  if (whiteLedsCurr != realWhiteLedsTarget) {
    // Serial.print("Change white LEDs to ");
    whiteLedsCurr += calculateChange(whiteLedsCurr, realWhiteLedsTarget);
    // Serial.println(whiteLedsCurr);
    analogWrite(WHITE_LEDS_PIN, pgm_read_byte(&gamma8[whiteLedsCurr]));
  }
}

int16_t calculateChange(uint16_t current, uint16_t target) {
  const int16_t diff = target - current;
  const uint16_t absDiff = abs(diff);
  int16_t changeBy;
  if (absDiff > 100) {
    changeBy = 10;
  } else if (absDiff > 50) {
    changeBy = 5;
  } else if (absDiff > 10) {
    changeBy = 1;
  }
  return diff > 0 ? changeBy : -changeBy;
}

// https://learn.adafruit.com/led-tricks-gamma-correction/the-quick-fix
const uint8_t PROGMEM gamma8[] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   2,   2,   2,
    2,   2,   2,   2,   2,   3,   3,   3,   3,   3,   3,   3,   4,   4,   4,   4,   4,   5,   5,   5,   5,   6,
    6,   6,   6,   7,   7,   7,   7,   8,   8,   8,   9,   9,   9,   10,  10,  10,  11,  11,  11,  12,  12,  13,
    13,  13,  14,  14,  15,  15,  16,  16,  17,  17,  18,  18,  19,  19,  20,  20,  21,  21,  22,  22,  23,  24,
    24,  25,  25,  26,  27,  27,  28,  29,  29,  30,  31,  32,  32,  33,  34,  35,  35,  36,  37,  38,  39,  39,
    40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  50,  51,  52,  54,  55,  56,  57,  58,  59,  60,  61,
    62,  63,  64,  66,  67,  68,  69,  70,  72,  73,  74,  75,  77,  78,  79,  81,  82,  83,  85,  86,  87,  89,
    90,  92,  93,  95,  96,  98,  99,  101, 102, 104, 105, 107, 109, 110, 112, 114, 115, 117, 119, 120, 122, 124,
    126, 127, 129, 131, 133, 135, 137, 138, 140, 142, 144, 146, 148, 150, 152, 154, 156, 158, 160, 162, 164, 167,
    169, 171, 173, 175, 177, 180, 182, 184, 186, 189, 191, 193, 196, 198, 200, 203, 205, 208, 210, 213, 215, 218,
    220, 223, 225, 228, 231, 233, 236, 239, 241, 244, 247, 249, 252, 255};
