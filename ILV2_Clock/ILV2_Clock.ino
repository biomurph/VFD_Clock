/*

  ILV2-7/5 Clock
  Targets a PIC32MX250F128B with ChipKIT DP32 bootloader
  Uses the MAX6920 VFD SIPO driver
  Uses DS3231 RTC
  Has V-Divider on Battery for Battery level sensing
  
  
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
boolean debounce[2] = {false,false};
unsigned int debounceTime;
unsigned long lastBlink;  
int blinkTimer = 300; // LED blink rate
int testTimer = 300;  // test timing
unsigned int lastTime = 0; // test timing
int testCounter = 0;
byte nibbleCounter = 0;
int progToRun = 0;
int timeToSet = 0;
int hours;
int mins;
boolean blankState = false;
unsigned int lastBlankTime = 0;
int blankTime = 200;

boolean VFD_ON = false;
int testGrid = 0;

boolean LEDstate = false;

float battVolts = 0.0;

boolean fade = false;
boolean goingUp = true;

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
  setUpStuph(); // pin directions an variables
//  FIRE UP TIMER 3
  fireUpTimer3(400);  // send the setup with Hz for Timer 3
  enableVFD();
  progToRun = '4';  // prints the time to VFD
  DateTime now = rtc.now(); // get the RTC time
  encodeTime(now.hour(),now.minute());  // display the RTC time
  rtc.writeAlarm2_minute(); // sets up Alarm2 to trigger INT_SQW pin on the minute
}



void loop(){
//  blink_LED();  // used for testing
  feelSwitches();
  checkRTCinterrupt();
  if(muxTime) {mux();}  // print to the VFD
  
  if((millis() - lastTime > testTimer) && (progToRun != '4')){
    lastTime = millis();
    runProg(progToRun);
  }
  
}



void serialEvent(){
  
  while(Serial.available()){
    char inChar = Serial.read();
//    Serial.print("in serialEvent "); Serial.println(inChar);
    
    switch (inChar){
      case 'e': enableVFD(); break;
      case 'd': disableVFD(); break;
      case '?': printVersion(); break;
      case 'b': printBatteryLevel(); break;
      case '0': progToRun = inChar; break;  // count up all digits 0-9
      case '1': progToRun = inChar; break;  // count binary nibble
      case '2': progToRun = inChar; break;  // display battery level
      case '3': progToRun = inChar; break;  // run test 3
      case '4': progToRun = inChar; break;  // display RTC time
      case '5': progToRun = inChar; break;  // ?
      case '6': progToRun = inChar; break;  // pwm test
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
  pinMode(BAT_SENS,INPUT);
  pinMode(RTC_INT,INPUT);

//  Some Useful Variables
  lastBlink = millis();
  lastTime = millis();
  digit = 0;
  digitalWrite(LED,HIGH); // to show power on
  
}

void getBatteryLevel(){
  int counts = analogRead(A1);
  float volts = float(counts)*(3.0/1023.0);
  battVolts = volts*2.0;
//  Serial.print(counts);Serial.print(", ");
//  Serial.print(volts); Serial.print(", ");
//  Serial.println(battVolts); 
}

void printBatteryLevel(){
  getBatteryLevel();
  Serial.print(battVolts); Serial.println("V");
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
  Serial.println("Press '0' to count up all segments");
  Serial.println("Press '1' to count binary nibble");
  Serial.println("Press '2' to display battery level");
  Serial.println("Press '3' to run test 3");
  Serial.println("Press '4' to display RTC time");
  Serial.println("Press '5' to set time");
  Serial.println("Press '6' to scroll pwm fade");
  Serial.println("Press SW1 to set hours");
  Serial.println("\tUse SW2 to increment hours");
  Serial.println("\tPress SW1 to set minutes");
  Serial.println("\tUse SW2 to increment minutes");
  Serial.println("\tPress SW1 to run clock");
  Serial.println("Press SW2 to toggle between time and battery level");
  Serial.println("Press '?' to print this list!");
  
}



void runProg(char test){
  switch (test){
    case '0': runProg_0(); break;
    case '1': runProg_1(); break;
    case '2': runProg_2(); break;
    case '3': runProg_3(); break;
    case '4': runProg_4(); break;
    case '5': runProg_5(); break;
    case '6': runProg_6(); break;
    default: break;
  }
}

void runProg_0(){
  testCounter++; 
  if(testCounter > 9) {testCounter = 0;}
  for(int i=0; i<4; i++){
    numToDisplay[i] = NUM[testCounter];
  }
}

void runProg_1(){
  byte nibbleBit = 3;
  nibbleCounter++;
  for(int i=0; i<4; i++){
    numToDisplay[i] = NUM[bitRead(nibbleCounter,nibbleBit)];
    nibbleBit--;
  }
}

void runProg_2(){
  displayBatteryLevel();
}

void runProg_3(){
  for(int i=0; i<4; i++){
    numToDisplay[i] = NUM[i]; 
  }
}

void runProg_4(){
  DateTime now = rtc.now();
  encodeTime(now.hour(),now.minute());
}

void runProg_5(){
//  encodeTime(hours,mins);
}

void runProg_6(){
  if(blankState){
    digitalWrite(BLANK,HIGH);
  }else{
    digitalWrite(BLANK,LOW);
  }
  blankState = !blankState;
}

void incrementTime(){
  
  switch(timeToSet){
    case 1:
      hours++;
      if(hours > 23) {hours = 0;}
      encodeTime(hours,mins);
      break;
    case 2:
      mins++;
      if(mins > 59) {mins = 0;}
      encodeTime(hours,mins);
      break;

    default:
      break;
    
  }
}


void checkRTCinterrupt(){
  int I = digitalRead(RTC_INT);
  if(I == 0){
    rtc.callBackToAlarm2(); // reset the flag in the rtc
    Serial.println("interrupt!");
    DateTime now = rtc.now();
    if(progToRun == '4'){
      encodeTime(now.hour(),now.minute());
    }
  }
}



