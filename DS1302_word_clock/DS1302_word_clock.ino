/* 
 *  Code by Moritz v. Sivers
*/

// load libraries
#include <stdio.h>
#include <DS1302.h>
#include <Adafruit_NeoPixel.h>

// variables for LED strip
#define PIN            9
#define NUMPIXELS     114
#define BRIGHTNESS    127

// pins for buttons
#define PIN_hour    10
#define PIN_min     11

// setup LED strip
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// current time
int second, minute, hour, day, month, year; 

// minute of last display update
int lastmin;

// Set the appropriate digital I/O pin connections. These are the pin
// assignments for the Arduino as well for as the DS1302 chip. See the DS1302
// datasheet:
//
//   http://datasheets.maximintegrated.com/en/ds/DS1302.pdf
#define kCePin   5  // Chip Enable
#define kIoPin   6  // Input/Output
#define kSclkPin 7  // Serial Clock

// Create a DS1302 object.
DS1302 rtc(kCePin, kIoPin, kSclkPin);

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

int flag = 0; //used for display effects to stop it showing more than once

//define colours
uint32_t Black = pixels.Color(0,0,0);
uint32_t White = pixels.Color(255,255,255);
uint32_t Green = pixels.Color(0,255,0);
uint32_t Red = pixels.Color(255,0,0);
uint32_t Gold = pixels.Color(255,204,0);
uint32_t Grey = pixels.Color(30,30,30);
uint32_t Blue = pixels.Color(0,0,255);
//About colours
uint32_t whiteblue = pixels.Color(255,255,255);
uint32_t lightblue = pixels.Color(153,204,255);
uint32_t midblue = pixels.Color(0,102,204);
uint32_t darkblue = pixels.Color(0,0,255);
//coffee
uint32_t Brown = pixels.Color(153,102,051);
//ME!
uint32_t Pink = pixels.Color(255,153,153);


// setup everything
//
void setup()
{
  pixels.begin();
  Serial.begin(9600);
  pinMode(PIN_hour, INPUT);
  pinMode(PIN_min,INPUT);
  digitalWrite(PIN_hour, HIGH);
  digitalWrite(PIN_min, HIGH);
  blank();
  pixels.setBrightness(BRIGHTNESS);

   // Initialize a new chip by turning off write protection and clearing the
  // clock halt flag. These methods needn't always be called. See the DS1302
  // datasheet for details.
  rtc.writeProtect(false);
  rtc.halt(false);
  
  // set the initial time here
  //year = 2017;
  //month = 12;
  //day = 13;
  //hour = 0;
  //minute = 0;
  //second = 0;

  //setTime();

  //gocrazy(); //
  test();

  readTime(); // read current time
  updateDisplay(false); // show time on display

  lastmin = minute; // initialize last update of display

}

//main loop
//
void loop()
{

 //readout time and check for pressed buttons
  readTime();
  readButtons();

  // update display every minute
  if((minute > lastmin) || (minute == 0 && lastmin == 59)) {
    updateDisplay(false);
    lastmin = minute;
  }

}


