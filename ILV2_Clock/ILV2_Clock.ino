/*

  ILV2-7/5 Clock
  Targets a PIC32MX250F128B with ChipKIT DP32 bootloader
  Uses the MAX6920 VFD SIPO driver
  Uses DS3231 RTC
  Powered by a 24V 2.5mm AC Adapter. North of 500mA should work


  Made by Joel Murphy, Winter 2017
  Designed to target https://github.com/biomurph/ILV2-7-5_VFD_Display hardware

  Based In Part On:
  Work with ILV2-5/7 tubes by Callum Nunez-Vaz https://callumnunesvaz.wordpress.com/portfolio/dogbonevfd/
                                               http://www.johngineer.com/blog/?p=1595
  Work with Timer 3 control by Jay Weeks https://www.instructables.com/id/Timer-Interrupts-on-the-DP32/

  Adafruit's RTClib with modifications to access Alarm2 for interrupts
  USE BIOMURPH's fork of Adafruit's RTClib https://github.com/biomurph/RTClib

  Updated by Joel Murphy, Spring 2019 to target new hardware version
  
*/


#include <ILV2_Defines.h>
#include <Wire.h>
#include "RTClib.h"

RTC_DS3231 rtc;

byte NUM[10] = {ZERO,ONE,TWO,THREE,FOUR,FIVE,SIX,SEVEN,EIGHT,NINE};
byte GRID[4] = {GRID_1,GRID_2,GRID_4,GRID_5};
short numToDisplay[4];

int SWITCH[2] = {SW_1,SW_2};
boolean lastSwitchState[2] = {false,false};
boolean debounce[2] = {false,false};
unsigned int debounceTime;

unsigned long lastBlink;
unsigned int lastTime = 0; // test timing
int testCounter = 0;
byte nibbleCounter = 0;
int progToRun = 0;
int timeToSet = 0;
int hours;
int mins;
int displayMode = 24;
boolean blankDot = false;
boolean blankState = false;
unsigned int lastBlankTime = 0;
int digit = 0;
int testGrid = 0;

boolean VFD_ON = false;
// int testGrid = 0;

boolean LEDstate = false;

float battVolts = 0.0;

// Timer 3 Variables
volatile uint32_t grid = 0;
volatile boolean muxTime = false;

// Timer 3 ISR
void __attribute__((interrupt)) MUX_TIMER()
{
  muxTime = true;
  grid++;
  if(grid > 3) {grid = 0;}
  clearIntFlag(_TIMER_3_IRQ); // do not not do this
}

void setup(){
  Serial.begin(115200);
  Wire.begin();
  setUpStuph();             // pin directions an variables
  fireUpTimer3(400);        // set up the timer to multiplex the GRIDS at (Hz)
  enableVFD();              // turn on the high voltage and the low voltage LDOs
  progToRun = '4';          // prints the time to VFD
  DateTime now = rtc.now(); // get the RTC time
  encodeTime(now.hour(),now.minute());  // prepare to display the RTC time
  rtc.writeAlarm2_minute(); // set up Alarm2 to trigger INT_SQW pin every minute (00 seconds)
}



void loop(){
  blink_LED();  // used for testing
  feelSwitches();       // switches to set time or view battery level
//  checkRTCinterrupt();  // check if we're at the top of the minute
//  if(muxTime) {mux();}  // multiplex to the VFD

//  if((millis() - lastTime > TEST_TIMER) && (progToRun != '4')){  // '4' = 'be a clock'
//    lastTime = millis();
//    runProg(progToRun);   // keeping legacy tests for fun
//  }

}



void serialEvent(){

  while(Serial.available()){
    char inChar = Serial.read();
//    Serial.print("in serialEvent "); Serial.println(inChar);

    switch (inChar){
      case 'e': enableVFD(); break;
      case 'd': disableVFD(); break;
      case '?': printVersion(); break;
//      case 'b': printBatteryLevel(); break;
      case '0': progToRun = inChar; break;  // count up all digits 0-9
      case '1': progToRun = inChar; break;  // count binary nibble
//      case '2': progToRun = inChar; break;  // display battery level
      case '3': progToRun = inChar; break;  // run test 3
      case '4': progToRun = inChar; break;  // display RTC time
      case '5': progToRun = inChar; break;  // set time
      case '6': progToRun = inChar; break;  // blanking blink test
      default:
        Serial.print("i got "); Serial.println(inChar);
        break;
    }
  }
}

void mux(){
  shiftOUT(numToDisplay[grid]);
  muxTime = false;
}

