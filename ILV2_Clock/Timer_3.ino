/*

  Instructable for this code: https://www.instructables.com/id/Timer-Interrupts-on-the-DP32/

  Prescaler  Max frequency Min frequency (According to Jay Weeks https://www.instructables.com/member/JayWeeks/)
    1:1               40 MHz     611 Hz     T3_PRESCALE_1_1
    1:2               20 MHz     306 Hz     T3_PRESCALE_1_2
    1:4               10 MHz     153 Hz     T3_PRESCALE_1_4
    1:8                5 MHz     77 Hz      T3_PRESCALE_1_8
    1:16             2.5 MHz     39 Hz      T3_PRESCALE_1_16
    1:32            1.25 MHz     20 Hz      T3_PRESCALE_1_32
    1:64             625 kHz     10 Hz      T3_PRESCALE_1_64
    1:256            312 kHz      5 Hz      T3_PRESCALE_1_256
*/

void fireUpTimer3(int Hz){
  start_timer_3(Hz);// set desired Hz
  setIntVector(_TIMER_3_VECTOR, MUX_TIMER);// point to ISR
  setIntPriority(_TIMER_3_VECTOR, 4, 0);// set priority
  clearIntFlag(_TIMER_3_IRQ);// Clear hardware flag
  setIntEnable(_TIMER_3_IRQ);// Enable 
}

/********************************
   Timer and interrupt setup function
   This should just work. Don't look too hard
 ********************************/
void start_timer_3(uint32_t frequency)
{
  uint32_t period, mask;

  // Calculate the period we need for our given frequency
  if (PRESCALE == 7) period = 256; // 1:256 is a special case
  else period = 1 << PRESCALE;
  period = period * frequency;
  period = CLOCK_FREQ / period;
  // Summary of the above equation:
  // period = CLOCK_FREQ / (1 << PRESCALE * frequency)
  // We can't just use this equation because for some reason it
  // doesn't work...

  // Set up our timer
  T3CONCLR = T3CON_ENABLE_BIT;            // Turn the timer off
  T3CONCLR = T3CON_PRESCALER_BITS;        // Clear the old prescaler
  mask = PRESCALE << 4;                   // Shift our new prescaler
  mask = mask | T3CON;                    // Mask our prescaler
  T3CON = mask;                           // Set the prescaler
  TMR3 = 0;                               // Clear the counter
  PR3 = period;                           // Set the period
  T3CONSET = T3CON_ENABLE_BIT;            // Turn the timer on
}

/********************************
   Setup Example
 ********************************
void setup()
{
  // Start our timer with the given frequency
  start_timer_3(INT_FREQUENCY); // The definition of this function is above

  // Set our interrupt service routine to the myISR function above
  setIntVector(_TIMER_3_VECTOR, myISR);

  // The interrupt priority tells us how important the interrupt is
  setIntPriority(_TIMER_3_VECTOR, 4, 0);

  // If this is left high, the interrupt will execute immediately
  clearIntFlag(_TIMER_3_IRQ);

  // Enable our interrupt so it can run!
  setIntEnable(_TIMER_3_IRQ);

  // Set our LEDs as outputs
  pinMode(PIN_LED1, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);
  pinMode(PIN_LED3, OUTPUT);
  pinMode(PIN_LED4, OUTPUT);
}
/
/********************************
   Loop Example
 ********************************
//void loop()
//{
//  if (flag) // Has our interrupt been run since last time?
//  { // If it has:
//    digitalWrite(PIN_LED1, HIGH); // Set our interrupt flag indicator LED high
//
//    // LED 2 represents the value of our least significant bit
//    if (count & (0b001 << BIT_SHIFT)) digitalWrite(PIN_LED2, HIGH);
//    else digitalWrite(PIN_LED2, LOW);
//
//    // LED 3 represents the next bit
//    if (count & (0b010 << BIT_SHIFT)) digitalWrite(PIN_LED3, HIGH);
//    else digitalWrite(PIN_LED3, LOW);
//
//    // LED 4 represents the least significant bit
//    if (count & (0b100 << BIT_SHIFT)) digitalWrite(PIN_LED4, HIGH);
//    else digitalWrite(PIN_LED4, LOW);
//    
//    flag = 0; // Reset our interrupt flag
//  } else digitalWrite(PIN_LED1, LOW); // Otherwise set our interrupt flag indicator low
//}
/
