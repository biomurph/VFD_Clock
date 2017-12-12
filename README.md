# VFD_Clock

This code is designed to target hardwarea found here

`https://github.com/biomurph/ILV2-7-5_VFD_Display`

## Features

* ILV2-7/5 Vacuum Flourescent Display tube
* ChipKIT DP32 bootloaded PICMX250F128B with USB programming
* DS3231 Real Time Clock with backup battery
* MAX6920 VFD SIPO driver

## This work is based In Part On:

  **Work with ILV2-5/7 tubes by Callum Nunez-Vaz**
  `https://callumnunesvaz.wordpress.com/portfolio/dogbonevfd/`
  `http://www.johngineer.com/blog/?p=1595`
  
  **Work with Timer 3 control by Jay Weeks**
  `https://www.instructables.com/id/Timer-Interrupts-on-the-DP32/`

  **Adafruit's RTClib**
  
  Forked with modifications to access Alarm2 for interrupts on the minute (seconds = 00)
  `https://github.com/biomurph/RTClib`

