/***************************************************************************************************************

 This sketch uses the following Libs
 
 * FastLED-3.0.3
 * ds3231-master
 
 
 (c) by Sven Scheil, 2015

*****************************************************************************************************************/

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <config.h>
#include <ds3231.h>
#include <Wire.h>
#include <FastLED.h>

// Params for width and height
const uint8_t kMatrixWidth = 8;
const uint8_t kMatrixHeight = 8;
// Param for different pixel layouts
const bool kMatrixSerpentineLayout = true;
// Number of RGB LEDs in the strand
#define NUM_LEDS 68

// DC3231
#define BUFF_MAX 128
uint8_t time[8];
char recv[BUFF_MAX];
unsigned int recv_size = 0;

// Define the array of leds
CRGB leds[NUM_LEDS];
// Arduino pin used for Data
#define DATA_PIN 4

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

// Colors for display in different set modes and normal time mode
#define COLOR_NORMAL_DISPLAY CRGB( 255, 255, 255)
#define COLOR_SET_DISPLAY CRGB( 255, 136, 0)
#define COLOR_SET_DAY CRGB( 255, 0, 0); 
#define COLOR_SET_MONTH CRGB( 0, 255, 0); 
#define COLOR_SET_YEAR CRGB( 0, 0, 255); 

#define BTN_MIN_PRESSTIME 95    //doftware debouncing: ms button to be pressed before action
#define TIMEOUT_SET_MODE 30000  //ms no button pressed

#define SET_MODE_DUMMY -1
#define SET_MODE_OFF 0
#define SET_MODE_LEDTEST 1
#define SET_MODE_YEAR 2
#define SET_MODE_MONTH 3
#define SET_MODE_DAY 4
#define SET_MODE_HOURS 5
#define SET_MODE_5MINUTES 6
#define SET_MODE_1MINUTES 7
#define SET_MODE_MAX 7

volatile int setModeState = SET_MODE_OFF;

#define START_WITH_YEAR 2015  // start year setting with

#define MIN_BRIGHTNESS 4 
#define MAX_BRIGHTNESS 65

#define SET_BTN1_PIN 2      // set mode button; Interrupt 0 is on DIGITAL PIN 2!
#define SET_BTN2_PIN 3      // set value button; Interrupt 1 is on DIGITAL PIN 3!
#define SET_BTN1_IRQ 0      // set mode button; Interrupt 0 is on DIGITAL PIN 2!
#define SET_BTN2_IRQ 1      // set value button; Interrupt 1 is on DIGITAL PIN 3!

#define INTERNAL_LED_PIN 13

#define BRIGHNTNESS_SENSOR_PIN 2

#define TERM 255

const byte whone[] PROGMEM = {28,29,30,31,TERM};
const byte whtwo[] PROGMEM = {35,36,41,42,TERM};
const byte whthree[] PROGMEM = {41,42,43,44,TERM};
const byte whfour[] PROGMEM = {20,21,22,23,TERM};
const byte whfive[] PROGMEM = {39,40,55,56,TERM};
const byte whsix[] PROGMEM = {24,25,26,27,28,TERM};
const byte whseven[] PROGMEM = {32,33,34,47,46,45,TERM};
const byte wheight[] PROGMEM = {63,62,61,60,TERM};
const byte whnine[] PROGMEM = {52,53,54,55,TERM};
const byte whten[] PROGMEM = {49,50,51,52,TERM};
const byte wheleven[] PROGMEM = {56,57,58,TERM};
const byte whtwelve[] PROGMEM = {35,36,37,38,39,TERM};

const byte* const whours[] PROGMEM = {whone, whtwo, whthree, whfour, whfive, whsix, whseven, wheight, whnine, whten, wheleven, whtwelve};

const byte wmone[] PROGMEM = {66,TERM};
const byte wmtwo[] PROGMEM = {66,65,TERM};
const byte wmthree[] PROGMEM = {66,65,64,TERM};
const byte wmfour[] PROGMEM = {66,65,64,67,TERM};
const byte wmfive[] PROGMEM = {0,1,2,3,TERM};
const byte wmten[] PROGMEM = {4,5,6,7,TERM};
const byte wmfiveten[] PROGMEM = {0,1,2,3,4,5,6,7,TERM};

