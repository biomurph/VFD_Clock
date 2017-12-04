/*

  Functions for controlling the MAX6920 VFD display driver

*/



void shiftOUT(byte d)
{
  int output = 0;
  boolean bitVal;
  output = d | (GRID[grid] << 8); // grid is incremented in ISR
  if(grid%2 == 0) output |= DOT; // test this for brightness
  
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


void enableVFD(){
  digitalWrite(VFD_EN,HIGH); // turn on the high voltage and 2.5V drives
  VFD_ON = true;
  Serial.println("VFD Enabled");
  testGrid = 0;
  testCounter = 0;
  digitalWrite(BLANK,LOW);
}

void disableVFD(){
  digitalWrite(VFD_EN,LOW); // turn off the high voltage and 2.5V drives
  VFD_ON = false;
  Serial.println("VFD Disabled");
  digitalWrite(BLANK,HIGH);
}
