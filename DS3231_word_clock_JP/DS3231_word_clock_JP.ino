/* 
 *  Code by Moritz v. Sivers
 *  requires libraries: 
 *    Adafruit GFX, NeoPixel, NeoMatrix 
 *    DS1307RTC 
 *    Wire
 *    Time
*/

// load libraries
#include <stdio.h>
#include <DS1307RTC.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Wire.h>
#include <Time.h>
#include <TimeLib.h> 

#define DS3231_ADDRESS 0x68

// variables for LED strip
#define PIN            9
#define NUMPIXELS     114
#define BRIGHTNESS    127

// pins for buttons
#define PIN_hour    10
#define PIN_min     11
//#define PIN_on      4

// pins for analog inputs
//#define PIN_LDR   0
//#define PIN_poti  1

// threshold for poti
//#define POTI_THRES  50

// minimum, maximum brightness of clock
//#define BRIGHTNESS_MIN  20 
//#define BRIGHTNESS_MAX  255

// setup LED strip
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800); 


// MATRIX DECLARATION:
// Parameter 1 = width of NeoPixel matrix
// Parameter 2 = height of matrix
// Parameter 3 = pin number (most are valid)
// Parameter 4 = matrix layout flags, add together as needed:
//   NEO_MATRIX_TOP, NEO_MATRIX_BOTTOM, NEO_MATRIX_LEFT, NEO_MATRIX_RIGHT:
//     Position of the FIRST LED in the matrix; pick two, e.g.
//     NEO_MATRIX_TOP + NEO_MATRIX_LEFT for the top-left corner.
//   NEO_MATRIX_ROWS, NEO_MATRIX_COLUMNS: LEDs are arranged in horizontal
//     rows or in vertical columns, respectively; pick one or the other.
//   NEO_MATRIX_PROGRESSIVE, NEO_MATRIX_ZIGZAG: all rows/columns proceed
//     in the same order, or alternate lines reverse direction; pick one.
//   See example below for these values in action.
// Parameter 5 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(11, 10, PIN,
  NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
  NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
  NEO_GRB            + NEO_KHZ800);

// minute of last display update
int lastmin;

time_t c_time; // structure for current time

//int onbutton; // button to switch LEDs on
//int brightness_lamp; // brightness of white LEDs
//int brightness_clock; // brightness of clock LEDs

//int freePixel[NUMPIXELS]; // 1 if pixel is not in use by clock otherwise 0

//Actual words as array variables
int ES[] = {10,9,-1};
int IST[] = {7,6,5,-1};
int FUENF_M[] = {3,2,1,0,-1};
int ZEHN_M[] = {11,12,13,14,-1};
int ZWANZIG[] = {15,16,17,18,19,20,21,-1};
int DREIVIERTEL[] = {32,31,30,29,28,27,26,25,24,23,22,-1};
int VIERTEL[] = {28,27,26,25,24,23,22,-1};
int VOR[] = {33,34,35,-1};
int NACH[] = {40,41,42,43,-1};
int HALB[] = {54,53,52,51,-1};
int ELF[] = {49,48,47,-1};
int FUENF_H[] = {47,46,45,44,-1};
int EIN[] = {55,56,57,-1};
int EINS[] = {55,56,57,58,-1};
int ZWEI[] = {62,63,64,65,-1};
int DREI[] = {76,75,74,73,-1};
int VIER[] = {69,68,67,66,-1};
int SECHS[] = {77,78,79,80,81,-1};
int ACHT[] = {84,85,86,87,-1};
int SIEBEN[] = {93,94,95,96,97,98,-1};
int ZWOELF[] = {92,91,90,89,88,-1};
int ZEHN_H[] = {99,100,101,102,-1};
int NEUN[] = {102,103,104,105,-1};
int UHR[] = {107,108,109,-1};


// setup everything
//
void setup()
{
  Wire.begin();
  pixels.begin();
  pixels.setBrightness(BRIGHTNESS);
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(BRIGHTNESS);
  Serial.begin(9600);
  pinMode(PIN_hour, INPUT_PULLUP);
  pinMode(PIN_min,INPUT_PULLUP);

  blank();
  
  test(); // LED test

  c_time = RTC.get(); // read current RTC time
  updateDisplay(false); // show time on display

  lastmin = minute(c_time); // initialize last update of display

}

