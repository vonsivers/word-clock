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
#include <FastLED.h>
#include <Wire.h>
#include <Time.h>
#include <TimeLib.h> 

#define DS3231_ADDRESS 0x68

// variables for LED strip
#define PIN            9
#define NUMPIXELS     114
#define BRIGHTNESS    255

// pins for buttons
#define PIN_hour    2
#define PIN_min     3
#define PIN_on      4

// pins for analog inputs
#define PIN_LDR   0
#define PIN_poti  1

// threshold for poti
#define POTI_THRES  50

// minimum, maximum brightness of clock
//#define BRIGHTNESS_MIN  100 
//#define BRIGHTNESS_MAX  255

// setup LED strip
//Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800); 
// Define the array of leds
CRGB leds[NUMPIXELS];


// minute of last display update
int lastmin;

time_t c_time; // structure for current time

int onbutton; // button to switch LEDs on
int brightness_clock = 127;
//int brightness_lamp; // brightness of white LEDs
//int brightness_clock_old = 255; // previous brightness of clock LEDs
//int brightness_clock_new = 255; // new brightness of clock LEDs
int colour1 = random(0,255); // color of words
int colour2 = random(0,255); 
int colour3 = random(0,255); 
int colour4 = random(0,255); 
int colour5 = random(0,255);
int colour6 = random(0,255);
int colour_dots = random(0,255); // color of dots 

int freePixel[NUMPIXELS]; // 1 if pixel is not in use by clock otherwise 0

//Actual words as array variables
int ES[] = {0,1,-1};
int IST[] = {3,4,5,-1};
int FUENF_M[] = {7,8,9,10,-1};
int ZEHN_M[] = {21,20,19,18,-1};
int ZWANZIG[] = {17,16,15,14,13,12,11,-1};
int VIERTEL[] = {26,27,28,29,30,31,32,-1};
int VOR[] = {43,42,41,-1};
int NACH[] = {36,35,34,33,-1};
int HALB[] = {44,45,46,47,-1};
int ELF[] = {49,50,51,-1};
int FUENF_H[] = {51,52,53,54,-1};
int EIN[] = {65,64,63,-1};
int EINS[] = {65,64,63,62,-1};
int ZWEI[] = {58,57,56,55,-1};
int DREI[] = {66,67,68,69,-1};
int VIER[] = {73,74,75,76,-1};
int SECHS[] = {87,86,85,84,83,-1};
int ACHT[] = {80,79,78,77,-1};
int SIEBEN[] = {88,89,90,91,92,93,-1};
int ZWOELF[] = {94,95,96,97,98,-1};
int ZEHN_H[] = {109,108,107,106,-1};
int NEUN[] = {106,105,104,103,-1};
int UHR[] = {101,100,99,-1};


// setup everything
//
void setup()
{
  Wire.begin();
  FastLED.addLeds<WS2812B, PIN, GRB>(leds, NUMPIXELS);
  FastLED.setBrightness(BRIGHTNESS);
  //FastLED.setTemperature(HighPressureSodium); 
  Serial.begin(9600);
  pinMode(PIN_hour, INPUT_PULLUP);
  pinMode(PIN_min,INPUT_PULLUP);
  pinMode(PIN_on,INPUT_PULLUP);

  LEDsOff();
  
  c_time = RTC.get(); // read current RTC time
  updateDisplay(0); // show time on display

  lastmin = minute(c_time); // initialize last update of display

}

//main loop
//
void loop()
{

  // readout time and check for pressed buttons
  c_time = RTC.get();
  readButtons();

  // check if LEDs are switched on
  if (onbutton==LOW) {
    readPoti();
    //readLDR();

    //lightLamp(); // light up lamp
    //updateClockBrightness(); // update clock brightness

    // change time  every minute
    if((minute(c_time) > lastmin) || (minute(c_time) == 0 && lastmin == 59)) {
      colour1 = random(0,255); // color of words
      colour2 = random(0,255); 
      colour3 = random(0,255); 
      colour4 = random(0,255); 
      colour5 = random(0,255);
      colour_dots = random(0,255); // color of dots
      updateDisplay(2);
      lastmin = minute(c_time);
    }
    else {
      updateDisplay(0);
    }
  }
  else {      // LEDs are switched off
    LEDsOff();
  }

}


