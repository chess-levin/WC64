#include <config.h>
#include <ds3231.h>
#include <Wire.h>
#include <FastLED.h>

// Number of RGB LEDs in the strand
#define NUM_LEDS 68
const uint8_t NUM_ROWS  = 8;

// DC3231
#define BUFF_MAX 128
uint8_t time[8];
char recv[BUFF_MAX];
unsigned int recv_size = 0;

// Define the array of leds
CRGB leds[NUM_LEDS];
// Arduino pin used for Data
#define DATA_PIN 2

#define BTN_MIN_PRESSTIME 150   //ms button to be pressed before action
#define TIMEOUT_SET_MODE 60000  //ms no button pressed

#define SET_MODE_OFF 1
#define SET_MODE_LEDTEST 2
#define SET_MODE_DAY 4
#define SET_MODE_MONTH 8
#define SET_MODE_YEAR 16
#define SET_MODE_HOURS 32
#define SET_MODE_5MINUTES 256
#define SET_MODE_1MINUTES 512
#define SET_MODE_MAX 512
#define START_WITH_YEAR 2015

#define MIN_BRIGHTNESS 25        // 16 is minimum
#define MAX_BRIGHTNESS 122

#define SET_BTN1_PIN 7
#define SET_BTN2_PIN 6
#define INTERNAL_LED_PIN 13

#define BRIGHNTNESS_SENSOR_PIN 2

uint8_t setMode = SET_MODE_OFF;
int brightnessVal = 37;  // Default hue

int wmone[] = {66};
int wmtwo[] = {66,65};
int wmthree[] = {66,65,64};
int wmfour[] = {66,65,64,67};
int wmfive[] = {0,1,2,3};
int wmten[] = {4,5,6,7};
int wmfiveten[] = {0,1,2,3,4,5,6,7};
int wto[] = {13,14,15};
int wpast[] = {9,10,11,12};
int whalf[] = {16,17,18,19};
int whone[] = {28,29,30,31};
int whtwo[] = {35,36,41,42};
int whthree[] = {41,42,43,44};
int whfour[] = {20,21,22,23};
int whfive[] = {39,40,55,56};
int whsix[] = {24,25,26,27,28};
int whseven[] = {32,33,34,47,46,45};
int wheight[] = {63,62,61,60};
int whnine[] = {52,53,54,55};
int whten[] = {49,50,51,52};
int wheleven[] = {56,57,58};
int whtwelve[] = {35,36,37,38,39};

void setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  
  pinMode(INTERNAL_LED_PIN, OUTPUT);
  pinMode(SET_BTN2_PIN, INPUT);
  digitalWrite(SET_BTN2_PIN, HIGH); // connect internal pull-up
  pinMode(SET_BTN1_PIN, INPUT);     // uses external pull-up
  digitalWrite(INTERNAL_LED_PIN, LOW);  // turn LED OFF

  Serial.begin(9600);  
  Wire.begin(); // init Wire Library
  DS3231_init(DS3231_INTCN);
  memset(recv, 0, BUFF_MAX);
  
  //Serial.println("Setting time");
  //           TssmmhhWDDMMYYYY aka set time
  //parse_cmd("T001919631012015",16);
}

void loop() {
    struct ts t;
    
    if (setMode == SET_MODE_OFF) {
      hueSensor();
      getRTCData(&t);
      showTime(t.hour, t.min);
    }

    idleLoop(&t);
    
    //showTestTimes();
}

