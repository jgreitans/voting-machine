#include <Pushbutton.h>

#define LED           9 // Moteinos have LEDs on D9
#define GREEN_PIN     4
#define YELLOW_PIN    5
#define RED_PIN       6
#define SERIAL_BAUD   115200

const unsigned long blinkIntervalMillis = 3000;
unsigned long lastBlinkTime = 0;

long greenCounter = 0, yellowCounter = 0, redCounter = 0;
Pushbutton green(GREEN_PIN), yellow(YELLOW_PIN), red(RED_PIN);

void Blink(byte pin, int durationMillis);
void blinkPeriodically();
void indicateActivity();
void handleInput();
void reset();
bool checkButtons();
void sendCounters();

void setup() {
  Serial.begin(SERIAL_BAUD);

  indicateActivity();
}

void loop() {
  blinkPeriodically();
  handleInput();
  if (checkButtons()) {
    sendCounters();
  }
}

void handleInput() {
  if (Serial.available() > 0)
  {
    char input = Serial.read();
    switch (input) {
      case 'r':
        reset();
        indicateActivity();
        break;
      default:
        // ignore
        break;
    }

    // read the rest of serial input and discard.
    while(Serial.available() > 0) {
      Serial.read();
    }
  }
}

void reset() {
  redCounter = 0;
  greenCounter = 0;
  yellowCounter = 0;
  lastBlinkTime = 0;
}

void Blink(int durationMillis)
{
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  delay(durationMillis);
  digitalWrite(LED, LOW);
}

void blinkPeriodically() {
  // overflow resistant due to how binary arithmetic works
  if (millis() - lastBlinkTime > blinkIntervalMillis) {
    Blink(100);
    lastBlinkTime = millis();
  }
}

void indicateActivity() {
  Blink(200); delay(200); Blink(200); delay(200); Blink(200);
}

bool checkButtons() {
  bool changed = false;
  if (green.getSingleDebouncedPress()) {
    greenCounter++;
    changed = true;
  }
  if (yellow.getSingleDebouncedPress()) {
    yellowCounter++;
    changed = true;
  }
  if (red.getSingleDebouncedPress()) {
    redCounter++;
    changed = true;
  }
  return changed;
}

void sendCounters() {
  Serial.print(greenCounter);
  Serial.print(',');
  Serial.print(yellowCounter);
  Serial.print(',');
  Serial.print(redCounter);
  Serial.print(',');
  Serial.println();
}

