/* 
 *  Code adapted from WhiteClockCompany WordClock2
*/

// load libraries
#include <stdio.h>
#include <Adafruit_NeoPixel.h>
#include <dcf77.h>


#if defined(__AVR__)
const uint8_t dcf77_analog_sample_pin = 5;
const uint8_t dcf77_sample_pin = A5;       // A5 == d19
const uint8_t dcf77_inverted_samples = 1;
const uint8_t dcf77_analog_samples = 1;
// const uint8_t dcf77_pin_mode = INPUT;  // disable internal pull up
const uint8_t dcf77_pin_mode = INPUT_PULLUP;  // enable internal pull up

const uint8_t dcf77_monitor_led = 18;  // A4 == d18

uint8_t ledpin(const uint8_t led) {
    return led;
}
#else
const uint8_t dcf77_sample_pin = 53;
const uint8_t dcf77_inverted_samples = 0;

// const uint8_t dcf77_pin_mode = INPUT;  // disable internal pull up
const uint8_t dcf77_pin_mode = INPUT_PULLUP;  // enable internal pull up

const uint8_t dcf77_monitor_led = 19;

uint8_t ledpin(const uint8_t led) {
    return led<14? led: led+(54-14);
}
#endif
 

// variables for LED strip
#define PIN            9
#define NUMPIXELS     114
#define BRIGHTNESS    127

// pins for buttons
//#define PIN_hour    10
//#define PIN_min     11

// setup LED strip
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800); 

// minute of last display update
int lastmin;

// structure to store time
Clock::time_t now;

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
  using namespace Clock;
  Serial.begin(9600);

  pinMode(ledpin(dcf77_monitor_led), OUTPUT);
  pinMode(dcf77_sample_pin, dcf77_pin_mode);

  DCF77_Clock::setup();
  DCF77_Clock::set_input_provider(sample_input_pin);

  pixels.begin();
  blank();
  pixels.setBrightness(BRIGHTNESS);
  
  test(); // LED test

    // Wait till clock is synced, depending on the signal quality this may take
    // rather long. About 5 minutes with a good signal, 30 minutes or longer
    // with a bad signal
    for (uint8_t state = Clock::useless;
        state == Clock::useless || state == Clock::dirty;
        state = DCF77_Clock::get_clock_state()) {

        // wait for next sec
        DCF77_Clock::get_current_time(now);

        // render one dot per second while initializing
        static uint8_t count = 0;
        Serial.print('.');
        ++count;
        if (count == 60) {
            count = 0;
            Serial.println();
        }
    }

  updateDisplay(false); // show time on display

  lastmin = now.minute.val; // initialize last update of display

}

void paddedPrint(BCD::bcd_t n) {
    Serial.print(n.digit.hi);
    Serial.print(n.digit.lo);
}

//main loop
//
void loop()
{
  
  DCF77_Clock::get_current_time(now);       

  // update display every minute
  if((now.minute.val > lastmin) || (now.minute.val == 0 && lastmin == 59)) {
    updateDisplay(false);
    lastmin = now.minute.val;
  }

  delay(1000); // without buttons it is okay to update every second

}


uint8_t sample_input_pin() {
    const uint8_t sampled_data =
        #if defined(__AVR__)
        dcf77_inverted_samples ^ (dcf77_analog_samples? (analogRead(dcf77_analog_sample_pin) > 200)
                                                      : digitalRead(dcf77_sample_pin));
        #else
        dcf77_inverted_samples ^ digitalRead(dcf77_sample_pin);
        #endif

    digitalWrite(ledpin(dcf77_monitor_led), sampled_data);
    return sampled_data;
}