//main loop
//
void loop()
{

  // readout time and check for pressed buttons
  c_time = RTC.get();
  readButtons();

    // show temperature on second 30 every two minutes
    if(abs(second(c_time)-30)<1 && minute(c_time)%2==0) {
      displayTemp();
      updateDisplay(false);
    }

    // update display every minute
    if((minute(c_time) > lastmin) || (minute(c_time) == 0 && lastmin == 59)) {
      updateDisplay(false);
      lastmin = minute(c_time);
    }
  

  //delay(1000); // without buttons it is okay to update every second

}


// shows time on LED strip
//
void updateDisplay(bool timeset) {

  // number of dots 
  int ndots = (minute(c_time) % 5);

  // choose display effect
  int effect;
  if (timeset) {          // no effect when setting time
    effect = 0;
  }
  else {
    if (ndots==0) {       // scrolling effect every 5 minutes
      effect = 1;
    }
    else {
      effect = 2; // typing effect otherwise
    }
  }

  // blank all pixels
  blank();
  
   // for "typing" or no effect light up "ES IST" first
  if (effect == 2 || effect == 0) {
    lightup(ES,effect);
    lightup(IST,effect);
  }

  // light up minutes and hours
  switch (minute(c_time) / 5) {
                case 0:
                    // glatte Stunde
                    if (effect == 1) {
                      gocrazy();
                      blank();
                      lightup(UHR,effect);
                      setHours(hour(c_time),true,effect);
                    }
                    else {
                      setHours(hour(c_time),true,effect);
                      lightup(UHR,effect);
                    }
                    
                    break;
                case 1:
                    // 5 nach
                    if (effect == 1) {
                      setHours(hour(c_time),false,effect);
                      lightup(NACH,effect);
                      lightup(FUENF_M,effect);
                    }
                    else {
                      lightup(FUENF_M,effect);
                      lightup(NACH,effect);
                      setHours(hour(c_time),false,effect);
                    }
                    break;
                case 2:
                    // 10 nach
                    if (effect == 1) {
                      setHours(hour(c_time),false,effect);
                      lightup(NACH,effect);
                      lightup(ZEHN_M,effect);
                    }
                    else {
                      lightup(ZEHN_M,effect);
                      lightup(NACH,effect);
                      setHours(hour(c_time),false,effect);
                    }
                    break;
                case 3:
                    // viertel nach
                    if (effect == 1) {
                      setHours(hour(c_time),false,effect);
                      lightup(NACH,effect);
                      lightup(VIERTEL,effect);
                    }
                    else {
                      lightup(VIERTEL,effect);
                      lightup(NACH,effect);
                      setHours(hour(c_time),false,effect);
                    }
                    
                    break;
                case 4:
                    // 20 nach
                     if (effect == 1) {
                      setHours(hour(c_time),false,effect);
                      lightup(NACH,effect);
                      lightup(ZWANZIG,effect);
                     }
                     else {
                      lightup(ZWANZIG,effect);
                      lightup(NACH,effect);
                      setHours(hour(c_time),false,effect);
                     }
                    break;
                case 5:
                    // 5 vor halb
                     if (effect == 1) {
                      setHours(hour(c_time) + 1,false,effect);
                      lightup(HALB,effect);
                      lightup(VOR,effect);
                      lightup(FUENF_M,effect);
                     }
                     else {
                      lightup(FUENF_M,effect);
                      lightup(VOR,effect);
                      lightup(HALB,effect);
                      setHours(hour(c_time) + 1,false,effect);
                     }
                   
                    break;
                case 6:
                    // halb
                    if (effect == 1) {
                      setHours(hour(c_time) + 1,false,effect);
                      lightup(HALB,effect);
                    }
                    else {
                      lightup(HALB,effect);
                      setHours(hour(c_time) + 1,false,effect);
                    }
                    break;
                case 7:
                    // 5 nach halb
                    if (effect == 1) {
                      setHours(hour(c_time) + 1,false,effect);
                      lightup(HALB,effect);
                      lightup(NACH,effect);
                      lightup(FUENF_M,effect);
                    }
                    else {
                      lightup(FUENF_M,effect);
                      lightup(NACH,effect);
                      lightup(HALB,effect);
                      setHours(hour(c_time) + 1,false,effect);
                    }
                    
                    break;
                case 8:
                    // 20 vor
                     if (effect == 1) {
                      setHours(hour(c_time) + 1,false,effect);
                      lightup(VOR,effect);
                      lightup(ZWANZIG,effect);
                     }
                     else {
                      lightup(ZWANZIG,effect);
                      lightup(VOR,effect);
                      setHours(hour(c_time) + 1,false,effect);
                     }
                    break;
                case 9:
                    // viertel vor
                    if (effect == 1) {
                      setHours(hour(c_time) + 1,false,effect);
                      lightup(VOR,effect);
                      lightup(VIERTEL,effect);
                    }
                    else {
                      lightup(VIERTEL,effect);
                      lightup(VOR,effect);
                      setHours(hour(c_time) + 1,false,effect);
                    }
                    break;
                case 10:
                    // 10 vor
                    if (effect == 1) {
                      setHours(hour(c_time) + 1,false,effect);
                      lightup(VOR,effect);
                      lightup(ZEHN_M,effect);
                    }
                    else {
                      lightup(ZEHN_M,effect);
                      lightup(VOR,effect);
                      setHours(hour(c_time) + 1,false,effect);
                    }
                    break;
                case 11:
                    // 5 vor
                    if (effect == 1) {
                      setHours(hour(c_time) + 1,false,effect);
                      lightup(VOR,effect);
                      lightup(FUENF_M,effect);
                    }
                    else {
                      lightup(FUENF_M,effect);
                      lightup(VOR,effect);
                      setHours(hour(c_time) + 1,false,effect);
                    }
                    break;
            }

  // for scrolling effect light up "ES IST" last
  if (effect == 1) {
    lightup(ES,effect);
    lightup(IST,effect);
  }

  // always light up dots last
  setDots(ndots, timeset);


  // print time to serial monitor
  //printTime();
}


