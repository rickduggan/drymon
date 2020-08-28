// Controller for dry ager

#include <Wire.h>
#include <Adafruit_BMP280.h>

#define RELAY_PIN       (3)
#define LOWER_THRESHOLD (0) // Celsius
#define UPPER_THRESHOLD (4) // Celsius
// make sure we don't turn on more often than every 5 minutes
// #define PROD_BUILD

#ifdef PROD_BUILD
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

// if we just turned on, make sure we don't start the compressor right away
long nextActionTimer = millis() + DELAY;

#ifdef I2C
Adafruit_BMP280 bmp; // I2C
#else
// Adafruit_BMP280 bmp(BMP_CS, BMP_MOSI, BMP_MISO, BMP_SCK);
#endif

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println(F("drymon dry ager monitor"));

  if (!bmp.begin()) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }

  pinMode(RELAY_PIN, OUTPUT);

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}

void loop() {
  // TODO: should we declare the variable here?
  float temp = bmp.readTemperature();

  Serial.println(temp);
  if (millis() > nextActionTimer) {
    if (temp > UPPER_THRESHOLD) {
      Serial.println("temp above 4C -- turn on ager");
      digitalWrite(RELAY_PIN, LOW);
    } 
    if (temp < LOWER_THRESHOLD) {
      Serial.println("temp below 0C -- turn off ager");
      digitalWrite(RELAY_PIN, HIGH);
    }
    nextActionTimer = millis() + DELAY;
  } 
  delay(2000);
  // TODO: post the temp to the server 
}
