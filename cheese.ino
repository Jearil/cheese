/**
  Cheese-O-Matic 8000 Control logic

  Required libraries:
  DHT: https://github.com/adafruit/DHT-sensor-library
  LCD: https://github.com/ladyada/Adafruit_CharacterOLED
  
  LCD circuit:
  * LCD RS pin to digital pin 5
  * LCD R/W pin to digital pin 6
  * LCD Enable pin to digital pin 7
  * LCD D4 pin to digital pin 8
  * LCD D5 pin to digital pin 9
  * LCD D6 pin to digital pin 10
  * LCD D7 pin to digital pin 11

  DHT22 (Humidity/Temperature) circuit:
  * DHT22 pin 1 to +5V
  * DHT22 pin 2 to digital pin 4
  * DHT22 pin 4 to GROUND
  * 10K resistor from pin 2 to pin 1 of sensor

  Copyright (C) 2012 Colin Miller
 */
#include <stdio.h>
#include <Adafruit_CharacterOLED.h>
#include <DHT.h>``

#define DHTPIN 4 // Using pin 5 for DHT
#define DHTTYPE DHT22 // Using a DHT22

unsigned long COIL_MIN_REST = 600000l;

// Buttons
#define TEMP_UP_PIN 2
#define TEMP_DOWN_PIN 3
#define HUMID_UP_PIN 0
#define HUMID_DOWN_PIN 0

// Freezer power
#define COOLING_PIN 12

Adafruit_CharacterOLED lcd(5, 6, 7, 8, 9, 10, 11);
DHT dht(DHTPIN, DHTTYPE);

// Length of time we want any custom text to stay for.
unsigned long text_hang_time = 10000;
unsigned long custom_text_time;
unsigned long alternate_time_on;
boolean display_temp;

float target_temp;
float target_humid;

boolean first_run;

// 15 minutes min wait for cooldown
unsigned long cooling_off_time;
float temp_band;

boolean cooling;
unsigned long rollovers; // number of rollovers
unsigned long start_time;

void setup()
{
    first_run = true;
    Serial.begin(9600);
    Serial.println("Cheese-O-Matic");
    // LCD
    Serial.println("Starting LCD");
    lcd.begin(16,2);
    Serial.println("LCD Set");
    lcd.print("Cheese-O-Matic");
    lcd.setCursor(0, 1);
    lcd.print("8000");
    //DHT
    dht.begin();
    pinMode(TEMP_UP_PIN, INPUT);
    pinMode(TEMP_DOWN_PIN, INPUT);
    pinMode(HUMID_UP_PIN, INPUT);
    pinMode(HUMID_DOWN_PIN, INPUT);
    pinMode(COOLING_PIN, OUTPUT);
    
    // Freezer should be able to be turned on 5 min after boot

    cooling_off_time = millis();
    target_temp = 55;
    target_humid = 85;
    temp_band = 2.5;
    cooling = false;
    custom_text_time = millis();
    alternate_time_on = millis();
    start_time = millis();
    display_temp = true;
    Serial.println("Finished Setup");
}

void loop()
{
    Serial.println("Starting loop");
    // check for rollover
    unsigned long last_start = start_time;
    start_time = millis();
    if (start_time < last_start) {
      cooling_off_time = start_time;
      custom_text_time = start_time;
      alternate_time_on = start_time;
      rollovers++;
    }
    
    // Check temp/humidity
    // True for F
    float temp = dht.readTemperature(true);
    float humid = dht.readHumidity();
    unsigned long end_time = millis();
    
    // How long does it take to get t/h
    long duration = end_time - start_time;
    
    // print display
    Serial.println("Printing stats");
    print_stats(temp, humid, duration);

    lcd.setCursor(0, 1);
    
    // Check if cave should be turned on/off
    adjust_temp(temp);
    
    // Check for button input
    check_buttons();
    
    // Clear custom text
    clear_custom_text();
    Serial.print("Target temp: ");
    Serial.println(target_temp);
}

