#include <EEPROM.h>

constexpr int LED_PIN_FIRST = 8;
constexpr int LED_PIN_COUNT = 4;

constexpr int BTN_INPUT_A = 2;
constexpr int BTN_INPUT_B = 3;
constexpr int BTN_OUTPUT_Y1 = 7;
constexpr int BTN_OUTPUT_Y2 = 8;

uint8_t leds = 0;
int buttons = 0;
int previous_buttons = 0;
unsigned long long last_ms = 0;

void setup()
{
  // Init serial communication
  Serial.begin(9600);

  // Select button
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);

  // Button read
  pinMode(7, INPUT);

  // Leds
  pinMode(8,  OUTPUT);
  pinMode(9,  OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);

  // Read startup value from EEPROM
  leds = EEPROM.read(0);
}

void loop()
{
  int current_buttons = read_buttons();
  if (current_buttons != previous_buttons) {
    last_ms = millis();
  }

  if (millis() - last_ms > 50) {
    if (current_buttons != buttons) {
      buttons = current_buttons;

      if (buttons & 1 << 0 && buttons & 1 << 1) {
        // both button 0 and 1
        // toggle led 1
        leds ^= 1 << 1;
      } else if (buttons & 1 << 0) {
        // button 0
        // toggle led 0
        leds ^= 1 << 0;
      } else if (buttons & 1 << 2 && buttons & 1 << 3) {
        // both button 2 and 3
        reset_leds();
      } else if (buttons & 1 << 2) {
        // button 2
        // shift leds right
        leds >>= 1;
      } else if (buttons & 1 << 3) {
        // button 3
        // shift leds left
        leds <<= 1;
        leds &= 0xf;
      }
    }
  }

  previous_buttons = current_buttons;
  write_leds(LED_PIN_FIRST, LED_PIN_COUNT, leds);
}

void write_leds(int first, int count, int value)
{
  for (int i = 0; i < count; ++i) {
    // Write each bit of `value' to leds
    digitalWrite(first+i, (value >> i) & 1);
  }
}

// Read from 74HC352N
// input A and B
// output Y
int read_buttons()
{
  int state = 0;
  for (int i = 0; i < 4; ++i) {
    digitalWrite(BTN_INPUT_A, i & 1);
    digitalWrite(BTN_INPUT_B, (i >> 1) & 1);
    state |= digitalRead(BTN_OUTPUT_Y1) << i;
    //state |= digitalRead(BTN_OUTPUT_Y2) << i;
  }
  return state;
}

void reset_leds()
{
  // Save leds
  EEPROM.write(0, leds);

  for (int i = 0; i < 4; ++i) {
    // Blink leds 4 times
    write_leds(LED_PIN_FIRST, LED_PIN_COUNT, 0);
    delay(50);
    write_leds(LED_PIN_FIRST, LED_PIN_COUNT, -1);
    delay(50);
  }

  // Set leds to all off
  leds = 0;
  write_leds(LED_PIN_FIRST, LED_PIN_COUNT, leds);
}
