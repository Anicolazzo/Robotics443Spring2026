#include <RobotLights.h>
#include "msp.h"
/*
 * MovementLights.c
 *
 *  Created on: Oct 23, 2022
 *      Author: John Tadrous
 */

/* MovementLights initializes and supports the LEDs on the chassis board and the
 * colored BGR LED on the MSP432 launchpad
 * BGR lights: P2.2-P2.0
 * LED Front Right: P8.5
 * LED Front Left: P8.0
 * LED Rear Right: P8.7
 * LED Rear Left: P8.6
 */

void MvtLED_Init(){
    // Front and back LEDS on port 8 0b1110001 or 0x71 back leds are 0x10 and 0x01 while front are 0x20 and 0x40
    P8 ->SEL1 &= ~0x71; // First function select 0
    P8 ->SEL0 &= ~0x71; // Second function select 1
    P8 ->DIR |= 0x71; // Direction output
}

// Turn on white light and chasis front lights
void Front_Lights_ON(){
   P8->OUT |= 0x60;
}

// Switch off BGR and chasis front lights
void Front_Lights_OFF(){
   P8->OUT &= ~0x60;
}

// Turn on back lights
void Back_Lights_ON(){
   P8->OUT |= 0x11;
}

// Turn off back lights
void Back_Lights_OFF(){
    P8->OUT &= ~0x11;
}
