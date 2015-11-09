// Thermo light
// Temperature notification by lights
// BMP180 temp
// led WS2812b
// Oled 0.96 screen
// RTC

// blue = cold 16 deg C
// red = hot 24 deg C

// september 2015
// b. Wentzler

//----------------------------INCLUDES-----------------------------
#include <Wire.h>  // i2c communication
#include <Adafruit_NeoPixel.h>  // to control WS2812b
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>  //OLED display




//---------------------------DEFINE--------------------------------

// BMP180 (doc bosh BMP180.pdf)
#define BMP_ADRESS 0x77  // module address bmp180 1110111
#define AC6 0xB4  // adresse  MSB for temp calculation
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
// var  BMP180 eeprom
long ac6,ac5,mc,md;


long temp =0, ut = 0;
float temp2 = 0;
unsigned char msb, lsb;

// RTC
String time = "";




//---------------------------PROTOTYPES--------------------------
// Function BMP 180
// get calibration datas from BMP180 eeprom
void bmp180_cal(long *, long *, long *, long *);
// get raw temp dats from BMP180
long bmp180_UT();
// temperature calculation
float calcTemp();

// get time from DS1307 and return as string
String getTime();


//  set WS2812b color according temp ex 18.25
void setColor(float temp);


// Set WS2812b and oled screen
Adafruit_NeoPixel b_led = Adafruit_NeoPixel(NUM_LED,DATA_PIN,NEO_GRB + NEO_KHZ800);
Adafruit_SSD1306 oled = Adafruit_SSD1306(OLED_SDA,OLED_SCL,OLED_DC,OLED_RST,7);

// *********************************************************************

void setup()
{
  Wire.begin(); // join i2c bus (address optional for master)
  oled.begin();
  Serial.begin(9600);
  
  pinMode(DATA_PIN, OUTPUT);
  b_led.begin();
  
  
  // get calibration datas from BMP180
  bmp180_cal(&ac6,&ac5,&mc,&md);
  
}

//******************************************************************

void loop()
{
  
  // get raw temp dats from BMP180
  ut = bmp180_UT();
  
  // calc temp
  temp2 = calcTemp();
  
  
  
  //Serial.print("Temperature: ");
  Serial.println(temp2,2);
  //Serial.println("deg C");
  
  //  set WS2812b color according temp ex 18.25
  setColor(temp2);
  
  time = getTime();
  
  // displays time and temp on Oled
  oled.clearDisplay();
  oled.setTextSize(3);
  //oled.setTextColor(WHITE);
  oled.setCursor(0,0);
  oled.print(" ");
  oled.print(time);
  oled.setTextColor(WHITE);
  oled.setCursor(10,40);
  oled.print(temp2,1);
  oled.println(" c");
  oled.display();
  
  delay(1000);
   
  

}
//********************************************************************************
// get time from DS1307 and return as string
String getTime(){
  
  String temp = "";
  byte sec = 0;
  byte secRaw = 0;
  byte secDix = 0;
  byte minuRaw = 0;
  byte minu = 0;
  byte minuDix = 0;
  byte hourRaw = 0;
  byte hour = 0;
  byte hourDix = 0;
  // require register 00h
  Wire.beginTransmission(0x68);
  Wire.write(0);
  Wire.endTransmission();
  
  // reading 3 byte sec,min,hour
  Wire.requestFrom(0x68,3);
  secRaw = Wire.read();
  minuRaw = Wire.read();
  hourRaw = Wire.read();
  
  // calc seconds   second method to decode BCD: (val/16*10) + (val%16)
  sec = B00001111 & secRaw;
  secDix = B11110000 & secRaw;
  secDix = secDix >> 4;
  
  // calc minutes
  minu = B00001111 & minuRaw;
  minuDix = B11110000 & minuRaw;
  minuDix = minuDix >> 4;
  
  // calc hours
  hour = B00001111 & hourRaw;
  hourDix = B11110000 & hourRaw;
  hourDix = hourDix >> 4;
  
  
  
  temp += hourDix;
  temp += hour;
  temp += "h";
  temp += minuDix;
  temp += minu;
  //temp += ":";
  //temp += secDix;
  //temp += sec;
  return temp;
  
}

//*************************************************************************************



// get calibration datas from BMP180 eeprom
void bmp180_cal(long *ac6=0, long *ac5=0, long *mc=0, long *md=0)
{

    
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

//**********************************************************************
// get raw temp dats from BMP180
long bmp180_UT()
{
  
  long temp = 0;
    // begin transmission with write address of BMP180
  Wire.beginTransmission(BMP_ADRESS);
  //envoie de l adresse du registre a ecrire
  Wire.write(0xF4);
  // conf du registre
  Wire.write(0x2E);
  // envoie du paquet au BMP180
  Wire.endTransmission();
  // attente de 5mS avant lecture de temp
  delay(5);
  
  
  // read temperature
  
  // adresse du BMP180 en lecture
  Wire.beginTransmission(BMP_ADRESS);
  // adresse a lire MSB temp
  Wire.write(0xF6);
  // envoie du paquet
  Wire.endTransmission();
  
  
  // 2 bytes request
  Wire.requestFrom(BMP_ADRESS, 2);
  // waiting for 2 bytes
  while(Wire.available()<2);;
  msb = Wire.read();
  lsb = Wire.read();
  temp = (long) msb<<8 | lsb;
  return temp;
  
}

//***********************************************************************

// temperature calculation
float calcTemp(){
  long x1 = (ut-ac6) * (ac5 / pow(2,15));
  long x2 = mc * pow(2,11) / (x1 + md);
  long b5 = x1 + x2;
  float data = (b5 + 8) / pow(2,4);
  
  // AH ah must divide temp per 10
  data = data / 10;
  return data;
}

//**************************************************************************

//  set WS2812b color according temp ex 18.25
void setColor(float temp)
{
  int r = 128,g = 0,b = 0;
  int var = 0;
  boolean on = false;
  
  
  // j enleve la virgule en multipliant par dix et arrondi en int
  temp = temp * 10;
  var = (int) temp;
  
  
 // case tests
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
 
 
 
   // send RGB values to each leds
   for(int i=0;i<NUM_LED;i++)
  {
    b_led.setPixelColor(i,b_led.Color(r,g,b));
  }
  b_led.show();
  
}

//**************************************************************************