#define mfive_idx 4
#define mten_idx 5
#define mfiveten_idx 6

const byte* const wminutes[] PROGMEM = {wmone, wmtwo, wmthree, wmfour, wmfive, wmten, wmfiveten};

const byte wto[] PROGMEM = {13,14,15,TERM};
const byte wpast[] PROGMEM = {9,10,11,12,TERM};
const byte whalf[] PROGMEM = {16,17,18,19,TERM};

#define mto_idx 0
#define mpast_idx 1
#define mhalf_idx 2

const byte* const wtime[] PROGMEM = {wto, wpast, whalf};

uint8_t minLastDisplayed = 0;

long lastPressedTime = 0;
struct ts t;

#define UPDATES_PER_SECOND 8

byte startpoint[] = {0,2,3,0,1,0,2};

void setup() {
  delay(3000);

  randomSeed(analogRead(0));

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  
  pinMode(INTERNAL_LED_PIN, OUTPUT);
  digitalWrite(INTERNAL_LED_PIN, LOW);  // turn LED OFF
  
  attachInterrupt(SET_BTN1_IRQ, initSetMode, RISING);
//  attachInterrupt(SET_BTN2_IRQ, setValue, RISING);
 
  pinMode(SET_BTN1_PIN, INPUT);     
  pinMode(SET_BTN2_PIN, INPUT);

  Serial.begin(9600);
  
  Wire.begin(); // init Wire Library
  DS3231_init(DS3231_INTCN);
  memset(recv, 0, BUFF_MAX);
  
  //Serial.println("Setting time");
  //           TssmmhhWDDMMYYYY aka set time
  //parse_cmd("T001919631012015",16);
}

void loop() {
  FastLED.show();                        // see https://github.com/FastLED/FastLED/wiki/FastLED-Temporal-Dithering

  if (setModeState == SET_MODE_OFF) {
    readBrightnessSensor();
    getRTCData(&t);
        
    if (t.min != minLastDisplayed) {
      showAnimation();
      showTime(t.hour, t.min);
      minLastDisplayed = t.min;
    }
    delay(2000);
  } else {
    queryButtonLoop();
  }
  
  Serial.print("setModeState in main loop: ");
  Serial.println(setModeState);
}

void showAnimation() { 
  for (byte loopCount = 0; loopCount < kMatrixHeight+3; loopCount++) {
    matrixRain();  
    FastLED.show();
    FastLED.delay(1000 / UPDATES_PER_SECOND);
  }
  setStartpoints();
}

void checkButton(byte mask, byte *pressed, byte btnPin, void (*action)()) {
      if (digitalRead(btnPin) == LOW) {
      *pressed |= mask;
    } else if ((*pressed & mask) == mask) {
      lastPressedTime = millis();
      action();
      *pressed &= ~mask;
    }
}

void queryButtonLoop() {
  byte pressed = 0b00000000;
  lastPressedTime = millis();
  
  while ((millis() - lastPressedTime < TIMEOUT_SET_MODE) && (setModeState > SET_MODE_OFF)){
    checkButton(0b000001, &pressed, SET_BTN1_PIN, &nextSetMode);
    checkButton(0b000010, &pressed, SET_BTN2_PIN, &nextStep);
  }
  
  if (setModeState > SET_MODE_OFF) {
    Serial.println("timeout");
    resetModeState();
  }
}

void initSetMode() {
  detachInterrupt(SET_BTN1_IRQ);
  Serial.println("initSetMode ");

  nextSetMode();
}