void print_stats(float temp, float humid, unsigned long duration)
{
    if(isnan(temp) || isnan(humid)) {
        Serial.println("DHT22 sensor fail");
        lcd.home();
        lcd.print("Temp Fail");
    } else {
        print_th(temp, humid);
        Serial.print("Getting temp took: ");
        Serial.print(duration);
        Serial.println("ms\n");
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

void check_buttons()
{
    int temp_up_button = digitalRead(TEMP_UP_PIN);
    int temp_down_button = digitalRead(TEMP_DOWN_PIN);
    int humid_up_button = digitalRead(HUMID_UP_PIN);
    int humid_down_button = digitalRead(HUMID_DOWN_PIN);
    if (temp_up_button == HIGH) {
        Serial.println("Raising temp");
        target_temp++;
        display_target_temp();
        custom_text_time = millis();
    }
    if (temp_down_button == HIGH) {
        target_temp--;
    }
    if (humid_up_button == HIGH) {
        target_humid++;
    }
    if (humid_down_button == HIGH) {
        target_humid--;
    }
}

void adjust_temp(float temp)
{
    Serial.println("Adjusting temp");
   if(cooling) {
       Serial.println("Still cooling");
       cooling_shutoff(temp);
   } else {
       float target_high = target_temp + temp_band;
       if(temp >= target_high) {
           start_cooling();
       }
   }
}

/* Start cooling down the cave assuming cooling timeoff
 * is passed the min time of coil rest. We don't want
 * the coils to be overexerted.
*/
void start_cooling()
{
    unsigned long time_passed = millis() - cooling_off_time;
    if (time_passed >= COIL_MIN_REST || first_run) {
        first_run = false;
        // Turn freezer pin on
        digitalWrite(COOLING_PIN, HIGH);
        cooling = true;
        lcd.print(">>Now Cooling<<");
        custom_text_time = millis();
        Serial.println("Starting cooldown");
    } else {
       unsigned long remaining = COIL_MIN_REST - time_passed;
       Serial.println("Can't cool, Coils resting");
       Serial.print("Time Remaining: ");
       Serial.println(remaining);
       Serial.print("Time passed: ");
       Serial.println(time_passed);
       Serial.print("COIL_MIN_REST: ");
       Serial.println(COIL_MIN_REST);
       Serial.print("Cooling off time: ");
       Serial.println(cooling_off_time);
    }
}

// Check to see if we need to shut off the freezer
void cooling_shutoff(float temp)
{
    float target_bottom = target_temp - temp_band;
    if(temp <= target_bottom) {
        // Turn freezer pin off
        Serial.println("Shutting off freezer");
        Serial.print("Current temp: ");
        Serial.println(temp);
        Serial.print("Target temp: ");
        Serial.println(target_bottom);
        digitalWrite(COOLING_PIN, LOW);
        cooling = false;
        cooling_off_time = millis();
        lcd.print(">>Cooling Done<<");
        custom_text_time = millis();
    }
}

/* Clear any custom text that's going on and display
 * default data. If cooling, alternate display with
 * cooling info.
 */
 void clear_custom_text()
 {
     unsigned long custom_duration = millis() - custom_text_time;
     if (custom_duration >= text_hang_time) {
         // now we can clear text
         if(cooling) {
             if(display_temp) {
                 display_target_temp();
                 if(millis() - alternate_time_on >= text_hang_time) {
                     display_temp = false;
                     alternate_time_on = millis();
                 }
             } else {
                 lcd.print(">>Now Cooling<<");
                 if (millis() - alternate_time_on > text_hang_time) {
                     display_temp = true; 
                     alternate_time_on = millis();
                 }
             }
         } else {
             display_target_temp();
         }
     }
 }
 
 void display_target_temp()
 {
     lcd.print("Target: ");
     lcd.print(target_temp);
     lcd.print("F      ");
 }
