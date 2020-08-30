// Controller for dry ager

#include <Wire.h>
#include <Adafruit_BMP280.h>

#define RELAY_PIN       (3)
#define SAFETY          (-10) // if sensor is disconnected, we want to default to ON // no sensor reads ~ -147
#define LOWER_THRESHOLD (0) // (0) Celsius 32F // use (30) to test at room temp
#define UPPER_THRESHOLD (4) // (4) Celsius 39F // use (50) to test at room temp
#define UPDATE_DELAY  (2000) // get new temp every 2 seconds

#define PROD_BUILD
#ifdef PROD_BUILD
// make sure we don't turn on more often than every 5 minutes
#define DELAY           (5 * 1000 * 60)
#else
#define DELAY           (1000)
#endif

#define MEGA
#define I2C

// Mega
#ifdef MEGA
#define BMP_SCK   (52)
#define BMP_MISO  (50)
#define BMP_MOSI  (51)
#define BMP_CS    (53)
#else
#define BMP_SCK  (14)
#define BMP_MISO (12)
#define BMP_MOSI (13)
#define BMP_CS   (15)
#endif

#ifdef I2C
Adafruit_BMP280 bmp; // I2C
#else
// Adafruit_BMP280 bmp(BMP_CS, BMP_MOSI, BMP_MISO, BMP_SCK);
#endif

// if we just turned on, make sure we don't start the compressor right away
long nextActionTimer = millis() + DELAY;
bool ledOn = true; // turn LED on when we start
bool sensorHasEverDisconnected = false;

void setup() {
//  randomSeed(analogRead(0));
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(LED_BUILTIN, HIGH);
  
  Serial.begin(9600);
  while (!Serial);
  Serial.println(F("drymon dry ager monitor v0.2"));

  if (!bmp.begin()) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}

void blink() {
  if (sensorHasEverDisconnected) {
    ledOn = !ledOn;
    digitalWrite(LED_BUILTIN, ledOn);
  }
}

void loop() {
  // TODO: should we declare the variable here?
  float temp = bmp.readTemperature();
//  temp = random(-4,9);

  // we delay here as there are 2 conditions below where we need to delay, and it's fine to do it up here
  delay(UPDATE_DELAY);

  blink();

  Serial.println(temp);

  if (millis() > nextActionTimer) {
    nextActionTimer = millis() + DELAY;
    if (temp < SAFETY) {
      Serial.println(F("temp less than safety threshold"));
      sensorHasEverDisconnected = true;
    }
    if (sensorHasEverDisconnected) {
      Serial.println(F("sensor has been disconnected"));
      // failsafe mode, make sure ager is ON
      digitalWrite(RELAY_PIN, LOW);
      return;
    }
    if (temp > UPPER_THRESHOLD) {
      Serial.println(F("temp above 4C -- turn on ager"));
      digitalWrite(RELAY_PIN, LOW);
      digitalWrite(LED_BUILTIN, HIGH);
    }
    if ((temp >= SAFETY) && (temp < LOWER_THRESHOLD)) {
      Serial.println(F("temp below 0C -- turn off ager"));
      digitalWrite(RELAY_PIN, HIGH);
      digitalWrite(LED_BUILTIN, LOW);
    }
  }
  // TODO: post the temp to the server
}
