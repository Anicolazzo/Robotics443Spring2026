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
#define  colisionDetectLeft 11000
#define  colisionDetectRight 10400

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
    //Motor_Forward(2500, 2500);
    while (1)
    {
        ADC_In17_14_16(&right, &center, &left);
        Clock_Delay1ms(100);
        if(center > 9000 && center <13000)
        {
            Motor_Stop();
        }
        else if(center > 13000)
        {
            Motor_Backward(2500, 2500);
        }
        else if (right >= colisionDetectRight-5000 )
        {
            Motor_Right(2500, 2500);
        }
        else if (left >= colisionDetectLeft-5000 )
        {
            Motor_Left(2500, 2500);
        }
        else if (center >= colisionDetectCenter-5000)
        {
            Motor_Forward(2500, 2500);
            // Turn to the right
        }
        else
        {
            Motor_Stop();
        }
        Clock_Delay1ms(1);
    }
}
