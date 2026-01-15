
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
//    printf("Button reg status = %d\n", P1->IN);
//    printf("Expected Output %d \n", (P1->IN & 0x12));
//    printf("1 = true 0 = false Status: %d \n", ((P1->IN & 0x12) == 0x00) ? 1: 0 );
    return (P1->IN & 0x12) == 0x00; // Pull down switches 0 means pressed
}

void cycleLED()
{
   static bool stopRepeat = false;
   static uint8_t counter = 0x00;
   if (checkSWStatus() && !stopRepeat)
   {
       stopRepeat = true;
       P2->OUT = counter;
       if(counter==0x7)
           counter = 0;
       counter ++;
   }
   else if(!checkSWStatus() && stopRepeat)
   {
       stopRepeat = false;
   }
}
void main(void)
{
    Clock_Init48MHz();
    initSwitches();
    initOnboardLED();
    P2->OUT = 0b111;
    while(1)
    {
        cycleLED();
    }
}