void idleLoop(struct ts *t) {
    long startTime = millis();
    long pressedTime1 = 0;
    long pressedTime2 = 0;
    long lastPressedTime = 0;
    
    while (millis() - startTime < 10000) {
      
      int val1 = digitalRead(SET_BTN1_PIN);
      if (val1 == LOW) {      // Button pressed
       if (pressedTime1 == 0) {
          pressedTime1 = millis();
          lastPressedTime = pressedTime1;
       }
      } else {               // Button released
        if (pressedTime1 > 0) {
          long duration = millis() - pressedTime1;
          if (duration > BTN_MIN_PRESSTIME) {
            nextSetMode(t);
          }
        }
        pressedTime1 = 0;
      }

      int val2 = digitalRead(SET_BTN2_PIN);
      if (val2 == LOW) {      // Button pressed
       if (pressedTime2 == 0) {
          pressedTime2 = millis();
          lastPressedTime = pressedTime2;
       }
      } else {               // Button released
        if (pressedTime2 > 0) {
          long duration = millis() - pressedTime2;
          if (duration > BTN_MIN_PRESSTIME) {
            nextStep(t);
          }
        }
        pressedTime2 = 0;
      }
    }
    if (millis() - lastPressedTime > TIMEOUT_SET_MODE) {
      resetSetMode();
    }
}

void nextStep(struct ts *t) {
  confirmationLEDFlash();
  if (setMode == SET_MODE_DAY) {
    Serial.println("Set next day");
    showDay(t->mday++);
  } else   if (setMode == SET_MODE_MONTH) {
    Serial.println("Set next month");
    showMonth(t->mon++);
  } else   if (setMode == SET_MODE_YEAR) {
    Serial.println("Set next year");
    showYear(t->year++);
  } else   if (setMode == SET_MODE_HOURS) {
    Serial.println("Set next hour");
    t->hour++;
    if (t->hour > 23) t->hour = 0;
    showTime(t->hour, t->min);
  } else   if (setMode == SET_MODE_5MINUTES) {
    Serial.println("Set next 5 minute word");
    t->min+=5;
    if (t->min > 55) t->min = 0;
    showTime(t->hour, t->min);
  } else   if (setMode == SET_MODE_1MINUTES) {
    Serial.println("Set next minute dot");
    t->min+=1;
    if (t->min > 60) t->min = 0;
    showTime(t->hour, t->min);
  } else   if (setMode == SET_MODE_LEDTEST) {
    if ((t->min%5) == 0) {
      t->min+=2;
    } else {
      t->min+=3;
    }
    if (t->min > 60) { 
      t->min = 0;
    }
    showTime(t->hour, t->min);
  } else {
    Serial.println("Unknown setMode");
  }
}

void nextSetMode(struct ts *t) {
  confirmationLEDFlash();
  setMode = setMode << 1;
  
  if (setMode == SET_MODE_HOURS) {
    t->hour=12;
    t->min=0;
    t->sec=0;
    showTime(t->hour, t->min);
  } else if (setMode == SET_MODE_LEDTEST) {
    t->hour=12;
    t->min=0;
    t->sec=0;
    showAllWordsSeq();
  } else if (setMode == SET_MODE_DAY) {
    t->mday;
    t->mon=0;
    t->year=START_WITH_YEAR;
    showDay(t->mday);
  } else if (setMode == SET_MODE_MONTH) {
    showMonth(t->mon);
  } else if (setMode == SET_MODE_YEAR) {
    showYear(t->year);
  }

  if (setMode > SET_MODE_MAX) {
    setMode = SET_MODE_OFF;
  }
  
  Serial.print("Switching to setMode: ");
  Serial.println(setMode);
  if (setMode == SET_MODE_OFF) {
    DS3231_set(*t);
    digitalWrite(INTERNAL_LED_PIN, LOW);
  } else {
    digitalWrite(INTERNAL_LED_PIN, HIGH);
  }

}

void confirmationLEDFlash() {
  digitalWrite(INTERNAL_LED_PIN, LOW);
  delay (200);
  digitalWrite(INTERNAL_LED_PIN, HIGH);
}

void resetSetMode() {
  if (setMode != SET_MODE_OFF) {
    setMode = SET_MODE_OFF;
    Serial.println("Resetting setMode after timeout pressing no button. Fallback to old Time & Date.");
    digitalWrite(INTERNAL_LED_PIN, LOW);
  }
}

