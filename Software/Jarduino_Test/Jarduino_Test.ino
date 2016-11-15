/*******************************************************************/
/*************** The Jarduino Test Program *************************/
/*******************************************************************/

#include <Wire.h>
#include <Time.h>
#include <dht11.h>
#include <avr/wdt.h>
#include <DS1307RTC.h>
#include <LiquidCrystal_I2C.h>

#define PIN_LUX A5        // LUX pin
#define BACKLIGHT_PIN 3   // LCD Screen Light
#define I2C_ADDR 0x27     // LCD Screen Adress
#define PIN_RELAY1 4      // Relais de commande pompe
#define PIN_RELAY2 9      // Relais de commande lumière
#define DHT11PIN 13       // Pin du capteur d'humidité / temperature

LiquidCrystal_I2C  lcd(I2C_ADDR, 16, 2);// LCD creation
dht11 DHT11;                            // Creation de l'entité DHT11
tmElements_t tm,tm2;                    // Horloge + Horloge comparaison
int TempAndHum;                         // Température + humidité
byte LuxPercent,Humidity,Temperature;
const char *monthName[12] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
int TimeBeetwenTests = 3000;

byte water[8] = 
{
  B00100,
  B01110,
  B01110,
  B11111,
  B11101,
  B11111,
  B01110,
  B00000,
};  
byte humidity[8] = 
{
  B01110,
  B11111,
  B01110,
  B10000,
  B00101,
  B10000,
  B00001,
  B00000,
};
byte light[8] = 
{
  B00010,
  B00100,
  B01100,
  B11111,
  B00110,
  B00100,
  B01000,
  B00000,
};  
byte temp[8] = 
{
  B00100,
  B01010,
  B01010,
  B01110,
  B11111,
  B11111,
  B01110,
};


void setup() {
  Wire.begin();
  RTC.read(tm);
  pinMode(PIN_RELAY1,OUTPUT);
  pinMode(PIN_RELAY2,OUTPUT);
  //--------------------------------------------------------------------
  lcd_initialisation();
  lcd.createChar(0, water);
  lcd.createChar(1, light);
  lcd.createChar(2, humidity);
  lcd.createChar(3, temp);
  lcd.clear();
  //--------------------------------------------------------------------
  if (getDate(__DATE__) && getTime(__TIME__)) {
    RTC.read(tm2);
    lcd.print("Heure : ");
    lcd.print(__TIME__);
    delay(TimeBeetwenTests);
    lcd.clear();
    if((tm.Year > tm2.Year) || (tm.Year == tm2.Year && tm.Month > tm2.Month) ||
       (tm.Year == tm2.Year && tm.Month == tm2.Month && tm.Day > tm2.Day) ||
       (tm.Year == tm2.Year && tm.Month == tm2.Month && tm.Day == tm2.Day && tm.Hour != tm2.Hour) ||
       (tm.Year == tm2.Year && tm.Month == tm2.Month && tm.Day == tm2.Day && tm.Hour == tm2.Hour && tm.Minute > tm2.Minute)) {
      if (RTC.write(tm)) {
        lcd.print("Heure : ");
        lcd.print(__TIME__);
        delay(TimeBeetwenTests);
        lcd.clear();
      }
    }
  }
  //--------------------------------------------------------------------
  lcd.setCursor(0, 0);
  lcd.print("TEST POMPE");
  digitalWrite(PIN_RELAY1, HIGH);
  lcd.setCursor(0, 0);
  lcd.print("POMPE ALLUMEE");
  delay(TimeBeetwenTests);
  lcd.clear();
  digitalWrite(PIN_RELAY1, LOW);
  lcd.setCursor(0, 0);
  lcd.print("POMPE ETEINTE");
  delay(TimeBeetwenTests);
  lcd.clear();
  //--------------------------------------------------------------------
  lcd.setCursor(0, 0);
  lcd.print("TEST LAMPE");
  digitalWrite(PIN_RELAY2, HIGH);
  lcd.setCursor(0, 0);
  lcd.print("LAMPE ALLUMEE");
  delay(TimeBeetwenTests);
  lcd.clear();
  digitalWrite(PIN_RELAY2, LOW);
  lcd.setCursor(0, 0);
  lcd.print("LAMPE ETEINTE");
  delay(TimeBeetwenTests);
  lcd.clear();
  //--------------------------------------------------------------------
  int Reading;
  int MOY = 3;
  Reading = analogRead(PIN_LUX);                          
  Reading = map(Reading, 0, 1000, 100, 0);
  LuxPercent = (Reading + LuxPercent * MOY)/(MOY + 1);                         
  while(DHT11.read(DHT11PIN) != 0){              
  }
  Humidity = DHT11.humidity;       
  Temperature = DHT11.temperature;
  lcd.write(byte(1));
  print2digitsLCD(LuxPercent);
  lcd.print("  ");
  lcd.write(byte(2));
  print2digitsLCD(Humidity);
  lcd.print("  ");
  lcd.write(byte(3));
  print2digitsLCD(Temperature);
  delay(5000);
  lcd.clear();
  //--------------------------------------------------------------------
  lcd.print("TEST FINI");
}

void loop() {
  delay(1000);
}

void lcd_initialisation() {
  byte error, address;
  byte I2CIC1 = 0x68;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      if(address != I2CIC1) {
        lcd.change_address(address);
        lcd.init();
        lcd.backlight();
        lcd.setCursor(0,0);
        lcd.print("ECRAN OK");
        delay(TimeBeetwenTests);
      }
    }
  }
  lcd.init();
}

bool getTime(const char *str){
  int Hour, Min, Sec;
  if (sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec) != 3) return false;
  tm.Hour = Hour;
  tm.Minute = Min;
  tm.Second = Sec;
  return true;
}

bool getDate(const char *str){
  char Month[12];
  int Day, Year;
  uint8_t monthIndex;
  if (sscanf(str, "%s %d %d", &Month, &Day, &Year) != 3) return false;
  for (monthIndex = 0; monthIndex < 12; monthIndex++) {
    if (strcmp(Month, monthName[monthIndex]) == 0) break;
  }
  if (monthIndex >= 12) return false;
  tm.Day = Day;
  tm.Month = monthIndex + 1;
  tm.Year = CalendarYrToTm(Year);
  return true;
}

void print2digitsLCD(byte number) {
  if (number >= 0 && number < 10) {
     lcd.print("0");
  }
   lcd.print(number);
}