// shows time on LED strip
//
void updateDisplay(bool timeset) {

  // number of dots 
  int ndots = (now.minute.val % 5);

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
  switch (now.minute.val / 5) {
                case 0:
                    // glatte Stunde
                    if (effect == 1) {
                      gocrazy();
                      blank();
                      lightup(UHR,effect);
                      setHours(now.hour.val,true,effect);
                    }
                    else {
                      setHours(now.hour.val,true,effect);
                      lightup(UHR,effect);
                    }
                    
                    break;
                case 1:
                    // 5 nach
                    if (effect == 1) {
                      setHours(now.hour.val,false,effect);
                      lightup(NACH,effect);
                      lightup(FUENF_M,effect);
                    }
                    else {
                      lightup(FUENF_M,effect);
                      lightup(NACH,effect);
                      setHours(now.hour.val,false,effect);
                    }
                    break;
                case 2:
                    // 10 nach
                    if (effect == 1) {
                      setHours(now.hour.val,false,effect);
                      lightup(NACH,effect);
                      lightup(ZEHN_M,effect);
                    }
                    else {
                      lightup(ZEHN_M,effect);
                      lightup(NACH,effect);
                      setHours(now.hour.val,false,effect);
                    }
                    break;
                case 3:
                    // viertel nach
                    if (effect == 1) {
                      setHours(now.hour.val,false,effect);
                      lightup(NACH,effect);
                      lightup(VIERTEL,effect);
                    }
                    else {
                      lightup(VIERTEL,effect);
                      lightup(NACH,effect);
                      setHours(now.hour.val,false,effect);
                    }
                    
                    break;
                case 4:
                    // 20 nach
                     if (effect == 1) {
                      setHours(now.hour.val,false,effect);
                      lightup(NACH,effect);
                      lightup(ZWANZIG,effect);
                     }
                     else {
                      lightup(ZWANZIG,effect);
                      lightup(NACH,effect);
                      setHours(now.hour.val,false,effect);
                     }
                    break;
                case 5:
                    // 5 vor halb
                     if (effect == 1) {
                      setHours(now.hour.val + 1,false,effect);
                      lightup(HALB,effect);
                      lightup(VOR,effect);
                      lightup(FUENF_M,effect);
                     }
                     else {
                      lightup(FUENF_M,effect);
                      lightup(VOR,effect);
                      lightup(HALB,effect);
                      setHours(now.hour.val + 1,false,effect);
                     }
                   
                    break;
                case 6:
                    // halb
                    if (effect == 1) {
                      setHours(now.hour.val + 1,false,effect);
                      lightup(HALB,effect);
                    }
                    else {
                      lightup(HALB,effect);
                      setHours(now.hour.val + 1,false,effect);
                    }
                    break;
                case 7:
                    // 5 nach halb
                    if (effect == 1) {
                      setHours(now.hour.val + 1,false,effect);
                      lightup(HALB,effect);
                      lightup(NACH,effect);
                      lightup(FUENF_M,effect);
                    }
                    else {
                      lightup(FUENF_M,effect);
                      lightup(NACH,effect);
                      lightup(HALB,effect);
                      setHours(now.hour.val + 1,false,effect);
                    }
                    
                    break;
                case 8:
                    // 20 vor
                     if (effect == 1) {
                      setHours(now.hour.val + 1,false,effect);
                      lightup(VOR,effect);
                      lightup(ZWANZIG,effect);
                     }
                     else {
                      lightup(ZWANZIG,effect);
                      lightup(VOR,effect);
                      setHours(now.hour.val + 1,false,effect);
                     }
                    break;
                case 9:
                    // viertel vor
                    if (effect == 1) {
                      setHours(now.hour.val + 1,false,effect);
                      lightup(VOR,effect);
                      lightup(VIERTEL,effect);
                    }
                    else {
                      lightup(VIERTEL,effect);
                      lightup(VOR,effect);
                      setHours(now.hour.val + 1,false,effect);
                    }
                    break;
                case 10:
                    // 10 vor
                    if (effect == 1) {
                      setHours(now.hour.val + 1,false,effect);
                      lightup(VOR,effect);
                      lightup(ZEHN_M,effect);
                    }
                    else {
                      lightup(ZEHN_M,effect);
                      lightup(VOR,effect);
                      setHours(now.hour.val + 1,false,effect);
                    }
                    break;
                case 11:
                    // 5 vor
                    if (effect == 1) {
                      setHours(now.hour.val + 1,false,effect);
                      lightup(VOR,effect);
                      lightup(FUENF_M,effect);
                    }
                    else {
                      lightup(FUENF_M,effect);
                      lightup(VOR,effect);
                      setHours(now.hour.val + 1,false,effect);
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
/*
void readButtons() {

  int minbutton = digitalRead(PIN_min);
  int hourbutton = digitalRead(PIN_hour);

  if ((minbutton == LOW) && (hourbutton == LOW)) {
    test();
  }


if (minbutton == LOW) {
  //Serial.println("Minute button pressed");
  if (minute(c_time) == 59) {
    minute(c_time) = 0;
    }
  else {
    minute(c_time)++;
    }
    setTime();
    updateDisplay(true);
    lastmin = minute(c_time);
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
    lastmin = minute(c_time);
    }

delay(200);

}
*/



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