void setUpStuph(){
//  Pin Directions
  pinMode(LED,OUTPUT); digitalWrite(LED,LOW);
  pinMode(VFD_EN,OUTPUT); digitalWrite(VFD_EN,LOW);
  pinMode(DIN,OUTPUT); digitalWrite(DIN,LOW);
  pinMode(CLK,OUTPUT); digitalWrite(CLK,LOW);
  pinMode(LOAD,OUTPUT); digitalWrite(LOAD,LOW);
  pinMode(BLANK,OUTPUT); digitalWrite(BLANK,LOW);
  pinMode(SW_1,INPUT);
  pinMode(SW_2,INPUT);
//  pinMode(BAT_SENS,INPUT);
  pinMode(RTC_INT,INPUT);

//  Some Useful Variables
  lastBlink = millis();
  lastTime = millis();
  digit = 0;
  digitalWrite(LED,HIGH); // to show power on

}

//void getBatteryLevel(){
//  int counts = analogRead(A1);
//  float volts = float(counts)*(3.0/1023.0);
//  battVolts = volts*2.0;
//}

//void printBatteryLevel(){
//  getBatteryLevel();
//  Serial.print(battVolts); Serial.println("V");
//}

//void displayBatteryLevel(){
//  int num;
//  getBatteryLevel();
//  numToDisplay[0] = BATTERY;          // print 'b' to VFD
//  num = int(battVolts*100.0);         // scaling
//  for(int i=3; i>0; i--){
//      numToDisplay[i] = NUM[num%10];  // print ones, tens, hundos
//      num /= 10;
//  }
//}

void blink_LED(){   // used for testing only. blinking is stressful.
  if(millis() - lastBlink > BLINK_TIMER) {
    lastBlink = millis();
    LEDstate = !LEDstate;
    digitalWrite(LED,LEDstate);
  }
}

void printVersion(){  // show what this code does!
  Serial.println("ILV2-7/5 Clock Test Code");
  Serial.println("Press '?' to print this list!");
  Serial.println("Press 'e' to enable VFD");
  Serial.println("Press 'd' to disable VFD");
//  Serial.println("Press 'b' to print battery level");
  Serial.println("Press '0' to count up all segments 0-9");
  Serial.println("Press '1' to count binary nibble 0-F");
//  Serial.println("Press '2' to display battery level");
  Serial.println("Press '3' to run test 3");
  Serial.println("Press '4' to display RTC time");
  Serial.println("Press '5' to set time");
  Serial.println("Press '6' to blink display with blanking");
  Serial.println("\nPress SW1 to set hours");
  Serial.println("\tUse SW2 to increment hours");
  Serial.println("Press SW1 to set minutes");
  Serial.println("\tUse SW2 to increment minutes");
  Serial.println("Press SW1 to set 12hr/24hr display mode");
  Serial.println("\tUse SW2 to toggle 12hr/24hr mode");
  Serial.println("Press SW1 to run clock at set time in set mode");
  Serial.println("\nPress SW2 to toggle between time and battery level");

}



void runProg(char test){
  switch (test){
    case '0': runProg_0(); break;
    case '1': runProg_1(); break;
//    case '2': runProg_2(); break;
    case '3': runProg_3(); break;
    case '4': runProg_4(); break;
    case '5': runProg_5(); break;
    case '6': runProg_6(); break;
    default: 
      Serial.print("I got "); Serial.println(test);
      break;
  }
}

void runProg_0(){ // counts up all segments from 0-9. Hz at TEST_TIMER millis match
  testCounter++;
  if(testCounter > 9) {testCounter = 0;}
  for(int i=0; i<4; i++){
    numToDisplay[i] = NUM[testCounter];
  }
}

void runProg_1(){ // counts binary 0-F. Hz at TEST_TIMER millis match
  byte nibbleBit = 3;
  nibbleCounter++;
  for(int i=0; i<4; i++){
    numToDisplay[i] = NUM[bitRead(nibbleCounter,nibbleBit)];
    nibbleBit--;
  }
}

//void runProg_2(){ // prints battery level. refresh on TEST_TIMER millis match
//  displayBatteryLevel();
//}

void runProg_3(){ // prints each segment position in GRID array
  for(int i=0; i<4; i++){
    numToDisplay[i] = NUM[i];
  }
}

void runProg_4(){ // displays current set time. updates on the minute
  DateTime now = rtc.now();
  encodeTime(now.hour(),now.minute());
}

void runProg_5(){  // updates the display while setting time
  if(timeToSet < 3){
    encodeTime(hours,mins);
  }
}

void runProg_6(){ //
  if(blankState){
    digitalWrite(BLANK,HIGH);
  }else{
    digitalWrite(BLANK,LOW);
  }
  blankState = !blankState;
}

void checkRTCinterrupt(){
  int I = digitalRead(RTC_INT);
  if(I == 0){
    rtc.Alarm2callBack(); // reset the flag in the rtc
    Serial.println("interrupt!");
    DateTime now = rtc.now();
    if(progToRun == '4'){
      encodeTime(now.hour(),now.minute());
    }
  }
}