// reads if buttons are pressed
//

void readButtons() {

  int minbutton = digitalRead(PIN_min);
  int hourbutton = digitalRead(PIN_hour);

  if ((minbutton == LOW) && (hourbutton == LOW)) {
    test();
  }

  int minute_new;
  int hour_new;

  if (minbutton == LOW) {
  //Serial.println("Minute button pressed");
  if (minute(c_time) == 59) {
    minute_new = 0;
    hour_new = hour(c_time)+1;
    }
  else {
    minute_new = minute(c_time)+1;
    }
    setTime(hour(c_time),minute_new,second(c_time),day(c_time),month(c_time),year(c_time));
    c_time = now();
    RTC.set(c_time);
    updateDisplay(true);
    lastmin = minute(c_time);
  }
  
if (hourbutton == LOW) {
  //Serial.println("Hour button pressed");
    if (hour(c_time) == 23) {
      hour_new = 0; 
    }
    else { 
      hour_new = hour(c_time)+1;
    }  
    setTime(hour_new,minute(c_time),second(c_time),day(c_time),month(c_time),year(c_time));
    c_time = now();
    RTC.set(c_time);
    updateDisplay(true);
    lastmin = minute(c_time);
    }

  delay(200);

}





// print time to serial monitor
//
void printTime() {
  char buf[50];
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
           year(c_time), month(c_time), day(c_time),
           hour(c_time), minute(c_time), second(c_time));

  // Print the formatted string to serial so we can see the time.
  Serial.println(buf);
}



// read temperature from DS3231
//
float tempDS3231() {
  float temp;
  int msb, lsb;
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write(0x11); // DS3231 Register zu 11h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_ADDRESS, 2); // 2 Byte Daten vom DS3231 holen
  msb = Wire.read();
  lsb = Wire.read();
  temp=((msb << 2) + (lsb >> 6) ) /4.0;
  return temp;
}

// display temperature as scrolling text
//
void displayTemp() {
  int temp = (int) tempDS3231()-5;
  char buf[6];
  sprintf(buf, "%d", temp); 
  int red = map(temp, 10, 30, 0, 255);
  int blue = map(temp, 30, 10, 0, 255);
  const uint16_t color =  matrix.Color(red, 0, blue);
  matrix.setTextColor(color); 
  for (int i=0; i<2; i++) {
  int  x = matrix.width();
    while (x>-20) {
      matrix.fillScreen(0); // 
      matrix.setCursor(x, 2);
      matrix.print(buf);
      matrix.fillRect(x+12, 2, 2, 2, color);
      matrix.setCursor(x+15, 2);
      matrix.print("C");
      matrix.show();
      x--;
      delay(100);
    }
  }
  
}