void nextStep() {
  if (setModeState == SET_MODE_DAY) {
    Serial.println("Set next day");
    t.mday+=1;
    if (t.mday > 31) t.mday = 1;
    showDay(t.mday);
  } else   if (setModeState == SET_MODE_MONTH) {
    Serial.println("Set next month");
    t.mon+=1;
    if (t.mon > 12) t.mon = 1;
    showMonth(t.mon);
  } else   if (setModeState == SET_MODE_YEAR) {
    Serial.println("Set next year");
    t.year+=1;
    if (t.year > 2020) t.year = START_WITH_YEAR;
    showYear(t.year);
  } else   if (setModeState == SET_MODE_HOURS) {
    Serial.println("Set next hour");
    t.hour++;
    if (t.hour > 23) t.hour = 0;
    showTime(t.hour, t.min);
  } else   if (setModeState == SET_MODE_5MINUTES) {
    Serial.println("Set next 5 minute word");
    t.min+=5;
    if (t.min > 55) t.min = 0;
    showTime(t.hour, t.min);
  } else   if (setModeState == SET_MODE_1MINUTES) {
    Serial.println("Set next minute dot");
    t.min+=1;
    if (t.min %5 == 0) t.min -= 5;
    showTime(t.hour, t.min);
  } else if (setModeState == SET_MODE_LEDTEST) {
    if ((t.min%5) == 0) {
      t.min+=2;
    } else {
      t.min+=3;
    }
    if (t.min > 60) { 
      t.min = 0;
    }
    showTime(t.hour, t.min);
  } else {
    Serial.println("Unknown setModeState");
  }
}

void nextSetMode() {
  setModeState++;

  Serial.print("Switching to setModeState: ");
  Serial.println(setModeState);
  
  if (setModeState == SET_MODE_HOURS) {
    t.hour=12;
    t.min=0;
    t.sec=0;
    showTime(t.hour, t.min);
  } else if (setModeState == SET_MODE_5MINUTES) {
    t.min=5;
    showTime(t.hour, t.min);
  } else if (setModeState == SET_MODE_1MINUTES) {
    t.min++;
    showTime(t.hour, t.min);    
  } else if (setModeState == SET_MODE_LEDTEST) {
    testShowAllWordsSeq();
  } else if (setModeState == SET_MODE_DAY) {
    showDay(t.mday);
  } else if (setModeState == SET_MODE_MONTH) {
    showMonth(t.mon);
  } else if (setModeState == SET_MODE_YEAR) {
    t.mday;
    t.mon;
    t.year=START_WITH_YEAR;
    showYear(t.year);
  }

  if (setModeState > SET_MODE_MAX) {
    Serial.println("===> Setting new Date & Time to: ");
    printRTCDataStruct(&t);
    DS3231_set(t);
    setModeState = SET_MODE_DUMMY;
    attachInterrupt(SET_BTN1_IRQ, initSetMode, RISING);
  }
}

void resetModeState() {
  if (setModeState != SET_MODE_OFF) {
    setModeState = SET_MODE_OFF;
    Serial.println("Resetting setModeState after timeout pressing no button. Fallback to old Time & Date.");
    attachInterrupt(SET_BTN1_IRQ, initSetMode, RISING);
  }
}

void getRTCData(struct ts *t) {
    float temperature;
    char buff[BUFF_MAX];
   
    DS3231_get(t); //Get time

    printRTCDataStruct(t);
}

void showTime(int hours, int minutes) {
  Serial.print("Show time: ");
  printTime(hours, minutes);
  FastLED.clear();
  int hcorrection = showMinutes(minutes);
  showHours(hours + hcorrection);
  FastLED.show();
}

