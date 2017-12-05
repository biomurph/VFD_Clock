

void readRTCregisters(){
    DateTime now = rtc.now();

    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
//    Serial.print(':');
//    Serial.print(now.second(), DEC);
//    Serial.println();
  
}

void encodeTime(int h, int m){
  
  
  Serial.print(h, DEC);
  Serial.print(':');
  Serial.println(m, DEC);

  numToDisplay[1] = NUM[h%10];
  h /= 10;
  numToDisplay[0] = NUM[h];
  numToDisplay[3] = NUM[m%10];
  m /= 10;
  numToDisplay[2] = NUM[m];
  
}


// This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
 void updateRTC(int h, int m){
  rtc.adjust(DateTime(2017, 1, 21, h, m, 0));
 }
