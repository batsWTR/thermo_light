// Thermo light
// systeme de notification de la temperature en couleur
// sonde temp BMP180
// led WS2812b

// bleu = froid
// rouge = chaud

// septembre 2015
// b. Wentzler

//----------------------------INCLUDES-----------------------------
#include <Wire.h>  // communication i2c
#include <Adafruit_NeoPixel.h>  // include  pour les leds
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>  //OLED display




//---------------------------DEFINE--------------------------------

// BMP180
#define BMP_ADRESS 0x77  // adresse du module bmp180 1110111
#define AC6 0xB4  // adresse  MSB pour le calcul temp
#define AC5 0xB2
#define MC 0xBC
#define MD 0xBE


// WS2812b
#define NUM_LED 3
#define DATA_PIN 12


// OLED
#define OLED_SDA 10
#define OLED_SCL 11
#define OLED_RST 9
#define OLED_DC 8


//----------------------------VAR-------------------------------------
// var de l eprom pour la mesure de temperature
long ac6,ac5,mc,md;


long temp =0, ut = 0;
float temp2 = 0;
unsigned char msb, lsb;



//---------------------------PROTOTYPES--------------------------
// Fonction BMP 180
void bmp180_cal(long *, long *, long *, long *);
long bmp180_UT();


//  Affichage de la couleur en fonction de la temp
void setColor(float temp);



Adafruit_NeoPixel b_led = Adafruit_NeoPixel(NUM_LED,DATA_PIN,NEO_GRB + NEO_KHZ800);
Adafruit_SSD1306 oled = Adafruit_SSD1306(OLED_SDA,OLED_SCL,OLED_DC,OLED_RST,7);

void setup()
{
  Wire.begin(); // join i2c bus (address optional for master)
  oled.begin();
  Serial.begin(9600);
  
  pinMode(DATA_PIN, OUTPUT);
  b_led.begin();
  
  
  
  // fonction permettant de recuperer les infos de calibration
  bmp180_cal(&ac6,&ac5,&mc,&md);
  
  


  
}



void loop()
{
  
  //  fonction de recuperation de la donnee brute
  ut = bmp180_UT();
  
  
  // calc temp
  long x1 = (ut-ac6) * (ac5 / pow(2,15));
  long x2 = mc * pow(2,11) / (x1 + md);
  long b5 = x1 + x2;
  temp2 = (b5 + 8) / pow(2,4);
  
  // AH ah la temp est a diviser par 10
  temp2 = temp2 / 10;
  
  
  
  //Serial.print("Temperature: ");
  Serial.println(temp2,2);
  //Serial.println("deg C");
  
  // calcul de la couleur des leds
  setColor(temp2);
  

  
  //  affichage de la temp sur l ecran oled
  oled.clearDisplay();
  oled.setTextSize(3);
  //oled.setTextColor(WHITE);
  oled.setCursor(0,0);
  oled.print(" 00:00");
  oled.setTextColor(WHITE);
  oled.setCursor(10,40);
  oled.print(temp2,1);
  oled.println(" c");
  oled.display();
  
  delay(1000);
  
  
  
  
  

}



void bmp180_cal(long *ac6=0, long *ac5=0, long *mc=0, long *md=0)
{
    // recuperation des donnees de calibration en eprom
    
    // AC6
  Wire.beginTransmission(BMP_ADRESS);
  Wire.write(AC6);  
  Wire.endTransmission();
  Wire.requestFrom(BMP_ADRESS,2);
  while(Wire.available() < 2);
  msb = Wire.read();
  lsb = Wire.read();
  *ac6 = msb<<8 | lsb;
  
  
  // AC5
  Wire.beginTransmission(BMP_ADRESS);
  Wire.write(AC5); 
  Wire.endTransmission();
  Wire.requestFrom(BMP_ADRESS,2);
  while(Wire.available() < 2);
  msb = Wire.read();
  lsb = Wire.read();
  *ac5 = msb<<8 | lsb;
  
  // MC
  Wire.beginTransmission(BMP_ADRESS);
  Wire.write(MC); 
  Wire.endTransmission();
  Wire.requestFrom(BMP_ADRESS,2);
  while(Wire.available() < 2);
  msb = Wire.read();
  lsb = Wire.read();
  *mc = msb<<8 | lsb;
  
  
  //MD
  Wire.beginTransmission(BMP_ADRESS);
  Wire.write(MD); 
  Wire.endTransmission();
  Wire.requestFrom(BMP_ADRESS,2);
  while(Wire.available() < 2);
  msb = Wire.read();
  lsb = Wire.read();
  *md = msb<<8 | lsb;
  
}

long bmp180_UT()
{
  
  long temp = 0;
    // demarre la transmission avec l adresse du BMP180 en ecriture
  Wire.beginTransmission(BMP_ADRESS);
  //envoie de l adresse du registre a ecrire
  Wire.write(0xF4);
  // conf du registre
  Wire.write(0x2E);
  // envoie du paquet au BMP180
  Wire.endTransmission();
  // attente de 5mS avant lecture de temp
  delay(5);
  
  
  // lecture temperature
  
  // adresse du BMP180 en lecture
  Wire.beginTransmission(BMP_ADRESS);
  // adresse a lire MSB temp
  Wire.write(0xF6);
  // envoie du paquet
  Wire.endTransmission();
  
  
  // on attend 2 bytes
  Wire.requestFrom(BMP_ADRESS, 2);
  // en attente des 2 bytes
  while(Wire.available()<2);;
  msb = Wire.read();
  lsb = Wire.read();
  temp = (long) msb<<8 | lsb;
  return temp;
  
}


// couleur en fonction de la temp
void setColor(float temp)
{
  int r = 128,g = 0,b = 0;
  int var = 0;
  boolean on = false;
  
  
  // j enleve la virgule en multipliant par dix et arrondi en int
  temp = temp * 10;
  var = (int) temp;
  
  
 // test des cas
 if(var < 160){
   // blink blue
   if(on){
     on = false;
     r = 0;
     g = 0;
     b = 0;
   }
   else{
     on = true;
     r = 0;
     g = 0;
     b = 255;
   }
 }
 else if(var >= 160 && var <170){
   r = 0;
   g = 0;
   b = 255;
 }
 else if(var >= 170 && var < 180){
   r = 72;
   g = 209;
   b = 204;
   
 }
 else if(var >= 180 && var < 190){
   r = 127;
   b = 255;
   g = 212;
 }
 else if(var >= 190 && var < 200){
   r = 0;
   g = 255;
   b = 0;
 }
 else if(var >= 200 && var < 210){
   r = 255;
   g = 255;
   b = 0;
 }
 else if(var >= 210 && var < 220){
   r = 255;
   g = 165;
   b = 0;
 }
 else if(var >= 220 && var < 230){
   r = 255;
   g = 20;
   b = 147;
 }
 else if(var >= 230 && var < 240){
   r = 255;
   g = 0;
   b = 0;
 }
 else if(var >= 240){
   // blink red
      if(on){
     on = false;
     r = 0;
     g = 0;
     b = 0;
   }
   else{
     on = true;
     r = 255;
     g = 0;
     b = 0;
   }
 }
 
 
 
 
   for(int i=0;i<NUM_LED;i++)
  {
    b_led.setPixelColor(i,b_led.Color(r,g,b));
  }
  b_led.show();
  
}





