/**
  Cheese-O-Matic 8000 Control logic

  Required libraries:
  DHT: https://github.com/adafruit/DHT-sensor-library
  LCD: https://github.com/ladyada/Adafruit_CharacterOLED
  
  LCD circuit:
  * LCD RS pin to digital pin 6
  * LCD R/W pin to digital pin 7
  * LCD Enable pin to digital pin 8
  * LCD D4 pin to digital pin 9
  * LCD D5 pin to digital pin 10
  * LCD D6 pin to digital pin 11
  * LCD D7 pin to digital pin 12

  DHT22 (Humidity/Temperature) circuit:
  * DHT22 pin 1 to +5V
  * DHT22 pin 2 to digital pin 2
  * DHT22 pin 4 to GROUND
  * 10K resistor from pin 2 to pin 1 of sensor

  Copyright (C) 2012 Colin Miller
 */
#include <stdio.h>
#include <Adafruit_CharacterOLED.h>
#include <DHT.h>
#include "setup.h"

#define DHTPIN 2 // Using pin 2 for DHT
#define DHTTYPE DHT22 // Using a DHT22

Adafruit_CharacterOLED lcd(6, 7, 8, 9, 10, 11, 12);
DHT dht(DHTPIN, DHTTYPE);

void setup()
{
    setup_lcd(lcd);
    setup_dht(dht);
}

void loop()
{
    // Add some profiling
    unsigned long start_time = millis();
    // True for F
    float temp = dht.readTemperature(true);
    float humid = dht.readHumidity();
    unsigned long end_time = millis();
    long duration = end_time - start_time;

    print_stats(temp, humid);
    char line2[17];
    sprintf(line2, "DEBUG: %d", duration);
    lcd.setCursor(0, 1);
    lcd.print(line2);

  // Check for temp/humidity
  // Display temp/humidity
  // Check if cave should be turned on/off
  // Check for button input
}

void print_stats(float temp, float humid)
{
    lcd.clear();
    lcd.home();
    char line1[17];
    sprintf(line1, "%dF %d%", temp, humid);
    lcd.print(line1);
}