// If there are <= 20 minutes in a hour a correction value of 1 is returned
// to be added to the hour
byte showMinutes(int minutes) {
  
  int tidx = -1; // 5, 10, 15
  int ridx = -1; // 1 past , 2 to 
  int qidx = -1; // 1 half
  int showMinutes = minutes;
  
  if (minutes == 0) {
  } else if (minutes < 5) {
    ridx = mpast_idx;
  } else if (minutes < 10) {
    tidx = mfive_idx;
    ridx = mpast_idx;
  } else if (minutes < 15) {
    tidx = mten_idx;
    ridx = mpast_idx;
  } else if (minutes < 20) {
    tidx = mfiveten_idx;
    ridx = mpast_idx;
  } else if (minutes == 20) {
    tidx = mten_idx;
    ridx = mto_idx;
    qidx = mhalf_idx;
  } else if (minutes < 25) {
    tidx = mfive_idx;
    ridx = mto_idx;
    qidx = mhalf_idx;
    showMinutes = 25 - minutes;
  } else if (minutes == 25) {
    tidx = mfive_idx;
    ridx = mto_idx;
    qidx = mhalf_idx;
  } else if (minutes < 30) {
    ridx = mto_idx;
    qidx = mhalf_idx;
    showMinutes = 30 - minutes;
  } else if (minutes == 30) {
    qidx = mhalf_idx;  
  } else if (minutes < 35) {
    qidx = mhalf_idx;  
    ridx = mpast_idx;
  } else if (minutes < 40) {
    tidx = mfive_idx;
    ridx = mpast_idx;
    qidx = mhalf_idx;  
  } else if (minutes == 40) {
    tidx = mten_idx;
    ridx = mpast_idx;
    qidx = mhalf_idx;  
  } else if (minutes < 45) {
    tidx = mfiveten_idx;
    ridx = mto_idx;
    showMinutes = 45 - minutes;
  } else if (minutes == 45) {
    tidx = mfiveten_idx;
    ridx = mto_idx;
  } else if (minutes < 50) {
    tidx = mten_idx;
    ridx = mto_idx;
    showMinutes = 50 - minutes;
  } else if (minutes == 50) {
    tidx = mten_idx;
    ridx = mto_idx;
  } else if (minutes < 55) {
    tidx = mfive_idx;
    ridx = mto_idx;
    showMinutes = 55 - minutes;
  } else if (minutes == 55) {
    tidx = mfive_idx;
    ridx = mto_idx;
  } else if (minutes < 60) {
    showMinutes = 60 - minutes;
    ridx = mto_idx;
  }
  
  if (tidx>=0) {
    showWord((byte*) pgm_read_word (&wminutes[tidx]));
  }
  
  if (ridx>=0) {
    showWord((byte*) pgm_read_word (&wtime[ridx]));
  }

  if (qidx>=0) {
    showWord((byte*) pgm_read_word (&wtime[qidx]));
  }

  if ((showMinutes % 5) > 0) {
    showWord((byte*) pgm_read_word (&wminutes[(showMinutes % 5)-1]));
  }
  
  if (minutes < 20) {
    return 0;
  } else {
    return 1;
  }
}

void showHours(int hours) {
  if (hours > 12) {
    hours -= 12;
  }
  
  showWord((byte*) pgm_read_word (&whours[hours-1]));
}
  
void testShowAllWordsSeq() {
  FastLED.clear();  
  showWord((byte*) pgm_read_word (&wminutes[0]));
  showWord((byte*) pgm_read_word (&wminutes[1]));
  showWord((byte*) pgm_read_word (&wminutes[2]));
  showWord((byte*) pgm_read_word (&wminutes[3]));
  showWord((byte*) pgm_read_word (&wminutes[mfive_idx]));
  showWord((byte*) pgm_read_word (&wminutes[mten_idx]));
  showWord((byte*) pgm_read_word (&wminutes[mfiveten_idx]));
  showWord((byte*) pgm_read_word (&wtime[mto_idx]));
  showWord((byte*) pgm_read_word (&wtime[mpast_idx])); 
  showWord((byte*) pgm_read_word (&wtime[mhalf_idx]));
  showWord((byte*) pgm_read_word (&whours[0]));
  showWord((byte*) pgm_read_word (&whours[1])); 
  showWord((byte*) pgm_read_word (&whours[2]));
  showWord((byte*) pgm_read_word (&whours[3]));
  showWord((byte*) pgm_read_word (&whours[4]));
  showWord((byte*) pgm_read_word (&whours[5])); 
  showWord((byte*) pgm_read_word (&whours[6]));
  showWord((byte*) pgm_read_word (&whours[7])); 
  showWord((byte*) pgm_read_word (&whours[8]));
  showWord((byte*) pgm_read_word (&whours[9])); 
  showWord((byte*) pgm_read_word (&whours[10]));
  showWord((byte*) pgm_read_word (&whours[11]));
  FastLED.show();  
}

