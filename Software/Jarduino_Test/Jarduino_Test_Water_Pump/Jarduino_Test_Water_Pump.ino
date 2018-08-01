/*******************************************************************/
/*************** The Jarduino Test Water pump **********************/
/*******************************************************************/

#include <LiquidCrystal_I2C.h>                     // Ajout de la librairie nécessaire pour l'utilisation de l'écran LCD
#include <Wire.h>                                  // Ajout de la librairie nécessaire pour la communication I2C

#define BACKLIGHT_PIN 3                            // Pin d'alimentation de l'écran LCD
#define I2C_ADDR 0x27                              // Adresse de l'I2C
#define PIN_RELAY1 4                               // Pin du relais de la pompe

LiquidCrystal_I2C  lcd(I2C_ADDR, 16, 2);           // Création de l'écran LCD
String cmd;                                        // Déclaration d'une chaine de charactère

/***************************** INIT ******************************/
void setup() {
  Serial.begin(9600);                              // Initialisation du moniteur série (Outils/Moniteur série)
  while (!Serial);                                 // Attend que le moniteur série soit connecté
  Wire.begin();                                    // Initialisation de l'I2C
  lcd_initialisation();                            // Initialisation de l'écran LCD
  lcd.backlight();                                 // Initialisation du rétro-éclairage de l'écran LCD
  pinMode(PIN_RELAY1,OUTPUT);                      // Configure le pin spécifié pour qu'il se comporte en sortie
  digitalWrite(PIN_RELAY1,LOW);                    // Met un niveau logique BAS sur le pin spécifié
}
/***************************** FIN INIT **************************/

/***************************** LOOP ******************************/
void loop() {
  cmd = Serial.readStringUntil('\n');              // Récupération de l'instruction venant du moniteur série
  if (cmd != NULL)
    Serial.println("Commande entrée: " + cmd);
  if (cmd.equals("on")) {                          // On compare l'instruction avec "on"
    digitalWrite(PIN_RELAY1, HIGH);                // Met un niveau logique HAUT sur le pin spécifié
    lcd.clear();                                   // Nettoyage de l'écran
    lcd.print("Pompe ON");                         // Affichage sur l'écran LCD
    Serial.println("Pompe ON");                    // Affichage sur le moniteur série
  }
  if (cmd.equals("off")) {                         // On compare l'instruction avec "off"
    digitalWrite(PIN_RELAY1, LOW);                 // Met un niveau logique BAS sur le pin spécifié
    lcd.clear();                                   // Nettoyage de l'écran
    lcd.print("Pompe OFF");                        // Affichage sur l'écran LCD
    Serial.println("Pompe OFF");                   // Affichage sur le moniteur série
  }
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
