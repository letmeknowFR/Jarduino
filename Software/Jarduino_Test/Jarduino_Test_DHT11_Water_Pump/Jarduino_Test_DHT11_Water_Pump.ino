/*******************************************************************/
/*********** The Jarduino Test DHT11 + Water Pump ******************/
/*******************************************************************/

#include <dht11.h>                            // Ajout de la librairie nécessaire pour l'utilisation de capteur DHT11
#include <LiquidCrystal_I2C.h>                // Ajout de la librairie nécessaire pour l'utilisation de l'écran LCD
#include <Wire.h>                             // Ajout de la librairie nécessaire pour la communication I2C
#include <Time.h>
#include <DS1307RTC.h>

#define PRINT 1                               // Affichage : LCD(1) - Moniteur série(0) 
#define MOY 3                                 // Nombre de valeurs utilisées pour les moyennes de mesures
#define DHT11PIN 13                           // Pin du capteur d'humidité / temperature
#define BACKLIGHT_PIN 3                       // Pin d'alimentation de l'écran LCD
#define I2C_ADDR 0x27                         // Adresse de l'I2C
#define PIN_RELAY1 4                          // Pin du relais de la pompe

LiquidCrystal_I2C  lcd(I2C_ADDR, 16, 2);      // Création de l'écran LCD
dht11 DHT11;                                  // Création de l'entité DHT11
int   TempAndHum;                             // Déclaration d'une variable température / humidité
byte  Humidity,Temperature = 1;               // Déclaration de deux variables humidité + température
tmElements_t tm,tm2;                          // tmElements est une structure contenant plusieurs information remplie grâce à la fonction RTC.read()

/***************************** INIT ******************************/
void setup() {
  Serial.begin(9600);                         // Initialisation du moniteur série (Outils/Moniteur série)
  Wire.begin();                               // Initialisation de l'I2C
  lcd_initialisation();                       // Initialisation de l'écran LCD
  lcd.backlight();                            // Initialisation du rétro-éclairage de l'écran LCD
  RTC.read(tm2);                              // Lecture du RTC chip (Real Time Clock)
  tm2.Minute = tm2.Minute + 2;                // Arrosage toutes les 2 minutes
  pinMode(PIN_RELAY1, OUTPUT);                // Configure le pin spécifié pour qu'il se comporte en sortie
  digitalWrite(PIN_RELAY1, LOW);              // Met un niveau logique BAS sur le pin spécifié
}
/***************************** FIN INIT **************************/

/***************************** LOOP ******************************/
void loop() {
  TempAndHum = DHT11.read(DHT11PIN);                                        // Récupérer la valeur du capteur de température / humidité
  Humidity = (DHT11.humidity + Humidity * MOY)/(MOY + 1);                   // Moyenner la valeur de l'humidité
  Temperature = (DHT11.temperature + Temperature * MOY)/(MOY + 1);          // Moyenner la valeur de temperature
  RTC.read(tm);                                                             // Lecture du RTC chip (Real Time Clock), rempli la structure tmElements

  #ifdef PRINT                                                              // Si affichage sur l'écran LCD
    lcd.clear();                                                            // Nettoyage de l'écran
    lcd.setCursor(0, 0);                                                    // On place le curseur en haut à gauche de l'écran
    lcd.print("Arrosage: ");                                                // Affichage de l'heure du future arrosage
    print2digitsLCD(tm.Hour);
    lcd.print(":");
    print2digitsLCD(tm2.Minute);
    lcd.setCursor(0, 1);                                                    // On place le curseur à la ligne inférieur
    lcd.print(Humidity);                                                    // Affichage du pourcentage d'humidité
    lcd.print("%");
  #endif
  Serial.print("Arrosage à ");                                              // Affichage sur le moniteur série du future arrosage + humidité
  Serial.print(tm2.Hour);
  Serial.print(":");
  Serial.print(tm2.Minute);
  Serial.print(" - Humidité : ");
  Serial.println(Humidity);
  if (tm2.Minute == tm.Minute && tm2.Hour == tm.Hour) {                     // Si c'est l'heure d'arroser
    if (Humidity < 50) {                                                    // Si l'humidité est inférieur à 50% on arrose
      tm2.Hour = tm.Hour;
      digitalWrite(PIN_RELAY1, HIGH);                                       // Mise en marche de la pompe
      Serial.println("Pompe ON");                                           // Affichage sur le moniteur série
      #ifdef PRINT                                                          // Si affichage sur l'écran LCD
        lcd.setCursor(0, 1);                                                // On place le curseur à la ligne inférieur
        lcd.print("Pompe ON");
      #endif
      delay(2000);                                                          // Arrosage de deux secondes
      digitalWrite(PIN_RELAY1, LOW);                                        // Arrèt de la pompe
    }
    tm2.Minute = tm.Minute + 2;                                             // Arrosage toutes les 2 minutes
  }
}
/***************************** FIN LOOP **************************/

/************************* 2 DIGITS ******************************/
void print2digitsLCD(byte number) {                   //Permet d'aficher un 0 avant un chiffre en dessous de 10
  if (number >= 0 && number < 10) {
     lcd.print("0");
  }
   lcd.print(number);
}
/*********************** FIN 2 DIGITS ****************************/

/****************** LCD INIT *************************************/
void lcd_initialisation() {                           // Fonction d'initlisation de l'écran LCD
  byte error, address;
  byte I2CIC1 = 0x68;

  for (address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);                  // L'i2c_scanner utilise la valeur de retournée par la fonction Write.endTransmisstion,
    error = Wire.endTransmission();                   // Afin de déterminer si un composant est assigné à l'adresse retourné.
    if (error == 0) {                                 // Si la transmission est un succés et
      if (address != I2CIC1)                          // Si différent de l'I2C alors on change l'adresse.
        lcd.change_address(address);
    }
  }
  lcd.init();                                         // Initialisation de l'écran LCD
}
/****************** FIN LCD INIT *********************************/
