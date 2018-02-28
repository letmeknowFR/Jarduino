/*******************************************************************/
/********************* The Jarduino ********************************/
/*******************************************************************/

#include <Wire.h>
#include <Time.h>
#include <dht11.h>
#include <avr/wdt.h>
#include <DS1307RTC.h>
#include <LiquidCrystal_I2C.h>

/********************* CONFIGURATION ******************************/

//#define DEBUG   1      // Debug Mode
#define LUXMIN 70        // Luminosité maximale
#define LUXMAX 50        // Luminosité minimale
#define HourMax 22       // Heure d'éteinte du Jarduino
#define HourMin 8        // Heure de démmarage du Jarduino
#define Startingdelay 500// Temps entre icone au démarrage

/********************* FIN CONFIGURATION **************************/

#define PIN_LUX A5        // LUX pin
#define BACKLIGHT_PIN 3   // LCD Screen Light
#define I2C_ADDR 0x27     // LCD Screen Adress
#define PIN_RELAY1 4      // Relais de commande pompe
#define PIN_RELAY2 9      // Relais de commande lumière
#define DHT11PIN 13       // Pin du capteur d'humidité / temperature
#define MOY 3             // Nombre de valeurs utilisées pour les moyennes de mesures

#ifdef DEBUG
#define PIN_TX 0          // Inutile de la définir comme entrée sortie sera fait par le Serial.begin
#define PIN_RX 1          // Inutile de la définir comme entrée sortie sera fait par le Serial.begin
#endif

LiquidCrystal_I2C  lcd(I2C_ADDR, 16, 2);// LCD creation
dht11 DHT11;                            // Creation de l'entité DHT11
tmElements_t tm,tm2;                    // Horloge + Horloge comparaison
int TempAndHum;                         // Température + humidité
boolean pumpon=false,lighton=false,pumplcd=false,pumplcdalreadyon=false,AfterWateringOrInit=false;
byte WateringTimeLCD,LuxPercent,LuxPercentOld,MinuteOld,Humidity,HumidityOld,Temperature,TemperatureOld=1;
const char *monthName[12] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
/********************************* LOGO **************************/

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
/***************************** FIN LOGO **************************/

/***************************** INIT ******************************/
void setup() {
  wdt_enable(WDTO_8S); // Watchdog de 8s
  
  #ifdef DEBUG
  bool parse=false;
  bool config=false;
  #endif

  // get the date and time the compiler was run
  if (getDate(__DATE__) && getTime(__TIME__)) {
    #ifdef DEBUG
    parse = true;
    #endif
    
    RTC.read(tm2); //Lecture du RTC Chip 2
    #ifdef DEBUG
    Serial.print("Lecture + tm2");
    delay(10000);
    #endif
    
    if((tm.Year > tm2.Year) || (tm.Year == tm2.Year && tm.Month > tm2.Month) ||
       (tm.Year == tm2.Year && tm.Month == tm2.Month && tm.Day > tm2.Day) ||
       (tm.Year == tm2.Year && tm.Month == tm2.Month && tm.Day == tm2.Day && tm.Hour > tm2.Hour) ||
       (tm.Year == tm2.Year && tm.Month == tm2.Month && tm.Day == tm2.Day && tm.Hour == tm2.Hour && tm.Minute > tm2.Minute)) {
      if (RTC.write(tm)) {
        #ifdef DEBUG
        config = true;
        Serial.println("Update"); 
        #endif
      }
    }
  }
  
  #ifdef DEBUG
  Serial.begin(9600);
  while (!Serial) ;
  delay(100);
  Serial.print("The Jarduino demarre... \n\n");
  if (parse && config) {
    Serial.print("RTC chip configure avec: Heure=");
    Serial.print(__TIME__);
    Serial.print(", Date=");
    Serial.println(__DATE__);
  } else if (parse)
      Serial.println("Pas de mise a jour de la RTC\n");
    else
      Serial.print("Erreur de communication avec le RTC chip\n");
  #endif
  
  Wire.begin();
  lcd_initialisation();
  lcd.createChar(0, water);
  lcd.createChar(1, light);
  lcd.createChar(2, humidity);
  lcd.createChar(3, temp);
  lcd.backlight();
  lcd.setCursor(4, 0);
  lcd.print("JARDUINO");
  delay(Startingdelay);
  lcd.setCursor(0, 1);
  lcd.print("   ");
  lcd.write(byte(3));
  delay(Startingdelay);
  lcd.print("  ");
  lcd.write(byte(2));
  delay(Startingdelay);
  lcd.print("  ");
  lcd.write(byte(1));
  delay(Startingdelay);
  lcd.print("  ");
  lcd.write(byte(0));
  delay(Startingdelay);
  lcd.clear();
  AfterWateringOrInit = true;
  
  pinMode(PIN_RELAY1,OUTPUT);
  digitalWrite(PIN_RELAY1,LOW);
  pinMode(PIN_RELAY2,OUTPUT);
  digitalWrite(PIN_RELAY2,LOW);
}
/************************* FIN INIT ******************************/