// shows time on LED strip
//
void updateDisplay(bool timeset) {

  // number of dots 
  int ndots = (minute % 5);

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
  switch (minute / 5) {
                case 0:
                    // glatte Stunde
                    if (effect == 1) {
                      gocrazy();
                      blank();
                      lightup(UHR,effect);
                      setHours(hour,true,effect);
                    }
                    else {
                      setHours(hour,true,effect);
                      lightup(UHR,effect);
                    }
                    
                    break;
                case 1:
                    // 5 nach
                    if (effect == 1) {
                      setHours(hour,false,effect);
                      lightup(NACH,effect);
                      lightup(FUENF_M,effect);
                    }
                    else {
                      lightup(FUENF_M,effect);
                      lightup(NACH,effect);
                      setHours(hour,false,effect);
                    }
                    break;
                case 2:
                    // 10 nach
                    if (effect == 1) {
                      setHours(hour,false,effect);
                      lightup(NACH,effect);
                      lightup(ZEHN_M,effect);
                    }
                    else {
                      lightup(ZEHN_M,effect);
                      lightup(NACH,effect);
                      setHours(hour,false,effect);
                    }
                    break;
                case 3:
                    // viertel nach
                    if (effect == 1) {
                      setHours(hour,false,effect);
                      lightup(NACH,effect);
                      lightup(VIERTEL,effect);
                    }
                    else {
                      lightup(VIERTEL,effect);
                      lightup(NACH,effect);
                      setHours(hour,false,effect);
                    }
                    
                    break;
                case 4:
                    // 20 nach
                     if (effect == 1) {
                      setHours(hour,false,effect);
                      lightup(NACH,effect);
                      lightup(ZWANZIG,effect);
                     }
                     else {
                      lightup(ZWANZIG,effect);
                      lightup(NACH,effect);
                      setHours(hour,false,effect);
                     }
                    break;
                case 5:
                    // 5 vor halb
                     if (effect == 1) {
                      setHours(hour + 1,false,effect);
                      lightup(HALB,effect);
                      lightup(VOR,effect);
                      lightup(FUENF_M,effect);
                     }
                     else {
                      lightup(FUENF_M,effect);
                      lightup(VOR,effect);
                      lightup(HALB,effect);
                      setHours(hour + 1,false,effect);
                     }
                   
                    break;
                case 6:
                    // halb
                    if (effect == 1) {
                      setHours(hour + 1,false,effect);
                      lightup(HALB,effect);
                    }
                    else {
                      lightup(HALB,effect);
                      setHours(hour + 1,false,effect);
                    }
                    break;
                case 7:
                    // 5 nach halb
                    if (effect == 1) {
                      setHours(hour + 1,false,effect);
                      lightup(HALB,effect);
                      lightup(NACH,effect);
                      lightup(FUENF_M,effect);
                    }
                    else {
                      lightup(FUENF_M,effect);
                      lightup(NACH,effect);
                      lightup(HALB,effect);
                      setHours(hour + 1,false,effect);
                    }
                    
                    break;
                case 8:
                    // 20 vor
                     if (effect == 1) {
                      setHours(hour + 1,false,effect);
                      lightup(VOR,effect);
                      lightup(ZWANZIG,effect);
                     }
                     else {
                      lightup(ZWANZIG,effect);
                      lightup(VOR,effect);
                      setHours(hour + 1,false,effect);
                     }
                    break;
                case 9:
                    // viertel vor
                    if (effect == 1) {
                      setHours(hour + 1,false,effect);
                      lightup(VOR,effect);
                      lightup(VIERTEL,effect);
                    }
                    else {
                      lightup(VIERTEL,effect);
                      lightup(VOR,effect);
                      setHours(hour + 1,false,effect);
                    }
                    break;
                case 10:
                    // 10 vor
                    if (effect == 1) {
                      setHours(hour + 1,false,effect);
                      lightup(VOR,effect);
                      lightup(ZEHN_M,effect);
                    }
                    else {
                      lightup(ZEHN_M,effect);
                      lightup(VOR,effect);
                      setHours(hour + 1,false,effect);
                    }
                    break;
                case 11:
                    // 5 vor
                    if (effect == 1) {
                      setHours(hour + 1,false,effect);
                      lightup(VOR,effect);
                      lightup(FUENF_M,effect);
                    }
                    else {
                      lightup(FUENF_M,effect);
                      lightup(VOR,effect);
                      setHours(hour + 1,false,effect);
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


if (minbutton == LOW) {
  //Serial.println("Minute button pressed");
  if (minute == 59) {
    minute = 0;
    }
  else {
    minute++;
    }
    setTime();
    updateDisplay(true);
    lastmin = minute;
  }
  
if (hourbutton == LOW) {
  //Serial.println("Hour button pressed");
    if (hour == 23) {
      hour = 0; 
    }
    else { 
      hour++;
    }  
    setTime();
    updateDisplay(true);
    lastmin = minute;
    }

delay(200);

}


// print time to serial monitor
//
void printTime() {
  char buf[50];
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
           year, month, day,
           hour, minute, second);

  // Print the formatted string to serial so we can see the time.
  Serial.println(buf);
}


// set time in RTC
//
void setTime() {
 
  // Make a new time object to set the date and time.
  Time t(year, month, day, hour, minute, second, Time::kSunday);

  // Set the time and date on the chip.
  rtc.time(t);
}


// read time from RTC
//
void readTime() {
  // Get the current time and date from the chip.
  Time t = rtc.time();
  year = t.yr; 
  month = t.mon; 
  day = t.date;
  hour = t.hr;
  minute = t.min; 
  second = t.sec;
}

// light up hours on display
//
void setHours(byte hour, boolean glatt, int effect) {
    switch (hour) {
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
        pixels.setPixelColor(x, Black);
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

// light up word on display with typing effect
//
/*
void lightup_typing(int Word[]) {
  uint32_t Colour = pixels.Color(random(0,255),random(0,255),random(0,255)); //choose random color
  for (int i = 0; i < 12; i++) {
    if(Word[i]==-1) break;
   
    //Serial.print("lighting up pixel ");
    //Serial.println(Word[i]);
  } 
}

// light up word on display with scroll down effect
//
void lightup_scroll(int Word[]) {
  uint32_t Colour = pixels.Color(random(0,255),random(0,255),random(0,255)); //choose random color
  for (int i = 0; i < 12; i++) {
    if(Word[i]==-1) break;
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
      pixels.setPixelColor(x, Black);
      pixels.show();
    }
    pixels.setPixelColor(Word[i], Colour);
    pixels.show();
    //Serial.print("lighting up pixel ");
    //Serial.println(Word[i]);
  } 
}
*/

// clear all pixels
//
void blank() {
for (int x = 0; x < NUMPIXELS; ++x) {
  pixels.setPixelColor(x, Black);
}
  pixels.show();

}

// wipe pixels
//
void wipe() {

  for (int x = 0; x < NUMPIXELS; ++x) {
    pixels.setPixelColor(x, Blue);
    delay(10);
    pixels.show();
    }
  delay(50);  
  for (int x = NUMPIXELS; x > -1; --x) {
    pixels.setPixelColor(x, Black);
    delay(10);
    pixels.show();
  }
  
   for (int x = 0; x < NUMPIXELS; ++x) {
    pixels.setPixelColor(x, Green);
    delay(10);
    pixels.show();
    }
  delay(50);  
  for (int x = NUMPIXELS; x > -1; --x) {
    pixels.setPixelColor(x, Black);
    delay(10);
    pixels.show();
  }
  
   for (int x = 0; x < NUMPIXELS; ++x) {
    pixels.setPixelColor(x, Red);
    delay(10);
    pixels.show();
   }
  delay(50);  
  for (int x = NUMPIXELS; x > -1; --x) {
    pixels.setPixelColor(x, Black);
    delay(10);
    pixels.show();
  }
  
  blank();

}


// display test
//
void test() {
  Serial.println("Running LED test...");
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
      pixels.setPixelColor(x, Black);
    }
    pixels.show();
  }
}

// flash pixels
//
void flash() {

blank();
for (int y = 0; y< 10; ++y) {
    for (int x = 0; x < NUMPIXELS; x=x+2) {
      pixels.setPixelColor(x, Pink);
    }
    //pixels.setBrightness(BRIGHTNESS);
    pixels.show();
    delay(50);
    blank();
    delay(50);

    for (int x = 1; x < NUMPIXELS; x=x+2) {
      pixels.setPixelColor(x, Pink);  
    }
    //pixels.setBrightness(BRIGHTNESS);
    pixels.show();
    delay(50);
    blank();
    delay(50);
}
blank();
}