void getRTCData(struct ts *t) {
    float temperature;
    char tempF[6];
    char buff[BUFF_MAX];
   
    DS3231_get(t); //Get time
    parse_cmd("C",1);
    temperature = DS3231_get_treg(); //Get temperature
    dtostrf(temperature, 5, 1, tempF);

    Serial.print(t->mday);
    
    printMonth(t->mon);
    
    Serial.print(t->year);
    Serial.print(", ");   
    Serial.print(t->hour);
    Serial.print(":");
    if(t->min<10)
    {
      Serial.print("0");
    }
    Serial.print(t->min);
    Serial.print(":");
    if(t->sec<10)
    {
      Serial.print("0");
    }
    Serial.print(t->sec);
    
    Serial.print(' ');
    Serial.print(tempF);
    Serial.print((char)223);
    Serial.print("C ");
}

void pushBtnState() {
  int val = digitalRead(SET_BTN1_PIN);  // read input value
  if (val == HIGH) {         // check if the input is HIGH (button released)
    digitalWrite(INTERNAL_LED_PIN, LOW);  // turn LED OFF
  } else {
    while(digitalRead(SET_BTN1_PIN) == LOW) {
      digitalWrite(INTERNAL_LED_PIN, HIGH);  // turn LED ON
    }
  }
  digitalWrite(INTERNAL_LED_PIN, LOW);  // turn LED OFF
}

void wait4PushBtnState() {
  int val = digitalRead(SET_BTN1_PIN);  // read input value
  while (val == HIGH) {         // check if the input is HIGH (button released)
    digitalWrite(INTERNAL_LED_PIN, LOW);  // turn LED OFF
    val = digitalRead(SET_BTN1_PIN);  // read input value
  }
  digitalWrite(INTERNAL_LED_PIN, HIGH);  // turn LED ON
  while(val == LOW) {
    val = digitalRead(SET_BTN1_PIN);
   }
  delay(250);
  digitalWrite(INTERNAL_LED_PIN, LOW);  // turn LED OFF
}

void showTestTimes() {
  int h = 12;
  showTestTime(h, 0);
  showTestTime(h, 1);
  showTestTime(h, 2);
  showTestTime(h, 3);
  showTestTime(h, 4);
  showTestTime(h, 5);
  showTestTime(h, 7);
  showTestTime(h, 10);
  showTestTime(h, 13);
  showTestTime(h, 15);
  showTestTime(h, 17);
  showTestTime(h, 20);
  showTestTime(h, 21);
  showTestTime(h, 23);
  showTestTime(h, 25);
  showTestTime(h, 29);
  showTestTime(h, 30);
  showTestTime(h, 32);
  showTestTime(h, 35);
  showTestTime(h, 37);
  showTestTime(h, 39);
  showTestTime(h, 40);
  showTestTime(h, 41);
  showTestTime(h, 45);
  showTestTime(h, 47);
  showTestTime(h, 50);
  showTestTime(h, 51);
  showTestTime(h, 55);
  showTestTime(h, 58);
}

void showTestTime(int hours, int minutes) {
  showTime(hours, minutes);
  wait4PushBtnState();
}
  
void showTime(int hours, int minutes) {
  Serial.println();
  Serial.print(hours);
  Serial.print(":");
  Serial.println(minutes);

  FastLED.clear();
  
  int hcorrection = showMinutes(minutes);
  showHours(hours + hcorrection);
}