void showWord(const byte* wordLeds) {

  byte idx = pgm_read_byte(wordLeds);
  
  while (idx < TERM) {
    if (setModeState <= SET_MODE_OFF) {
      leds[idx] = COLOR_NORMAL_DISPLAY; 
    } else {
      leds[idx] = COLOR_SET_DISPLAY; 
    }
    wordLeds++;
    idx = pgm_read_byte( wordLeds);
  }
}

void showDay(int day) {
  FastLED.clear();
  for(byte i = 0; i < day; i++) {
    leds[i] = COLOR_SET_DAY; 
  }
  FastLED.show();
}
  
void showMonth(int month) {
  FastLED.clear();
  for(byte i = 0; i < month; i++) {
    leds[i] = COLOR_SET_MONTH; 
  }
  FastLED.show();
}
  
void showYear(int year) {
  FastLED.clear();
  for(byte i = 0; i < year-2000; i++) {
    leds[i] = COLOR_SET_YEAR; 
  }
  FastLED.show();
}

void readBrightnessSensor() {
  char buffer [28];
  int brightnessVal = map(analogRead(BRIGHNTNESS_SENSOR_PIN), 0, 1023, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
  FastLED.setBrightness( brightnessVal );

  sprintf(buffer, "Brightness [%d,%d] = %d", MIN_BRIGHTNESS, MAX_BRIGHTNESS, brightnessVal);
  Serial.println(buffer);
}

void printRTCDataStruct(struct ts *t) {
    printDate(t);
    printTime(t);
}

void printTime(int hours, int minutes) {
    char buffer [5];
    
    sprintf(buffer, "%02d:%02d", hours, minutes);
    Serial.println(buffer);
}

void printTime(struct ts *t) {
  uint8_t m = t->min;
  Serial.print(t->hour, DEC);
  Serial.print(":");
  Serial.print(m);
  Serial.print(":");
  Serial.println(t->sec);
  
  char buffer [12];
    
    sprintf(buffer, "%02d:%02d:%02d", (uint8_t) t->hour, t->min, t->sec);
    Serial.println(buffer);
}

void printDate(struct ts *t)
{
   char buffer [12];
    
   sprintf(buffer, "%02d.%02d.%04d, ", t->mday, t->mon, t->year);
   Serial.print(buffer);
}

void matrixRain() {
  for (byte x = 0; x < kMatrixWidth; x++) {
    matrixRainCol(startpoint[x]++, x);
  }
}

void matrixRainCol(int currentLine, byte x) {
  byte startline = MAX(currentLine - 2, 0);
  byte endline = MIN(kMatrixHeight, currentLine + 1);
  
  for (byte y = 0; y < kMatrixHeight; y++) {
  
    if ((y >= startline) && (y < endline) ) {
      if ((y+1 == endline) && (endline < kMatrixHeight))
        leds[xy2LedIndex(x, y)] = CRGB(255,255,255);
      else 
        leds[xy2LedIndex(x, y)] = CRGB(45, 45, 45);
    } else {
      if (y < endline)
        leds[xy2LedIndex(x, y)] = CRGB::Black;
    }
  }
}

void setStartpoints() {
  startpoint[0] = random(3);
  startpoint[1] = random(3);
  startpoint[2] = random(3);
  startpoint[3] = random(3);
  startpoint[4] = random(3);
  startpoint[5] = random(3);
  startpoint[6] = random(3);
  startpoint[7] = random(3);
  startpoint[8] = random(3);
}

uint16_t xy2LedIndex( uint8_t x, uint8_t y)
{
  uint16_t i;
  
  if( kMatrixSerpentineLayout == false) {
    i = (y * kMatrixWidth) + x;
  }

  if( kMatrixSerpentineLayout == true) {
    if( y & 0x01) {
      // Odd rows run backwards
      uint8_t reverseX = (kMatrixWidth - 1) - x;
      i = (y * kMatrixWidth) + reverseX;
    } else {
      // Even rows run forwards
      i = (y * kMatrixWidth) + x;
    }
  }
  
  return i;
}