/***************************** LOOP ******************************/
void loop() {
  wdt_reset(); // Reset Watchdog
  
  RTC.read(tm); //Lecture du RTC Chip
  GetSensorsValues();
  WateringTimeCalcul();
  Light();
  Pump();
  LCDDisplay();
  
  #ifdef DEBUG
  Serial.print("Pompe: ");
  Serial.print(pumpon);
  Serial.print(" | ");
  Serial.print("Lumiere: ");
  Serial.print(lighton);
  Serial.print(" | ");
  Serial.print("Pompe LCD: ");
  Serial.print(pumplcd);
  Serial.print(" | ");
  Serial.print("Temps arrosage: ");
  Serial.print(WateringTimeLCD);
  Serial.print(" | ");
  print2digitsSERIAL(tm.Hour);
  Serial.write(':');
  print2digitsSERIAL(tm.Minute);
  Serial.write(':');
  print2digitsSERIAL(tm.Second);
  Serial.print("\n");
  #endif
  
  LuxPercentOld = LuxPercent;
  HumidityOld = Humidity;
  TemperatureOld = Temperature;
  MinuteOld = tm.Minute;
  delay(500);
}
/************************* FIN LOOP ******************************/

/***************** RECUPERATION HEURE/DATE ***********************/
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
/*************** END RECUPERATION HEURE/DATE ***********************/

/************************* 2 DIGITS ******************************/
//Permet d'aficher un 0 avant un chiffre en dessous de 10
#ifdef DEBUG
void print2digitsSERIAL(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}
#endif

void print2digitsLCD(byte number) {
  if (number >= 0 && number < 10) {
     lcd.print("0");
  }
   lcd.print(number);
}
/*********************** FIN 2 DIGITS ****************************/

/************************* VALEURS CAPTEURS **********************/
void GetSensorsValues() {
  int Reading = analogRead(PIN_LUX);                              // Récupérer la valeur du capteur de luminosité
  Reading = map(Reading, 0, 1000, 100, 0);                        // Ramener la valeur entre 0 et 100
  LuxPercent = (Reading + LuxPercent * MOY)/(MOY + 1);            // Moyenner la valeur
  TempAndHum = DHT11.read(DHT11PIN);                              // Récupérer la valeur du capteur de température / humidité
  Humidity = (DHT11.humidity + Humidity * MOY)/(MOY + 1);         // Moyenner la valeur de l'humidité
  Temperature = (DHT11.temperature + Temperature * MOY)/(MOY + 1);// Moyenner la valeur de temperature
  
  #ifdef DEBUG
  Serial.print("Luminosite: ");
  Serial.print(LuxPercent);
  Serial.print(" | ");
  Serial.print("Humidite: ");
  Serial.print(Humidity);
  Serial.print(" | ");
  Serial.print("Temperature: ");
  Serial.print(Temperature);
  Serial.print(" | ");
  #endif
}
/******************** FIN VALEURS CAPTEURS ***********************/

/************************* POMPE *********************************/
void Pump() {
  if (tm.Minute == 0 || tm.Minute == 20 || tm.Minute == 40){
    if (tm.Second > 0 && tm.Second <= 20){
      if (pumpon == false){
        digitalWrite(PIN_RELAY1, HIGH);
        pumpon = true;
        pumplcd = true;
      }
    } else {
      if (pumpon == true){
        digitalWrite(PIN_RELAY1, LOW);
        pumpon = false;
        pumplcd = false;
        pumplcdalreadyon = false;
        lcd.clear();
        AfterWateringOrInit = true;
      }
    }
  } else {
    if (pumpon == true || digitalRead(PIN_RELAY1) == HIGH){
      digitalWrite(PIN_RELAY1, LOW);
      pumpon = false; 
      pumplcd = false;
      pumplcdalreadyon = false;
    }
  }
}
/************************** FIN POMPE ****************************/

