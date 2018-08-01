/*******************************************************************/
/*************** The Jarduino Test LDR + Lampe *********************/
/*******************************************************************/

#include <LiquidCrystal_I2C.h>                // Ajout de la librairie nécessaire pour l'utilisation de l'écran LCD
#include <Wire.h>                             // Ajout de la librairie nécessaire pour la communication I2C

#define PRINT 0                               // Affichage : LCD(0) - Moniteur série(1) 
#define MOY 3                                 // Nombre de valeurs utilisées pour les moyennes de mesures
#define PIN_LUX A5                            // Pin de la photorésistance LDR
#define BACKLIGHT_PIN 3                       // Pin d'alimentation de l'écran LCD
#define I2C_ADDR 0x27                         // Adresse de l'I2C
#define PIN_RELAY1 9                          // Pin du relais de la 

LiquidCrystal_I2C  lcd(I2C_ADDR, 16, 2);      // Création de l'écran LCD
byte LuxPercent = 1;                          // Déclaration d'une variable pour le pourcentage de lumière

/***************************** INIT ******************************/
void setup() {
  if (PRINT == 1) {
    Serial.begin(9600);                        // Initialisation du moniteur série (Outils/Moniteur série)
    while (!Serial);                           // Attend que le moniteur série soit connecté
  }
  else {
    Wire.begin();                              // Initialisation de l'I2C
    lcd_initialisation();                      // Initialisation de l'écran LCD
    lcd.backlight();                           // Initialisation du rétro-éclairage de l'écran LCD
  }
  pinMode(PIN_RELAY1,OUTPUT);                  // Configure le pin spécifié pour qu'il se comporte en sortie
  digitalWrite(PIN_RELAY1,LOW);                // Met un niveau logique BAS sur le pin spécifié
}
/***************************** FIN INIT **************************/

/***************************** LOOP ******************************/
void loop() {
  int Reading = analogRead(PIN_LUX);                              // Récupérer la valeur du capteur de luminosité
  Reading = map(Reading, 0, 1000, 100, 0);                        // Ramener la valeur entre 0 et 100
  LuxPercent = (Reading + LuxPercent * MOY)/(MOY + 1);            // Moyenner la valeur

  if (PRINT == 1) {                                               // Affichage de la luminosité sur le moniteur série
    Serial.print("Luminosite : ");
    Serial.print(LuxPercent);
    Serial.println("%");
  } else {                                                        // Affichage de la luminosité sur l'écran LCD
    lcd.clear();                                                  // Nettoyage de l'écran
    lcd.setCursor(0, 0);                                          // On place le curseur en haut à gauche de l'écran
    lcd.print("Luminosite : ");                                   // Affichage de la luminosité
    lcd.print(LuxPercent);
    lcd.print("%");
  }
  if (LuxPercent < 60) {                                          // Si la luminosité est inférieur à 60% alors on allume la lampe
    digitalWrite(PIN_RELAY1, HIGH);                               // Met un niveau logique HAUT sur le pin spécifié
    lcd.setCursor(0, 1);                                          // On place le curseur à la ligne inférieur
    lcd.print("Lampe ON");                                        // Affichage sur l'écran LCD
    Serial.println("Lampe ON");                                   // Affichage sur le moniteur série
  } else {
    digitalWrite(PIN_RELAY1, LOW);                                // Met un niveau logique BAS sur le pin spécifié
  }
  delay(500);                                                     // Pause de 0,5 seconde
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
