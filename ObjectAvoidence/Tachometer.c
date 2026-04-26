// Tachometer.c
// Runs on MSP432
// Provide mid-level functions that initialize ports, take
// angle and distance measurements, and report total travel
// statistics.
// Daniel Valvano
// December 20, 2018
// Modified by J. Tadrous on Aug. 9, 2022
// Modified by J. Tadrous on Feb. 16, 2023 added getSteps function
// Modified by J. Tadrous on July 6, 2023 added getSpaceTime function
// Modified by J. Tadrous on Nov. 2, 2023 removed tachometerGet function

/* This example accompanies the book
   "Embedded Systems: Introduction to Robotics,
   Jonathan W. Valvano, ISBN: 9781074544300, copyright (c) 2019
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/

Simplified BSD License (FreeBSD License)
Copyright (c) 2019, Jonathan Valvano, All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are
those of the authors and should not be interpreted as representing official
policies, either expressed or implied, of the FreeBSD Project.
*/

// Left Encoder A connected to P10.5 (J5)
// Left Encoder B connected to P5.2 (J2.12)
// Right Encoder A connected to P10.4 (J5)
// Right Encoder B connected to P5.0 (J2.13)

#include <stdint.h>
#include "Clock.h"
#include "TA3InputCapture.h"
#include "msp.h"
#include "Tachometer.h"
#include "Precision_Moves.h"



uint16_t Tachometer_FirstRightTime, Tachometer_SecondRightTime;
uint16_t Tachometer_FirstLeftTime, Tachometer_SecondLeftTime;
uint32_t timeAccumulatorLeft=0,
        timeAccumulatorRight=0; // keep track of total time

int Tachometer_RightSteps = 0;     // incremented with every step forward; decremented with every step backward
int Tachometer_LeftSteps = 0;      // incremented with every step forward; decremented with every step backward


void tachometerRightInt(uint16_t currenttime){
    Tachometer_FirstRightTime = Tachometer_SecondRightTime;
    Tachometer_SecondRightTime = currenttime;
    uint16_t tDiff;
    tDiff = currenttime- Tachometer_FirstRightTime;
    timeAccumulatorRight += tDiff;
  if((P5->IN&0x01) == 0){
    // Encoder B is low, so this is a step backward
    Tachometer_RightSteps = Tachometer_RightSteps - 1;
  }else{
    // Encoder B is high, so this is a step forward
    Tachometer_RightSteps = Tachometer_RightSteps + 1;
  }
}

void tachometerLeftInt(uint16_t currenttime){
  Tachometer_FirstLeftTime = Tachometer_SecondLeftTime;
  Tachometer_SecondLeftTime = currenttime;
  uint16_t tDiff;
  tDiff = currenttime- Tachometer_FirstLeftTime;
  timeAccumulatorLeft += tDiff;
  if((P5->IN&0x04) == 0){
    // Encoder B is low, so this is a step backward
    Tachometer_LeftSteps = Tachometer_LeftSteps - 1;
  }else{
    // Encoder B is high, so this is a step backward
    Tachometer_LeftSteps = Tachometer_LeftSteps + 1;
  }
}


// ------------Tachometer_Init------------
// Initialize GPIO pins for input, which will be
// used to determine the direction of rotation.
// Initialize the input capture interface, which
// will be used to measure the speed of rotation.
// Input: none
// Output: none
void Tachometer_Init(void){
  // initialize P5.0 and P5.2 and make them GPIO inputs
  P5->SEL0 &= ~0x05;
  P5->SEL1 &= ~0x05;               // configure P5.0 and P5.2 as GPIO
  P5->DIR &= ~0x05;                // make P5.0 and P5.2 in
  TimerA3Capture_Init01(&tachometerRightInt, &tachometerLeftInt);
}



// ------------Tachometer_Get_Steps------------
// Get the most recent tachometer measured steps.
// Input:
//        leftSteps  is pointer to store total number of forward steps measured for left wheel (360 steps per ~220 mm circumference)
//
//        rightSteps is pointer to store total number of forward steps measured for right wheel (360 steps per ~220 mm circumference)
// Output: none
// Assumes: Tachometer_Init() has been called
// Assumes: Clock_Init48MHz() has been called
// By J. Tadrous on 2/16/2023
void Tachometer_Get_Steps(int32_t *leftSteps, int32_t *rightSteps){

    *leftSteps = Tachometer_LeftSteps;
    *rightSteps = Tachometer_RightSteps;

}

// ------------Tachometer_Get_SpaceTime------------
// Get the most recent tachometer measured steps and time.
// Input:
//        leftSteps  is pointer to store total number of forward steps measured for left wheel (360 steps per ~220 mm circumference)
//        rightSteps is pointer to store total number of forward steps measured for right wheel (360 steps per ~220 mm circumference)
//        leftTime   is a pointer to store aggregate time (clock cycles) spent between ticks of left wheel
//        rightTime is a pointer to store aggregate time (clock cycles) spent between ticks of right wheel
// Output: none
// Assumes: Tachometer_Init() has been called
// Assumes: Clock_Init48MHz() has been called
// By J. Tadrous on 7/6/2023
void Tachometer_Get_SpaceTime(int32_t *leftSteps, int32_t *rightSteps, uint32_t *leftTime, uint32_t *rightTime){
    *leftSteps = Tachometer_LeftSteps;
    *rightSteps = Tachometer_RightSteps;
    *leftTime = timeAccumulatorLeft;
    *rightTime = timeAccumulatorRight;
}