/**************************** LUMIERE ****************************/
void Light() {
  if (tm.Hour <= HourMax && tm.Hour >= HourMin){
      if (LuxPercent >= LUXMIN){
        if (lighton == true){
            digitalWrite(PIN_RELAY2, LOW);
            lighton = false;
            pumplcd=false;
        }
      } else {
        if (LuxPercent <= LUXMAX){
          if (lighton == false){
            digitalWrite(PIN_RELAY2, HIGH);
            lighton = true;
          }
        }
      }
  } else {
    if (lighton == true || digitalRead(PIN_RELAY2) == HIGH) {
      digitalWrite(PIN_RELAY2, LOW);
      lighton = false;
    }
  }
}
/************************* FIN LUMIERE ***************************/

/************************* AFFICHAGE LCD *************************/
void LCDDisplay() {
  if (pumpon == true && pumplcd == true && pumplcdalreadyon == false) {
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.write(byte(2));
    lcd.print("  WATERING  ");
    lcd.write(byte(2));
    lcd.setCursor(0, 1);
    lcd.write(byte(2));
    lcd.print(" IN PROGRESS  ");
    lcd.write(byte(2)); 
    pumplcdalreadyon = true;
  } else if (pumpon == true && pumplcd == true) {
    lcd.setCursor(1, 0);
    lcd.write(byte(2));
    lcd.print("  WATERING  ");
    lcd.write(byte(2));
    
    lcd.setCursor(0, 1);
    lcd.write(byte(2));
    lcd.print(" IN PROGRESS  ");
    lcd.write(byte(2)); 
  } else {
    if(AfterWateringOrInit == true){
        lcd.clear();
        lcd.setCursor(0,0);
        print2digitsLCD(tm.Hour);
        lcd.print(':');
        print2digitsLCD(tm.Minute);
        lcd.setCursor(11,0);
        lcd.write(byte(2));
        print2digitsLCD(WateringTimeLCD);
        lcd.print("mn");
        lcd.setCursor(6,1);
        lcd.write(byte(0));
        print2digitsLCD(Humidity);
        lcd.print("%");
        lcd.setCursor(0,1);
        lcd.write(byte(3));
        print2digitsLCD(Temperature);
        lcd.print((char)223);
        lcd.print("C");
        lcd.setCursor(12,1);
        lcd.write(byte(1));
        print2digitsLCD(LuxPercent);
        lcd.print("%");
        AfterWateringOrInit = false;
    } else{
      if(tm.Minute != MinuteOld){
        lcd.setCursor(0,0);
        print2digitsLCD(tm.Hour);
        lcd.print(':');
        print2digitsLCD(tm.Minute);
        lcd.setCursor(11,0);
        lcd.write(byte(2));
        print2digitsLCD(WateringTimeLCD);
        lcd.print("mn");
      }
      if(Humidity != HumidityOld){
        lcd.setCursor(6,1);
        lcd.write(byte(0));
        print2digitsLCD(Humidity);
        lcd.print("%");
      }
      if(Temperature != TemperatureOld){
        lcd.setCursor(0,1);
        lcd.write(byte(3));
        print2digitsLCD(Temperature);
        lcd.print((char)223);
        lcd.print("C");
      }
      if(LuxPercent != LuxPercentOld){
        lcd.setCursor(12,1);
        lcd.write(byte(1));
        print2digitsLCD(LuxPercent);
        lcd.print("%");
      }
    }
   }
}
/********************** FIN AFFICHAGE LCD ************************/

/******************* CALCUL TEMPS ARROSAGE ***********************/
void WateringTimeCalcul()
{ 
  WateringTimeLCD = 20 - (tm.Minute % 20); // Nombre de minutes avant le prochain arrosage
}
/****************** FIN CALCUL TEMPS ARROSAGE ********************/

/****************** LCD INIT *************************************/
void lcd_initialisation() { // Fonction d'initlisation LCD
  byte error, address;
  byte I2CIC1 = 0x68;

  #ifdef DEBUG
  Serial.println("Scanning...");
  #endif
 
  for(address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
 
    if (error == 0)
    {
      if(address != I2CIC1) {
        lcd.change_address(address);
      }
    }
  }
  lcd.init();
}
/****************** FIN LCD INIT *********************************/
