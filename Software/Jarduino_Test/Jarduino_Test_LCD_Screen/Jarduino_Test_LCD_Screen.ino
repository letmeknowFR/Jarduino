/*******************************************************************/
/*************** The Jarduino Test LCD Screen **********************/
/*******************************************************************/

#include <LiquidCrystal_I2C.h>                     // Ajout de la librairie nécessaire pour l'utilisation de l'écran LCD
#include <Wire.h>                                  // Ajout de la librairie nécessaire pour la communication I2C

#define BACKLIGHT_PIN 3                            // Pin d'alimentation de l'écran LCD
#define I2C_ADDR 0x27                              // Adresse de l'I2C

LiquidCrystal_I2C  lcd(I2C_ADDR, 16, 2);           // Création de l'écran LCD

/********************************* LOGO ****************************/
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
/***************************** FIN LOGO ****************************/

/***************************** INIT ********************************/
void setup() {
  Serial.begin(9600);                              // Initialisation du moniteur série (Outils/Moniteur série)
  Wire.begin();                                    // Initialisation de l'I2C
  lcd_initialisation();                            // Initialisation de l'écran LCD
  lcd.backlight();                                 // Initialisation du rétro-éclairage de l'écran LCD
  lcd.createChar(0, water);                        // Création des quatres différents logo
  lcd.createChar(1, light);
  lcd.createChar(2, humidity);
  lcd.createChar(3, temp);
}
/***************************** FIN INIT ****************************/

/***************************** LOOP ********************************/
void loop() {
  int i;                                         // Déclaration d'une variable de type entier
  lcd.clear();                                   // Nettoyage de l'écran
  lcd.setCursor(0, 0);                           // On place le curseur en haut à gauche de l'écran
  lcd.print("Test LCD");                         // Affichage sur l'écran LCD
  lcd.setCursor(0, 1);                           // On place le curseur à la ligne inférieur
  Serial.println("Test LCD");                    // Affichage sur le moniteur série
  delay(500);                                    // Pause de 0,5 seconde
  for (i = 0; i < 4; i++) {
    lcd.print("  ");
    lcd.write(byte(i));                          // Affichage du logo sur l'écran LCD
    Serial.print("Logo ");                       // Affichage sur le moniteur série
    Serial.println(i);
    delay(1000);                                 // Pause d'une seconde
  }
}
/***************************** FIN LOOP ****************************/

/****************** LCD INIT ***************************************/
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
/****************** FIN LCD INIT ***********************************/
