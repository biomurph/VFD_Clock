/*

    Defines for driving the ILV2-7/5 VFD Display tube
    Targets a PIC32MX250F128B uC
    With MAX6920 HV SIPO, DS3231M RTC, 2 Pushbutton Switches, and Batt Level Monitor

    github.com/biomurph

    Made by Joel Murphy, Winter 2017
    Use with care

*/
#include <Arduino.h>

#ifndef _ILV2_Defines_h_
#define _ILV2_Defines_h_


/***************************************************************
    PINS Based On ChipKIT DP32 Pinout
***************************************************************/
#define LED 11  // also PGD
#define SW_1 13
#define SW_2 17 // also BOOT_EN
#define BAT_SENS 1  // also GPIO 7
#define INT_SQW 8
#define DIN 18
#define CLK 0   // data shifted from DIN on rising edge
#define LOAD 1  // Passes data to outputs on rising edge, Latch on falling edge
#define BLANK 6 // Active HIGH PWM-able!
#define VFD_EN 14

/***************************************************************
  VFD Drive Binaries
  MAX6920 is a 12bit SIPO Latch
  Data is shifted out MSBfirst
  DOT also control the DOT grid

  0 a
  1 b                  _a_
  2 c                f|   |b
  3 d                 |_g_|
  4 e                e|   |c
  5 f                 |___|
  6 g                   d
  7 DOT
  8 GRID_1
  9 GRID_2
  10  GRID_4
  11  GRID_5

***************************************************************/

#define ZERO  0x3F
#define ONE   0x06
#define TWO   0x5B
#define THREE 0x4F
#define FOUR  0x66
#define FIVE  0x6D
#define SIX   0x7D
#define SEVEN 0x07
#define EIGHT 0x7F
#define NINE  0x6F
#define DOT     0x80

#define GRID_1  0x01
#define GRID_2  0x02
#define GRID_4  0x04
#define GRID_5  0x08

#define BATTERY 0x7C


/***************************************************************
    TIMER 3 INTERRUPT CONTROL  
 ***************************************************************/
#define T3CON_ENABLE_BIT 0x8000
#define T3CON_PRESCALER_BITS 0x0070
#define T3_SOURCE_INT 0

// Prescaler values
// Don't change these. Set the prescaler below using these.
#define T3_PRESCALE_1_1 0
#define T3_PRESCALE_1_2 1
#define T3_PRESCALE_1_4 2
#define T3_PRESCALE_1_8 3
#define T3_PRESCALE_1_16 4
#define T3_PRESCALE_1_32 5
#define T3_PRESCALE_1_64 6
#define T3_PRESCALE_1_256 7

/***************************************************************
   TIMER3 USER DEFINED VARIABLES
 ***************************************************************/
// Set the prescaler value we want to use
#define PRESCALE T3_PRESCALE_1_64    // Use chart above to set range

// The DP32 runs at 40 MHz
// The uC32 and WF32 run at 80 MHz
#define CLOCK_FREQ 40000000   // Verified with scope Y/N

// Set our target frequency
// This is the frequency that our interrupt will run at
//#define INT_FREQUENCY 400     // SET THIS IN THE SETUP

/***************************************************************
DS3231 Defines
 ***************************************************************/
#define RTC_ADD 0x68

#endif
