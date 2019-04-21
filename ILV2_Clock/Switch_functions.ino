


void feelSwitches(){
  boolean switchState;
  for(int i=0; i<2; i++){
    switchState = digitalRead(SWITCH[i]);
    if(switchState != lastSwitchState[i]){
      debounce[i] = true;
      debounceTime = millis();
      lastSwitchState[i] = switchState;
    }
    if(debounce[i] && (millis() - debounceTime > 10)){
      debounce[i] = false;
      
        if(i == 0 && switchState == 1){
          Serial.println("SW 0 pressed");
          if(progToRun == '5'){
            if(timeToSet < 3){
              incrementTime();  // increment the time setting
              return;
            }else if(timeToSet == 3){
              if(displayMode == 12){ displayMode = 24; return;}
              if(displayMode == 24){ displayMode = 12; return;}
              return;
            }
          } else if(progToRun == '4'){
            progToRun = '2';  // show alarm with option to set
            Serial.println("show alarm");
            return;
          } else if(progToRun == '2'){
            progToRun = '4';  // set to tell time
            Serial.println("show time");
            DateTime now = rtc.now();
            encodeTime(now.hour(),now.minute());
            return;
          }
        }
        
        if(i == 1 && switchState == 1){
          Serial.println("SW 1 pressed");
          if(progToRun == '4'){
            progToRun = '5';  // set time
            Serial.println("set time");
            DateTime now = rtc.now();
            hours = now.hour();
            mins = now.minute();
            timeToSet = 1;  // prep to set hours
            return;
          } else {
            timeToSet++;
            Serial.print("increment timeToSet "); Serial.println(timeToSet);
            if(timeToSet == 3){
              Serial.println("incrementTime");
              incrementTime();
            }
            if(timeToSet == 4){
              Serial.println("done setting time, prog 4");
              timeToSet = 0;
              updateRTC(hours,mins);
              blankDot = false;
              progToRun = '4';  // done setting time!
            }
          }
        }
      }
   }  
}

void incrementTime(){ // used to set hours, minutes, and 12/24 display
  
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
    case 3:
      selectDisplayMode();
      break;
    default:
      break;
    
  }
}

void selectDisplayMode(){

  switch (displayMode){
    case 12:
      Serial.println("12 hour");
      numToDisplay[0] = TWO;
      numToDisplay[1] = FOUR;
      displayMode = 24;
      break;
    case 24:
      Serial.println("24 hour");
      numToDisplay[0] = ONE;
      numToDisplay[1] = TWO;
      displayMode = 12;
      break;
    default: break;
  }
  numToDisplay[2] = H;
  numToDisplay[3] = R;
  
}
