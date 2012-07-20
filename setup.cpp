#include <Adafruit_CharacterOLED.h>
#include <DHT.h>

void setup_lcd(Adafruit_CharacterOLED lcd)
{
    // LCD
    lcd.begin(16,2);
    lcd.print("Cheese-O-Matic");
    lcd.setCursor(0, 1);
    lcd.print("8000");
}

void setup_dht(DHT dht)
{
    dht.begin();
}