// shows time on LED strip
//
void updateDisplay(int property) {

  // number of dots 
  int ndots = (minute(c_time) % 5);

  // choose display effect
  int effect;
  if (property<2) {          // no effect when setting time
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
  if (property>0)  blank();
  
   // for "typing" or no effect light up "ES IST" first
  if (effect == 2 || effect == 0) {
    lightup(ES,colour1,effect);
    lightup(IST,colour2,effect);
  }

  // light up minutes and hours
  switch (minute(c_time) / 5) {
                case 0:
                    // glatte Stunde
                    if (effect == 1) {
                      lightup(UHR,colour4,effect);
                      setHours(hour(c_time),true,colour3,effect);
                    }
                    else {
                      setHours(hour(c_time),true,colour3,effect);
                      lightup(UHR,colour4,effect);
                    }
                    
                    break;
                case 1:
                    // 5 nach
                    if (effect == 1) {
                      setHours(hour(c_time),false,colour5,effect);
                      lightup(NACH,colour4,effect);
                      lightup(FUENF_M,colour3,effect);
                    }
                    else {
                      lightup(FUENF_M,colour3,effect);
                      lightup(NACH,colour4,effect);
                      setHours(hour(c_time),false,colour5,effect);
                    }
                    break;
                case 2:
                    // 10 nach
                    if (effect == 1) {
                      setHours(hour(c_time),false,colour5,effect);
                      lightup(NACH,colour4,effect);
                      lightup(ZEHN_M,colour3,effect);
                    }
                    else {
                      lightup(ZEHN_M,colour5,effect);
                      lightup(NACH,colour4,effect);
                      setHours(hour(c_time),false,colour3,effect);
                    }
                    break;
                case 3:
                    // viertel nach
                    if (effect == 1) {
                      setHours(hour(c_time),false,colour5,effect);
                      lightup(NACH,colour4,effect);
                      lightup(VIERTEL,colour3,effect);
                    }
                    else {
                      lightup(VIERTEL,colour5,effect);
                      lightup(NACH,colour4,effect);
                      setHours(hour(c_time),false,colour3,effect);
                    }
                    
                    break;
                case 4:
                    // 20 nach
                     if (effect == 1) {
                      setHours(hour(c_time),false,colour3,effect);
                      lightup(NACH,colour4,effect);
                      lightup(ZWANZIG,colour5,effect);
                     }
                     else {
                      lightup(ZWANZIG,colour5,effect);
                      lightup(NACH,colour4,effect);
                      setHours(hour(c_time),false,colour3,effect);
                     }
                    break;
                case 5:
                    // 5 vor halb
                     if (effect == 1) {
                      setHours(hour(c_time) + 1,false,colour3,effect);
                      lightup(HALB,colour5,effect);
                      lightup(VOR,colour4,effect);
                      lightup(FUENF_M,colour6,effect);
                     }
                     else {
                      lightup(FUENF_M,colour6,effect);
                      lightup(VOR,colour4,effect);
                      lightup(HALB,colour5,effect);
                      setHours(hour(c_time) + 1,false,colour3,effect);
                     }
                   
                    break;
                case 6:
                    // halb
                    if (effect == 1) {
                      setHours(hour(c_time) + 1,false,colour3,effect);
                      lightup(HALB,colour5,effect);
                    }
                    else {
                      lightup(HALB,colour5,effect);
                      setHours(hour(c_time) + 1,false,colour3,effect);
                    }
                    break;
                case 7:
                    // 5 nach halb
                    if (effect == 1) {
                      setHours(hour(c_time) + 1,false,colour3,effect);
                      lightup(HALB,colour5,effect);
                      lightup(NACH,colour4,effect);
                      lightup(FUENF_M,colour6,effect);
                    }
                    else {
                      lightup(FUENF_M,colour6,effect);
                      lightup(NACH,colour4,effect);
                      lightup(HALB,colour5,effect);
                      setHours(hour(c_time) + 1,false,colour3,effect);
                    }
                    
                    break;
                case 8:
                    // 20 vor
                     if (effect == 1) {
                      setHours(hour(c_time) + 1,false,colour3,effect);
                      lightup(VOR,colour4,effect);
                      lightup(ZWANZIG,colour5,effect);
                     }
                     else {
                      lightup(ZWANZIG,colour5,effect);
                      lightup(VOR,colour4,effect);
                      setHours(hour(c_time) + 1,false,colour3,effect);
                     }
                    break;
                case 9:
                    // viertel vor
                    if (effect == 1) {
                      setHours(hour(c_time) + 1,false,colour3,effect);
                      lightup(VOR,colour4,effect);
                      lightup(VIERTEL,colour5,effect);
                    }
                    else {
                      lightup(VIERTEL,colour5,effect);
                      lightup(VOR,colour4,effect);
                      setHours(hour(c_time) + 1,false,colour3,effect);
                    }
                    break;
                case 10:
                    // 10 vor
                    if (effect == 1) {
                      setHours(hour(c_time) + 1,false,colour3,effect);
                      lightup(VOR,colour4,effect);
                      lightup(ZEHN_M,colour5,effect);
                    }
                    else {
                      lightup(ZEHN_M,colour5,effect);
                      lightup(VOR,colour4,effect);
                      setHours(hour(c_time) + 1,false,colour3,effect);
                    }
                    break;
                case 11:
                    // 5 vor
                    if (effect == 1) {
                      setHours(hour(c_time) + 1,false,colour3,effect);
                      lightup(VOR,colour4,effect);
                      lightup(FUENF_M,colour5,effect);
                    }
                    else {
                      lightup(FUENF_M,colour5,effect);
                      lightup(VOR,colour4,effect);
                      setHours(hour(c_time) + 1,false,colour3,effect);
                    }
                    break;
            }

  // for scrolling effect light up "ES IST" last
  if (effect == 1) {
    lightup(ES,colour1,effect);
    lightup(IST,colour2,effect);
  }

  // always light up dots last
  setDots(ndots, property);


  // print time to serial monitor
  //printTime();
}


// reads if buttons are pressed
//

void readButtons() {

  int minbutton = digitalRead(PIN_min);
  int hourbutton = digitalRead(PIN_hour);
  onbutton = digitalRead(PIN_on);

  if (minbutton == LOW) {
    adjustTime(60);
    c_time = now();
    RTC.set(c_time);
    updateDisplay(1);
    lastmin = minute(c_time);
  }
  
  if (hourbutton == LOW) {
    adjustTime(3600);
    c_time = now();
    RTC.set(c_time);
    updateDisplay(1);
    lastmin = minute(c_time);
   }

  delay(200);

}

// read potentiometer and set brightness of lamp
//
void readPoti() {
  
  int poti = analogRead(PIN_poti);
  
  brightness_clock = map(poti,0,1023,0,255);
 
  //Serial.println(brightness_clock);
  
/*
  

  if (poti>POTI_THRES) {
    brightness_lamp = map(poti,POTI_THRES,1023,0,255); // map analog input values (0-1024) to brightness level (0-255)
  }
  else {
    brightness_lamp = 0;
  }
  */
  
}

// read LDR and adjust brightness of clock
//
/*
void readLDR() {

  int ldr = analogRead(PIN_LDR);
  brightness_clock_new = map(ldr,0,1023,BRIGHTNESS_MIN,BRIGHTNESS_MAX); // map analog input values to brightness level
  
}

*/

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





// light up hours on display
//
void setHours(byte c_hour, boolean glatt, int colour, int effect) {
    switch (c_hour) {
                case 0:
                case 12:
                case 24:
                    lightup(ZWOELF,colour,effect);
                    break;
                case 1:
                case 13:
                    if (glatt) {
                        lightup(EIN,colour,effect);
                    } else {
                        lightup(EINS,colour,effect);
                    }
                    break;
                case 2:
                case 14:
                   lightup(ZWEI,colour,effect);
                    break;
                case 3:
                case 15:
                    lightup(DREI,colour,effect);
                    break;
                case 4:
                case 16:
                    lightup(VIER,colour,effect);
                    break;
                case 5:
                case 17:
                    lightup(FUENF_H,colour,effect);
                    break;
                case 6:
                case 18:
                    lightup(SECHS,colour,effect);
                    break;
                case 7:
                case 19:
                    lightup(SIEBEN,colour,effect);
                    break;
                case 8:
                case 20:
                    lightup(ACHT,colour,effect);
                    break;
                case 9:
                case 21:
                    lightup(NEUN,colour,effect);
                    break;
                case 10:
                case 22:
                    lightup(ZEHN_H,colour,effect);
                    break;
                case 11:
                case 23:
                    lightup(ELF,colour,effect);
                    break;
            }            
}

// light up the dots
//
void setDots(int nDots, int property) {
  //Serial.print("Number of dots: ");
  //Serial.println(nDots);
  //uint32_t Colour = random(0,255); //choose random color
  for (int i = 0; i < nDots; i++) {
    freePixel[110+i] = 0;
    leds[110+i] = CHSV(colour_dots,255,brightness_clock);
    FastLED.show();
    if (property==2) delay(50);
    //Serial.print("lighting up pixel ");
    //Serial.println(i);
  }
  
}

// light up word on display
//
void lightup(int Word[],int Colour, int effect) {
  //uint32_t Colour = random(0,255); //choose random color
  for (int i = 0; i < 12; i++) {
    if(Word[i]==-1) break;
    freePixel[Word[i]] = 0;   // mark pixel as occupied
    if (effect==2) {
      leds[Word[i]] = CHSV(Colour,255,brightness_clock);
      FastLED.show();
      delay(50);
    }
    else if (effect==1) {
      int row = Word[i]/11;
      int column;
      if ((row % 2) == 1) {
        column = Word[i]-row*11;
      }
      else {
        column = row*11+10-Word[i];
      }
      for (int j=0; j<row; j++) {
        int x;
        if ((j%2) == 1) {
          x = column+j*11;
        }
        else {
          x = j*11+10-column;
        }
        leds[x] = CHSV(Colour,255,brightness_clock);
        FastLED.show();
        delay(30);
        leds[x] = CRGB::Black;   // change color back to white with defined brightness
        FastLED.show();
      }
      leds[Word[i]] = CHSV(Colour,255,brightness_clock);
      FastLED.show();
    }
    else {
      leds[Word[i]] = CHSV(Colour,255,brightness_clock);
      FastLED.show();
    }
  }
}

// light up the lamp LEDs
//
/*
void lightLamp() {

  for (int i=0; i<NUMPIXELS-4; i++) {   // iterate through all pixels, dots are not used for lamp
    if (freePixel[i]) {  // check if pixel is not used for clock
      leds[i] = CRGB(brightness_lamp,brightness_lamp,brightness_lamp);
    }
  }
  FastLED.show();
 
}
*/

/*
void updateClockBrightness() {
  for (int i=0;i<NUMPIXELS;i++) {
    if (freePixel[i] == 0) { // pixel belongs to clock
      leds[i]+= brightness_clock_new - brightness_clock_old;
      //leds[i] *= scaling;
      Serial.print("brightness old: ");
      Serial.println(brightness_clock_old);
      Serial.print("brightness new: ");
      Serial.println(brightness_clock_new);
    }
  }
  brightness_clock_old = brightness_clock_new;
}
*/

// clear all pixels
//
void blank() {

  // turn all letters to white
  for (int i = 0; i < NUMPIXELS-4; ++i) {
    leds[i] = CRGB::Black;
    freePixel[i] = 1;                     // mark pixel as free
  }
  // turn dots black
  for (int i=NUMPIXELS-4; i<NUMPIXELS; i++) {
    leds[i] = CRGB::Black;
  }
  
  FastLED.show();

}

void LEDsOff() {

  for (int i = 0; i < NUMPIXELS; ++i) {
    leds [i] = CRGB::Black;
    freePixel[i] = 1;                     // mark pixel as free
  }
  
  FastLED.show();
}
