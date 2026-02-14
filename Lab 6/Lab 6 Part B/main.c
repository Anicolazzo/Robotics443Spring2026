#include "msp.h"
#include "BumpInt.h"
#include "Reflectance.h"
#include "Motor.h"
#include "Clock.h"
#include "RobotLights.h"
#include "ADC14.h"
#include "RobotLights.h"
/**
 * main.c
 */
// FOR ANYONE WHO SEES THIS EVERYTHING IS BACKWARDS ITS ALL INVERTED RIGHT IS LEFT AND LEFT IS RIGHT YOU  HAVE BEEN WARNED

#define colisionDetectCenter 10300
#define colisionDetectRight 11000
#define colisionDetectLeft 10400

void main(void)
{
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer
    BumpInt_Init(BumpTask);
    Clock_Init48MHz();
    MvtLED_Init();
    Motor_Init();
    Reflectance_Init();
    ADC0_InitSWTriggerCh17_14_16();
    uint32_t left, right, center;
    Motor_Forward(2500, 2500);
    while (1)
    {
        ADC_In17_14_16(&left, &center, &right);
        Clock_Delay1ms(100);
        if (left >= colisionDetectLeft && right >= colisionDetectRight)
        {
            // turn around
            Motor_Stop();
            Motor_Left(2500, 2500);
            Clock_Delay1ms(2500);
            Motor_Stop();
        }
        else if (right >= colisionDetectRight)
        {
            Motor_Left(2500, 2500);
            Clock_Delay1ms(1700);
            Motor_Stop();
        }
        else if (left >= colisionDetectLeft)
        {
            Motor_Right(2500, 2500);
            Clock_Delay1ms(1700);
            Motor_Stop();
        }
        else if (center >= colisionDetectCenter)
        {
            // back up for half a second
            Motor_Stop();
            Motor_Backward(2500, 2500);
            Clock_Delay1ms(500);
            Motor_Stop();
            Motor_Left(2500, 2500);
            Clock_Delay1ms(2500);
            Motor_Stop();
            // Turn to the right
        }
        else
        {
            Motor_Forward(2500, 2500);
        }
        Clock_Delay1ms(100);
    }
}
