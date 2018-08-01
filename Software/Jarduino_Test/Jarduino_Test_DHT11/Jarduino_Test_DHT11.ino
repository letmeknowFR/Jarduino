/*******************************************************************/
/*************** The Jarduino Test DHT11 ***************************/
/*******************************************************************/

#include <dht11.h>                            // Ajout de la librairie nécessaire pour l'utilisation de capteur DHT11
#include <LiquidCrystal_I2C.h>                // Ajout de la librairie nécessaire pour l'utilisation de l'écran LCD
#include <Wire.h>                             // Ajout de la librairie nécessaire pour la communication I2C

#define PRINT 1                               // Affichage : LCD(0) - Moniteur série(1) 
#define MOY 3                                 // Nombre de valeurs utilisées pour les moyennes de mesures
#define DHT11PIN 13                           // Pin du capteur d'humidité / temperature
#define BACKLIGHT_PIN 3                       // Pin d'alimentation de l'écran LCD
#define I2C_ADDR 0x27                         // Adresse de l'I2C

LiquidCrystal_I2C  lcd(I2C_ADDR, 16, 2);      // Création de l'écran LCD
dht11 DHT11;                                  // Création de l'entité DHT11
int   TempAndHum;                             // Déclaration d'une variable température / humidité
byte  Humidity,Temperature = 1;               // Déclaration de deux variables humidité + température

/***************************** INIT ******************************/
void setup() {
  if (PRINT == 1) {
    Serial.begin(9600);                        // Initialisation du moniteur série (Outils/Moniteur série)
    while (!Serial);                           // Attend que le moniteur série soit connecté
  } else {
    Wire.begin();                              // Initialisation de l'I2C
    lcd_initialisation();                      // Initialisation de l'écran LCD
    lcd.backlight();                           // Initialisation du rétro-éclairage de l'écran LCD
  }
}
/***************************** FIN INIT **************************/

/***************************** LOOP ******************************/
void loop() {
  TempAndHum = DHT11.read(DHT11PIN);                                // Récupérer la valeur du capteur de température / humidité
  Humidity = (DHT11.humidity + Humidity * MOY)/(MOY + 1);           // Moyenner la valeur de l'humidité
  Temperature = (DHT11.temperature + Temperature * MOY)/(MOY + 1);  // Moyenner la valeur de temperature

  if (PRINT == 1) {                                                 // Affichage de l'humidité + température sur le moniteur série
    Serial.print("Temperature : ");
    Serial.print(Temperature);
    Serial.print(" - Humidite : ");
    Serial.println(Humidity);
  } else {                                                          // Affichage de l'humidité + température sur l'écran LCD
    lcd.clear();                                                    // Nettoyage de l'écran
    lcd.setCursor(0, 0);                                            // On place le curseur en haut à gauche de l'écran
    lcd.print("Temperature : ");                                    // Affichage de la température
    lcd.print(Temperature);
    lcd.setCursor(0, 1);                                            // On place le curseur à la ligne inférieur
    lcd.print("Humidite :    ");                                    // Affichage de l'humidité
    lcd.print(Humidity);
  }
  delay(500);                                                       // Pause de 0,5 seconde
}
/***************************** FIN LOOP **************************/

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