int showMinutes(int minutes) {
  
  int tidx = 0; // 5, 10, 15
  int ridx = 0; // 1 nach , 2 vor 
  int qidx = 0; // 1 halb
  int showMinutes = minutes;
  
  if (minutes == 0) {
  } else if (minutes < 5) {
    tidx = 0;
    ridx = 1;
  } else if (minutes < 10) {
    tidx = 5;
    ridx = 1;
  } else if (minutes < 15) {
    tidx = 10;
    ridx = 1;
  } else if (minutes < 20) {
    tidx = 15;
    ridx = 1;
  } else if (minutes == 20) {
    tidx = 10;
    ridx = 2;
    qidx = 1;
  } else if (minutes < 25) {
    tidx = 5;
    ridx = 2;
    qidx = 1;
    showMinutes = 25 - minutes;
  } else if (minutes == 25) {
    tidx = 5;
    ridx = 2;
    qidx = 1;
  } else if (minutes < 30) {
    ridx = 2;
    qidx = 1;
    showMinutes = 30 - minutes;
  } else if (minutes == 30) {
    qidx = 1;  
  } else if (minutes < 35) {
    qidx = 1;  
    ridx = 1;
  } else if (minutes < 40) {
    tidx = 5;
    ridx = 1;
    qidx = 1;  
  } else if (minutes == 40) {
    tidx = 10;
    ridx = 1;
    qidx = 1;  
  } else if (minutes < 45) {
    tidx = 15;
    ridx = 2;
    showMinutes = 45 - minutes;
  } else if (minutes == 45) {
    tidx = 15;
    ridx = 2;
  } else if (minutes < 50) {
    tidx = 10;
    ridx = 2;
    showMinutes = 50 - minutes;
  } else if (minutes == 50) {
    tidx = 10;
    ridx = 2;
  } else if (minutes < 55) {
    tidx = 5;
    ridx = 2;
    showMinutes = 55 - minutes;
  } else if (minutes == 55) {
    tidx = 5;
    ridx = 2;
  } else if (minutes < 60) {
    showMinutes = 60 - minutes;
    ridx = 2;
  }
   
  switch (tidx) {
    case 5: showWord(wmfive, sizeof(wmfive) / sizeof(int)); break;
    case 10: showWord(wmten, sizeof(wmten) / sizeof(int)); break;
    case 15: showWord(wmfiveten, sizeof(wmfiveten) / sizeof(int)); break;
  }
  
  switch (ridx) {
    case 1: showWord(wpast, sizeof(wpast) / sizeof(int)); break;
    case 2: showWord(wto, sizeof(wto) / sizeof(int)); break;
  }

  switch (qidx) {
    case 1: showWord(whalf, sizeof(whalf) / sizeof(int)); break;
  }
  
  switch (showMinutes % 5) {
    case 1:showWord(wmone, sizeof(wmone) / sizeof(int)); break;
    case 2:showWord(wmtwo, sizeof(wmtwo) / sizeof(int)); break;
    case 3:showWord(wmthree, sizeof(wmthree) / sizeof(int)); break;
    case 4:showWord(wmfour, sizeof(wmfour) / sizeof(int)); break;
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
  switch (hours ) {
    case 0: showWord(whtwelve, sizeof(whtwelve) / sizeof(int)); break;
    case 1: showWord(whone, sizeof(whone) / sizeof(int)); break;
    case 2: showWord(whtwo, sizeof(whtwo) / sizeof(int)); break;
    case 3: showWord(whthree, sizeof(whthree) / sizeof(int)); break;
    case 4: showWord(whfour, sizeof(whfour) / sizeof(int)); break;
    case 5: showWord(whfive, sizeof(whfive) / sizeof(int)); break;
    case 6: showWord(whsix, sizeof(whsix) / sizeof(int)); break;
    case 7: showWord(whseven, sizeof(whseven) / sizeof(int)); break;
    case 8: showWord(wheight, sizeof(wheight) / sizeof(int)); break;
    case 9: showWord(whnine, sizeof(whnine) / sizeof(int)); break;
    case 10: showWord(whten, sizeof(whten) / sizeof(int)); break;
    case 11: showWord(wheleven, sizeof(wheleven) / sizeof(int)); break;
    case 12: showWord(whtwelve, sizeof(whtwelve) / sizeof(int)); break;
  }
}
  
void showAllWordsSeq() {
  showWord(wmfive, sizeof(wmfive) / sizeof(int));
  showWord(wmten, sizeof(wmten) / sizeof(int));
  showWord(wto, sizeof(wto) / sizeof(int));
  showWord(wpast, sizeof(wpast) / sizeof(int)); 
  showWord(whalf, sizeof(whalf) / sizeof(int));
  showWord(whone, sizeof(whone) / sizeof(int));
  showWord(whtwo, sizeof(whtwo) / sizeof(int)); 
  showWord(whthree, sizeof(whthree) / sizeof(int));
  showWord(whfour, sizeof(whfour) / sizeof(int));
  showWord(whfive, sizeof(wmfive) / sizeof(int));
  showWord(whsix, sizeof(whsix) / sizeof(int)); 
  showWord(whseven, sizeof(whseven) / sizeof(int));
  showWord(wheight, sizeof(wheight) / sizeof(int)); 
  showWord(whnine, sizeof(whnine) / sizeof(int));
  showWord(whten, sizeof(whten) / sizeof(int)); 
  showWord(wheleven, sizeof(wheleven) / sizeof(int));
  showWord(whtwelve, sizeof(whtwelve) / sizeof(int));
  showWord(wmfour, sizeof(wmfour) / sizeof(int));
  showWord(wmone, sizeof(wmone) / sizeof(int));
  showWord(wmtwo, sizeof(wmtwo) / sizeof(int));
  showWord(wmthree, sizeof(wmthree) / sizeof(int));
  showWord(wmfour, sizeof(wmfour) / sizeof(int));
}

void showWord(int wordLeds[], int elementCount) {  
    for(int i = 0; i < elementCount; i++) {
      leds[wordLeds[i]] = CHSV( 0, 0, brightnessVal); 
    }
    FastLED.show();
}

void showDay(int day) {
  FastLED.clear();
  for(byte i = 0; i < day; i++) {
    leds[i] = CHSV( 0, 0, brightnessVal); 
  }
  FastLED.show();
}
  
void showMonth(int month) {
  FastLED.clear();
  for(byte i = 0; i < month; i++) {
    leds[i] = CHSV( 0, 0, brightnessVal); 
  }
  FastLED.show();
}
  
void showYear(int year) {
  FastLED.clear();
  for(byte i = 0; i < year-START_WITH_YEAR; i++) {
    leds[i] = CHSV( 0, 0, brightnessVal); 
  }
  FastLED.show();
}

void hueSensor() {
  int val = analogRead(BRIGHNTNESS_SENSOR_PIN);    // read the value from the sensor
  brightnessVal = map(val, 0, 1023, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
  Serial.print("Brightness [");
  Serial.print(MIN_BRIGHTNESS);
  Serial.print(", ");
  Serial.print(MAX_BRIGHTNESS);
  Serial.print("] = ");
  Serial.println(brightnessVal);
}

int ledIndex(int col, int row) {
  if (row  % 2 == 0)
    return row * NUM_ROWS + col;
   else
     return row * NUM_ROWS + NUM_ROWS - 1 - col;
}

void printMonth(int month)
{
  switch(month)
  {
    case 1: Serial.print(" January ");break;
    case 2: Serial.print(" February ");break;
    case 3: Serial.print(" March ");break;
    case 4: Serial.print(" April ");break;
    case 5: Serial.print(" May ");break;
    case 6: Serial.print(" June ");break;
    case 7: Serial.print(" July ");break;
    case 8: Serial.print(" August ");break;
    case 9: Serial.print(" September ");break;
    case 10: Serial.print(" October ");break;
    case 11: Serial.print(" November ");break;
    case 12: Serial.print(" December ");break;
    default: Serial.print(" Error ");break;
  } 
}

void parse_cmd(char *cmd, int cmdsize)
{
    uint8_t i;
    uint8_t reg_val;
    char buff[BUFF_MAX];
    struct ts t;

    //snprintf(buff, BUFF_MAX, "cmd was '%s' %d\n", cmd, cmdsize);
    //Serial.print(buff);

    // TssmmhhWDDMMYYYY aka set time
    if (cmd[0] == 84 && cmdsize == 16) {
        //T355720619112011
        t.sec = inp2toi(cmd, 1);
        t.min = inp2toi(cmd, 3);
        t.hour = inp2toi(cmd, 5);
        t.wday = inp2toi(cmd, 7);
        t.mday = inp2toi(cmd, 8);
        t.mon = inp2toi(cmd, 10);
        t.year = inp2toi(cmd, 12) * 100 + inp2toi(cmd, 14);
        DS3231_set(t);
        Serial.println("OK");
    } else if (cmd[0] == 49 && cmdsize == 1) {  // "1" get alarm 1
        DS3231_get_a1(&buff[0], 59);
        Serial.println(buff);
    } else if (cmd[0] == 50 && cmdsize == 1) {  // "2" get alarm 1
        DS3231_get_a2(&buff[0], 59);
        Serial.println(buff);
    } else if (cmd[0] == 51 && cmdsize == 1) {  // "3" get aging register
        Serial.print("aging reg is ");
        Serial.println(DS3231_get_aging(), DEC);
    } else if (cmd[0] == 65 && cmdsize == 9) {  // "A" set alarm 1
        DS3231_set_creg(DS3231_INTCN | DS3231_A1IE);
        //ASSMMHHDD
        for (i = 0; i < 4; i++) {
            time[i] = (cmd[2 * i + 1] - 48) * 10 + cmd[2 * i + 2] - 48; // ss, mm, hh, dd
        }
        boolean flags[5] = { 0, 0, 0, 0, 0 };
        DS3231_set_a1(time[0], time[1], time[2], time[3], flags);
        DS3231_get_a1(&buff[0], 59);
        Serial.println(buff);
    } else if (cmd[0] == 66 && cmdsize == 7) {  // "B" Set Alarm 2
        DS3231_set_creg(DS3231_INTCN | DS3231_A2IE);
        //BMMHHDD
        for (i = 0; i < 4; i++) {
            time[i] = (cmd[2 * i + 1] - 48) * 10 + cmd[2 * i + 2] - 48; // mm, hh, dd
        }
        boolean flags[5] = { 0, 0, 0, 0 };
        DS3231_set_a2(time[0], time[1], time[2], flags);
        DS3231_get_a2(&buff[0], 59);
        Serial.println(buff);
    } else if (cmd[0] == 67 && cmdsize == 1) {  // "C" - get temperature register
        Serial.print("temperature reg is ");
        Serial.println(DS3231_get_treg(), DEC);
    } else if (cmd[0] == 68 && cmdsize == 1) {  // "D" - reset status register alarm flags
        reg_val = DS3231_get_sreg();
        reg_val &= B11111100;
        DS3231_set_sreg(reg_val);
    } else if (cmd[0] == 70 && cmdsize == 1) {  // "F" - custom fct
        reg_val = DS3231_get_addr(0x5);
        Serial.print("orig ");
        Serial.print(reg_val,DEC);
        Serial.print("month is ");
        Serial.println(bcdtodec(reg_val & 0x1F),DEC);
    } else if (cmd[0] == 71 && cmdsize == 1) {  // "G" - set aging status register
        DS3231_set_aging(0);
    } else if (cmd[0] == 83 && cmdsize == 1) {  // "S" - get status register
        Serial.print("status reg is ");
        Serial.println(DS3231_get_sreg(), DEC);
    } else {
        Serial.print("unknown command prefix ");
        Serial.println(cmd[0]);
        Serial.println(cmd[0], DEC);
    }
}

/*
void poti() {
  int val = analogRead(potPin);    // read the value from the sensor
  Serial.println(val);
  digitalWrite(INTERNAL_LED_PIN, HIGH);  // turn the INTERNAL_LED_PIN on
  delay(val);                  // stop the program for some time
  digitalWrite(INTERNAL_LED_PIN, LOW);   // turn the INTERNAL_LED_PIN off
  delay(val);    
}
 
void potiBar() {
  int val = analogRead(2);
    int numLedsToLight = map(val, 0, 1023, 0, NUM_LEDS);

    // First, clear the existing led values
    FastLED.clear();
    for(int led = 0; led < numLedsToLight; led++) { 
        leds[led] = CRGB::Blue; 
    }
    FastLED.show();  
}

void snake() {
  for(int dot = 0; dot < NUM_LEDS; dot++) { 
      leds[dot] = CRGB::Blue;
      FastLED.show();
      // clear this led for the next time around the loop
      leds[dot] = CRGB::Black;
      delay(30);
   }
}

void initTest() {
  leds[0] = CRGB::Red; 
  FastLED.show(); 
  delay(30);
  
  leds[1] = CRGB::Green; 
  FastLED.show(); 
  delay(30);
  
  leds[2] = CRGB::Blue; 
  FastLED.show(); 
  delay(30);
}
*/
