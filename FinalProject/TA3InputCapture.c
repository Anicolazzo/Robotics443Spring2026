// TA3InputCapture.c
// Runs on MSP432
// Use Timer A3 in capture mode to request interrupts on rising
// edges of P10.4 (TA3CCP0) and P8.2 (TA3CCP2) and call user
// functions.
// Use Timer A3 in capture mode to request interrupts on rising
// edges of P10.4 (TA3CCP0) and P10.5 (TA3CCP1) and call user
// functions.
// Daniel Valvano
// July 11, 2019
// Completed by J. Tadrous
// August 2022

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

// external signal connected to P10.5 (TA3CCP1) (trigger on rising edge)
// external signal connected to P10.4 (TA3CCP0) (trigger on rising edge)

#include <stdint.h>
#include "msp.h"

//void ta3dummy(uint16_t t){};       // dummy function
void (*CaptureTask0)(uint16_t time);// = ta3dummy;// user function
void (*CaptureTask1)(uint16_t time);// = ta3dummy;// user function


//------------TimerA3Capture_Init01------------
// Initialize Timer A3 in edge time mode to request interrupts on
// the rising edges of P10.4 (TA3CCP0) - right motor - and P10.5 (TA3CCP1) - left motor.
// The interrupt service routines acknowledge the interrupt and call
// a user function.
// Input: task0 is a pointer to a user function called when P10.4 (TA3CCP0) right motor edge occurs
//              parameter is 16-bit up-counting timer value when P10.4 (TA3CCP0) edge occurred (units of 0.083 usec)
//        task1 is a pointer to a user function called when P10.5 (TA3CCP1) left motor edge occurs
//              parameter is 16-bit up-counting timer value when P10.5 (TA3CCP1) edge occurred (units of 0.083 usec)
// Output: none
// Assumes: low-speed subsystem master clock is 12 MHz
void TimerA3Capture_Init01(void(*task0)(uint16_t time), void(*task1)(uint16_t time)){
    CaptureTask0 = task0;
    CaptureTask1 = task1;
    // write this for Lab 16
    // initialize P10.4 and P10.5 for input capture
    P10 -> SEL0 |= 0x30; //  TA3CCP1 & TA3CCP0
    P10 -> SEL1 &= ~0x30; // TA3CCP1 & TA3CCP0
    P10 -> DIR &= ~0x30; // input pins

    TIMER_A3 -> CTL &= ~0x0030; // Halt Timer A3 (MC=00 stop mode - bits 5,4 of TIMER_A3->CTL)
    TIMER_A3 ->EX0 &=~0x07; // TAIDX pre-scaler = 1
    TIMER_A3 -> CTL =0x200; // TASSEL =10 SMCLK, ID = 00 prescaler = 1,
    TIMER_A3 ->CCTL[0] = 0x4910; // CM=01 Rising, CCIS=00 Pin input, SCS=1 Sync., CAP=1 Capture, CCIE=1 Arm interrupts.
    TIMER_A3 ->CCTL[1] = 0x4910; // Same for TA3CCP1
    // NVIC Config.
    NVIC -> IP[3] = (NVIC -> IP[3] & 0x0000FFFF) | 0x40400000; // Set priority level 2 to both TA3_0 (pin 0) and TA3_N (other pins)
    NVIC -> ISER[0] |= 0xC000;

    // Let's run the timer
    TIMER_A3 ->CTL |= 0x024; //  ID prescaler (00), now ID = 11, MC continuous up (10), TACLR (1)

}

void TA3_0_IRQHandler(void){
    // write this for Lab 16
    // A rising edge on TA3.0 - P10.4
    TIMER_A3 -> CCTL[0] &=~0x01; // clear bit 0 (CCIFG) to ACK interrupt.
    (*CaptureTask0)(TIMER_A3 -> CCR[0]);
}

void TA3_N_IRQHandler(void){
    // write this for Lab 16
    // A rising edge on TA3.1 - P10.5
    TIMER_A3 -> CCTL[1] &=~0x01; // clear bit (CCIFG) to ACK interrupt
    (*CaptureTask1)(TIMER_A3 -> CCR[1]);


}

