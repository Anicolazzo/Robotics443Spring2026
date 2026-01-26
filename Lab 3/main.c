#include "msp.h"
#include "Motor.h"
#include "Reflectance.h"
#include "Clock.h"
#include <stdbool.h>
#include "RobotLights.h"
/**
 * main.c
 */

void initSwitches()
{
    // SEL1 and SEL0 together set the pin into GPIO mode must both be 00
    P1->SEL1 &= ~0x12; // 43210, 0x02 corresponds to SW1 0x10 corresponds to SW2
    P1->SEL0 &= ~0x12;
    P1->DIR &= ~0x02; // Direction input
    P1->REN |= 0x12; // Engage resistor for SW1
    P1->OUT |= 0x12; // Make it pull-up resistor
}

bool checkSWStatus() // return true if switch is pressed
{
//    printf("Button reg status = %d\n", P1->IN);
//    printf("Expected Output %d \n", (P1->IN & 0x12));
//    printf("1 = true 0 = false Status: %d \n", ((P1->IN & 0x12) == 0x00) ? 1: 0 );
    return (P1->IN & 0x10) == 0x00; // Pull down switches 0 means pressed
}

void main(void)
{
    Clock_Init48MHz();
    initSwitches();
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer
    Motor_Init();
    Motor_Stop();
    MvtLED_Init();
    Reflectance_Init();
    uint8_t RefData = 0;
    while (1)
    {
        while (!checkSWStatus())
        {
        }
        RefData = 0;
        while (RefData == 0)
        {
            Motor_Forward(10000, 10000);
            Front_Lights_ON();
            Clock_Delay1ms(10);
            RefData = Reflectance_Read(1000);

        }
        Front_Lights_OFF();
        Motor_Stop();
        Clock_Delay1ms(100);
        RefData = Reflectance_Read(1000);
        RefData = 0;

        while (RefData == 0)
        {
            Back_Lights_ON();
            Motor_Backward(2500, 2500);
            Clock_Delay1ms(1);
            RefData = Reflectance_Read(1000);
        }
        Back_Lights_OFF();
        Motor_Stop();
    }
}
