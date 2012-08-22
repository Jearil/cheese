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

// Some statistics
unsigned long cooling_start;
unsigned long last_cooling_duration;

void setup()
{
    // Allows the freezer to be turned on when we first
    // plug it in
    first_run = true;
    Serial.begin(9600);
    Serial.println("Cheese-O-Matic");
    // LCD
    Serial.println("Starting LCD");
    lcd.begin(16,2);
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
    
    cooling_off_time = millis();
    target_temp = 55;
    target_humid = 85;
    temp_band = 2.0;
    cooling = false;
    custom_text_time = millis();
    alternate_time_on = millis();
    start_time = millis();
    display_temp = true;
    last_cooling_duration = 0;
    Serial.println("Finished Setup");
}

void loop()
{
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
    print_stats(temp, humid, duration);

    lcd.setCursor(0, 1);
    
    // Check if cave should be turned on/off
    adjust_temp(temp);
    
    // Check for button input
    check_buttons();
    
    // Clear custom text
    clear_custom_text();
}

void print_stats(float temp, float humid, unsigned long duration)
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
}

void check_buttons()
{
    int temp_up_button = digitalRead(TEMP_UP_PIN);
    int temp_down_button = digitalRead(TEMP_DOWN_PIN);
    int humid_up_button = digitalRead(HUMID_UP_PIN);
    int humid_down_button = digitalRead(HUMID_DOWN_PIN);
    if (temp_up_button == HIGH) {
        target_temp++;
        display_target_temp();
        custom_text_time = millis();
    }
    if (temp_down_button == HIGH) {
        target_temp--;
        display_target_temp();
        custom_text_time = millis();
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
   if(cooling) {
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
        cooling_start = millis();
        custom_text_time = millis();
        Serial.println("Starting cooldown");
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
        last_cooling_duration = cooling_off_time - cooling_start;
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
                text_switch_check();
            } else {
                lcd.print(">>Now Cooling<<");
                text_switch_check();
            }
        } else if(display_temp) {
            display_target_temp();
            text_switch_check();
        } else {
            display_last_duration();
            text_switch_check();
        }
    }
}


boolean text_switch_check()
{
    if (millis() - alternate_time_on >= text_hang_time) {
        alternate_time_on = millis();
        return !display_temp;
    }
    return display_temp;
}

void display_target_temp()
{
    lcd.print("Target: ");
    lcd.print(target_temp);
    lcd.print("F      ");
}

void display_last_duration()
{
    lcd.print("LCool: ");
    unsigned long duration_seconds = last_cooling_duration / 60;
    lcd.print(duration_seconds);
    lcd.print("S        ");
}
