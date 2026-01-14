
#include "msp.h"
#include "Clock.h"
#include <stdbool.h>

void initSwitches();
void initOnboardLED();
bool checkSWStatus();
/**
 * main.c
 */


void initSwitches()
{
    // SEL1 and SEL0 together set the pin into GPIO mode must both be 00
    P1 ->SEL1 &= ~0x12; // 43210, 0x02 corresponds to SW1 0x10 corresponds to SW2
    P1 ->SEL0 &= ~0x12;
    P1 ->DIR &= ~0x02; // Direction input
    P1 ->REN |=0x12; // Engage resistor for SW1
    P1 ->OUT |=0x12; // Make it pull-up resistor
}

void initOnboardLED()
{
    //BRG Led P2 pins 2-0
    P2 ->SEL1 &= ~0x07;
    P2 ->SEL0 &= ~0x07;
    P2 ->DIR |= 0x07; // Direction Output
}

bool checkSWStatus() // return true if switch is pressed
{
    return (P1->IN & 0x12) == 0x00; // Pull down switches 0 means pressed
}

void cycleLED()
{
   static uint8_t counter = 0x00;
   if (checkSWStatus())
   {
       P2->OUT = counter;
       if(counter==0x7)
           counter = 0;
   }
}
void main(void)
{
    Clock_Init48MHz();
    initSwitches();
    initOnboardLED();
    while(1)
    {
        cycleLED();
    }
}
