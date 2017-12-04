/*

  ILV2-7/5 Clock
  Targets a PIC32MX250F128B with ChipKIT DP32 bootloader
  Uses the MAX6920 VFD SIPO driver
  Uses DS3231 RTC
  
  
  Made by Joel Murphy
  Designed to target https://github.com/biomurph/ILV2-7-5_VFD_Display 

  Based In Part On:
  Work with ILV2-5/7 tubes by Callum Nunez-Vaz https://callumnunesvaz.wordpress.com/portfolio/dogbonevfd/
                                               http://www.johngineer.com/blog/?p=1595
  Work with Timer 3 control by Jay Weeks https://www.instructables.com/id/Timer-Interrupts-on-the-DP32/

*/


#include <ILV2_Defines.h>
#include <Wire.h>
#include "RTClib.h"

RTC_DS3231 rtc;

byte NUM[10] = {ZERO,ONE,TWO,THREE,FOUR,FIVE,SIX,SEVEN,EIGHT,NINE};
byte GRID[4] = {GRID_1,GRID_2,GRID_4,GRID_5};
short numToDisplay[4];
byte digit = EIGHT;
int SWITCH[2] = {SW_1,SW_2};
boolean lastSwitchState[2] = {false,false};
unsigned long lastBlink;  
int blinkTimer = 300; // LED blink rate
int testTimer = 400;  // test timing
unsigned int lastTime = 0; // test timing
int testCounter = 0;
byte nibbleCounter = 0;
int testToRun = 0;

boolean VFD_ON = false;
int testGrid = 0;

boolean LEDstate = false;

float battVolts = 0.0;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};


// Timer 3 Variables
volatile uint32_t grid = 0;
volatile boolean muxTime = false;

// Timer 3 ISR
void __attribute__((interrupt)) MUX_TIMER()
{
//  count++;
  muxTime = true;
  grid++;
  if(grid > 3) {grid = 0;}
  clearIntFlag(_TIMER_3_IRQ); // do not not do this
}

void setup(){
  Serial.begin(115200);
  Wire.begin();
  setUpPins(); 
//  FIRE UP TIMER 3
  start_timer_3(400);// set desired Hz
  setIntVector(_TIMER_3_VECTOR, MUX_TIMER);// point to ISR
  setIntPriority(_TIMER_3_VECTOR, 4, 0);// set priority
  clearIntFlag(_TIMER_3_IRQ);// Clear hardware flag
  setIntEnable(_TIMER_3_IRQ);// Enable 

  lastBlink = millis();
  lastTime = millis();
  digit = 0;
  digitalWrite(LED,HIGH);
}



void loop(){
  blink_LED();
  feelSwitches();
  if(muxTime) {mux();}

  if(millis() - lastTime > testTimer){
    lastTime = millis();
    runTest(testToRun);
  }
}



void serialEvent(){
  
  while(Serial.available()){
    char inChar = Serial.read();
    Serial.print("in serialEvent "); Serial.println(inChar);
    
    switch (inChar){
      case 'e': enableVFD(); break;
      case 'd': disableVFD(); break;
      case '?': printVersion(); break;
      case 'b': printBatteryLevel(); break;
      case '0': testToRun = inChar; break;
      case '1': testToRun = inChar; break;
      case '2': testToRun = inChar; break;
      case '3': testToRun = inChar; break;
      case '4': testToRun = inChar; break;
//      case '2': testToRun = inChar; break;
      default:
//        Serial.print("i got "); Serial.println(inChar);
        break;
    }
  }
}

void mux(){
  
  shiftOUT(numToDisplay[grid]);
    
  muxTime = false;
}

void setUpPins(){
  pinMode(LED,OUTPUT); digitalWrite(LED,LOW);
  pinMode(VFD_EN,OUTPUT); digitalWrite(VFD_EN,LOW);
  pinMode(DIN,OUTPUT); digitalWrite(DIN,LOW);
  pinMode(CLK,OUTPUT); digitalWrite(CLK,LOW);
  pinMode(LOAD,OUTPUT); digitalWrite(LOAD,LOW);
  pinMode(BLANK,OUTPUT); digitalWrite(BLANK,LOW);
  pinMode(SW_1,INPUT);
  pinMode(SW_2,INPUT);
  pinMode(BAT_SENS,INPUT);
  pinMode(INT_SQW,INPUT);
  
}

void feelSwitches(){
  boolean switchState;
  for(int i=0; i<2; i++){
    switchState = digitalRead(SWITCH[i]);
    if(switchState != lastSwitchState[i]){
      lastSwitchState[i] = switchState;
      Serial.print("Switch ");Serial.print(i+1);Serial.print(" = "); Serial.println(switchState);
      if(i == 0 && switchState == 1){
        if(VFD_ON){
          disableVFD();
        }else{
          enableVFD();
        }
      }
      if(i == 1 && switchState == 1){
        printBatteryLevel();
        displayBatteryLevel();
        }
      }
    }
  }


void getBatteryLevel(){
  int counts = analogRead(A1);
  float volts = float(counts)*(3.3/1023.0);
  battVolts = volts*2.0;
//  Serial.print(counts);Serial.print(", ");
//  Serial.print(volts); Serial.print(", ");
//  Serial.print(battVolts); Serial.print(", ");
//  Serial.println();
}


void printBatteryLevel(){
  getBatteryLevel();
  Serial.println(battVolts);
}

void displayBatteryLevel(){
  int num;
  getBatteryLevel();
  numToDisplay[0] = BATTERY; 
  num = int(battVolts*100.0);
  for(int i=3; i>0; i--){
      numToDisplay[i] = NUM[num%10];
      num /= 10;
  }
}

void blink_LED(){
  if(millis() - lastBlink > blinkTimer) {
    lastBlink = millis();
    LEDstate = !LEDstate;
    digitalWrite(LED,LEDstate);
  }
}

void printVersion(){
  Serial.println("ILV2-7/5 Clock Test Code");
  Serial.println("Press 'e' to enable VFD");
  Serial.println("Press 'd' to disable VFD");
  Serial.println("Press 'b' to print battery level");
  Serial.println("Press '?' to print this list");
  Serial.println("Press '0' to count up all segments");
  Serial.println("Press '1' to count binary nibble");
  Serial.println("Press '2' to display battery level");
  Serial.println("Press '3' to run test 3");
  Serial.println("Press '4' to run RTC Test");
//  Serial.println("Press '1' to run test 1");
}



void runTest(char test){
  switch (test){
    case '0': runTest_0(); break;
    case '1': runTest_1(); break;
    case '2': runTest_2(); break;
    case '3': runTest_3(); break;
    case '4': 
      if (! rtc.begin()) {
        Serial.println("Couldn't find RTC");
      } else {
        runTest_4(); 
      }
      break;
//    case '2': runTest_2(); break;
    default: break;
  }
}

void runTest_0(){
  testCounter++; 
  if(testCounter > 9) {testCounter = 0;}
  for(int i=0; i<4; i++){
    numToDisplay[i] = NUM[testCounter];
  }
}

void runTest_1(){
  byte nibbleBit = 3;
  nibbleCounter++;
  for(int i=0; i<4; i++){
    numToDisplay[i] = NUM[bitRead(nibbleCounter,nibbleBit)];
    nibbleBit--;
  }
}

void runTest_2(){
  displayBatteryLevel();
}

void runTest_3(){
  for(int i=0; i<4; i++){
    numToDisplay[i] = NUM[i]; 
  }
}

void runTest_4(){
  Serial.println("in test 4");
  readRTCregisters();
  Serial.println("----------");
}

