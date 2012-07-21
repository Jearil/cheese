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
//#include "setup.h"

#define DHTPIN 2 // Using pin 2 for DHT
#define DHTTYPE DHT22 // Using a DHT22

Adafruit_CharacterOLED lcd(6, 7, 8, 9, 10, 11, 12);
DHT dht(DHTPIN, DHTTYPE);

int tempUpPin = 13;

int counter;

void setup()
{
    Serial.begin(9600);
    // LCD
    lcd.begin(16,2);
    lcd.print("Cheese-O-Matic");
    lcd.setCursor(0, 1);
    lcd.print("8000");
	//DHT
	dht.begin();
    pinMode(tempUpPin, INPUT);
    counter = 0;
}

void loop()
{
    int val = digitalRead(tempUpPin);
    // Add some profiling
    unsigned long start_time = millis();
    
    // Check temp/humidity
    // True for F
    float temp = dht.readTemperature(true);
    float humid = dht.readHumidity();
    unsigned long end_time = millis();

    if (isnan(temp) || isnan(humid)) {
        Serial.println("Failed to read from DHT");
        lcd.clear();
        lcd.home();
        lcd.print("Failed DHT read");
    }
    else {
        print_stats(temp, humid);
    }
    // How long does it take to get t/h
    long duration = end_time - start_time;

    char line2[17];
    sprintf(line2, "DEBUG: %d", duration);
    lcd.setCursor(0, 1);
    lcd.print(line2);
    if (val == HIGH) {
      counter++;
    }
      
    lcd.print(" ");
    lcd.print(counter);
    
  // Display temp/humidity
  // Check if cave should be turned on/off
  // Check for button input
}

void print_stats(float temp, float humid)
{
    if(isnan(temp) || isnan(humid)) {
        Serial.println("DHT22 sensor fail");
        lcd.home();
        lcd.print("Temp Fail");
    } else {
        print_th(temp, humid);
    }
}

void print_th(float temp, float humid)
{
    lcd.home();
    lcd.print(temp);
    lcd.print("F ");
    lcd.print(humid);
    lcd.print("\%    ");
    Serial.println(temp);
}