// light up hours on display
//
void setHours(byte c_hour, boolean glatt, int effect) {
    switch (c_hour) {
                case 0:
                case 12:
                case 24:
                    lightup(ZWOELF,effect);
                    break;
                case 1:
                case 13:
                    if (glatt) {
                        lightup(EIN,effect);
                    } else {
                        lightup(EINS,effect);
                    }
                    break;
                case 2:
                case 14:
                   lightup(ZWEI,effect);
                    break;
                case 3:
                case 15:
                    lightup(DREI,effect);
                    break;
                case 4:
                case 16:
                    lightup(VIER,effect);
                    break;
                case 5:
                case 17:
                    lightup(FUENF_H,effect);
                    break;
                case 6:
                case 18:
                    lightup(SECHS,effect);
                    break;
                case 7:
                case 19:
                    lightup(SIEBEN,effect);
                    break;
                case 8:
                case 20:
                    lightup(ACHT,effect);
                    break;
                case 9:
                case 21:
                    lightup(NEUN,effect);
                    break;
                case 10:
                case 22:
                    lightup(ZEHN_H,effect);
                    break;
                case 11:
                case 23:
                    lightup(ELF,effect);
                    break;
            }            
}

// light up the dots
//
void setDots(int nDots, bool timeset) {
  //Serial.print("Number of dots: ");
  //Serial.println(nDots);
  uint32_t Colour = pixels.Color(random(0,255),random(0,255),random(0,255)); //choose random color
  for (int i = 0; i < nDots; i++) {
    pixels.setPixelColor(113-i, Colour);
    pixels.show();
    if (!timeset) delay(50);
    //Serial.print("lighting up pixel ");
    //Serial.println(i);
  }
  
}

// light up word on display
//
void lightup(int Word[], int effect) {
  uint32_t Colour = pixels.Color(random(0,255),random(0,255),random(0,255)); //choose random color
  for (int i = 0; i < 12; i++) {
    if(Word[i]==-1) break;
    if (effect==2) {
      pixels.setPixelColor(Word[i], Colour);
      pixels.show();
      delay(50);
    }
    else if (effect==1) {
      int row = Word[i]/11;
      int column;
      if ((row % 2) == 0) {
        column = Word[i]-row*11;
      }
      else {
        column = row*11+10-Word[i];
      }
      for (int j=0; j<row; j++) {
        int x;
        if ((j%2) == 0) {
          x = column+j*11;
        }
        else {
          x = j*11+10-column;
        }
        pixels.setPixelColor(x, Colour);
        pixels.show();
        delay(30);
        pixels.setPixelColor(x, pixels.Color(0,0,0));   // 
        pixels.show();
      }
      pixels.setPixelColor(Word[i], Colour);
      pixels.show();
    }
    else {
      pixels.setPixelColor(Word[i], Colour);
      pixels.show();
    }
  }
}



// clear all pixels
//
void blank() {

  uint32_t Colour = pixels.Color(0,0,0); // 

  for (int i = 0; i < NUMPIXELS; ++i) {
    pixels.setPixelColor(i, Colour);
  }
  
  pixels.show();

}



// wipe pixels
//
void wipe() {

  for (int x = 0; x < NUMPIXELS; ++x) {
    pixels.setPixelColor(x, pixels.Color(255,0,0));
    delay(10);
    pixels.show();
    }
  delay(50);  
  for (int x = NUMPIXELS; x > -1; --x) {
    pixels.setPixelColor(x, pixels.Color(0,0,0));
    delay(10);
    pixels.show();
  }
  
   for (int x = 0; x < NUMPIXELS; ++x) {
    pixels.setPixelColor(x, pixels.Color(0,255,0));
    delay(10);
    pixels.show();
    }
  delay(50);  
  for (int x = NUMPIXELS; x > -1; --x) {
    pixels.setPixelColor(x, pixels.Color(0,0,0));
    delay(10);
    pixels.show();
  }
  
   for (int x = 0; x < NUMPIXELS; ++x) {
    pixels.setPixelColor(x, pixels.Color(0,0,255));
    delay(10);
    pixels.show();
   }
  delay(50);  
  for (int x = NUMPIXELS; x > -1; --x) {
    pixels.setPixelColor(x, pixels.Color(0,0,0));
    delay(10);
    pixels.show();
  }
  
  blank();

}


// display test
//
void test() {
  //Serial.println("Running LED test...");
  blank();
  wipe();
  blank();
  gocrazy();
  blank();
}

// light up pixels randomly
//
void gocrazy() {
  for(int i=0; i<50; i++) {
    int n = random(0,NUMPIXELS);
    for (int j=0; j<n; j++) {
      int x = random(0,NUMPIXELS);
      uint32_t Colour = pixels.Color(random(0,255),random(0,255),random(0,255)); //choose random color
      pixels.setPixelColor(x, Colour);
    }
    pixels.show();
    int t = random(0,10);
    delay(t);
    n = random(0,NUMPIXELS);
    for (int j=0; j<n; j++) {
      int x = random(0,NUMPIXELS);
      pixels.setPixelColor(x, pixels.Color(0,0,0));
    }
    pixels.show();
  }
}
