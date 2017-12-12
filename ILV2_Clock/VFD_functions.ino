/*

  Functions for controlling the MAX6920 VFD display driver

*/



void shiftOUT(byte d)
{
    int output = 0;
    boolean bitVal;
    output = d | (GRID[grid] << 8); // grid is incremented in ISR
  
    checkForBlanking(d);
  
    if((grid%2 == 0)&&(!blankDot)){
      output |= DOT; // illuminate the dots
    }
  
  for(int i=11; i>=0; i--){
    digitalWrite(CLK,LOW);  // prime the pump
    bitVal = bitRead(output,i);
    digitalWrite(DIN,bitVal);  // prepare the bit
    digitalWrite(CLK,HIGH); // shift out on rising edge
    delayMicroseconds(2);
  }
  digitalWrite(LOAD,HIGH);  // pass data to the data
  delayMicroseconds(2);
  digitalWrite(LOAD,LOW);   // latch the data
}

void checkForBlanking(byte d){
  unsigned int thisTime = millis();
  digitalWrite(BLANK,LOW); // enable display driver

  if(timeToSet == 1){
    if((grid == 0 || grid == 1) ){
      if(thisTime-lastBlankTime > blankTime){
        lastBlankTime = thisTime;
        blankState = !blankState;
      }
      digitalWrite(BLANK,blankState);
    }
  } 
  if(timeToSet == 2){
    if(grid == 2 || grid == 3){
      if(thisTime-lastBlankTime > blankTime){
        lastBlankTime = thisTime;
        blankState = !blankState;
      }
      digitalWrite(BLANK,blankState);
    }
  }
  if(timeToSet == 3){
    if(thisTime-lastBlankTime > blankTime){
        lastBlankTime = thisTime;
        blankDot = !blankDot;
      }
  }
  
  if(grid == 0 && d == ZERO){
    digitalWrite(BLANK,HIGH);
  }
}

void enableVFD(){
  digitalWrite(VFD_EN,HIGH); // turn on the high voltage and 2.5V drives
  VFD_ON = true;
  Serial.println("VFD Enabled");
  testGrid = 0;
  testCounter = 0;
  digitalWrite(BLANK,LOW);  // enable driver output to VFD
}

void disableVFD(){
  digitalWrite(VFD_EN,LOW); // turn off the high voltage and 2.5V drives
  VFD_ON = false;
  Serial.println("VFD Disabled");
  digitalWrite(BLANK,HIGH); // disable driver output to VFD
}
