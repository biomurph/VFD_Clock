


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
          if(progToRun == '5'){
            incrementTime();  // increment the time setting
            return;
          } else if(progToRun == '4'){
            progToRun = '2';  // set to show battery level
            return;
          } else if(progToRun == '2'){
            progToRun = '4';  // set to tell time
            DateTime now = rtc.now();
            encodeTime(now.hour(),now.minute());
            return;
          }
        }
        if(i == 1 && switchState == 1){
          if(progToRun == '4'){
            progToRun = '5';  // set time
            DateTime now = rtc.now();
            hours = now.hour();
            mins = now.minute();
            timeToSet = 1;  // prep to set hours
            return;
          } else {
            timeToSet++;
            if(timeToSet == 3){
              timeToSet = 0;
              updateRTC(hours,mins);
              progToRun = '4';  // done setting time!
            }
          }
        }
      }
   }  
}
