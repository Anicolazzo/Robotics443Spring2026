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

#define colisionDetectCenter 13500
#define colisionDetectRight 7400
#define colisionDetectLeft 7400

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

    Motor_Forward(1900, 1900);

    while (1)
    {
        ADC_In17_14_16(&right, &center, &left);
        // New behavior:
        // 1) If center >= collisionDetectCenter AND right <= colisionDetectRight -> turn right
        // 2) Else if center >= collisionDetectCenter AND right >= colisionDetectRight -> turn left
        // 3) Else if center <= collisionDetectCenter -> go forward

        if (center >= colisionDetectCenter && right <= colisionDetectRight)
        {
            Motor_Stop();
            Motor_Right(1900, 1900);
            Clock_Delay1ms(1500);
            Motor_Stop();
        }
        else if (center >= colisionDetectCenter && right >= colisionDetectRight)
        {
            Motor_Stop();
            Motor_Left(1900, 1900);
            Clock_Delay1ms(1600);
            Motor_Stop();
        }
        else if (right <= 4500 && center <= 3000)
        {
            Motor_Forward(1900, 1900);
            Clock_Delay1ms(3500);
            ADC_In17_14_16(&right, &center, &left);
            if (right >= 11000)
            {
                Motor_Stop();
            }
            else
            {
                Motor_Right(1900, 1900);
                Clock_Delay1ms(1500);
                Motor_Stop();
            }
        }
        else if (right >= 12000)
        {
            Motor_Left(1900, 1900);
            Clock_Delay1ms(100);
        }
        else if (left >= 12000)
        {
            Motor_Right(1900, 1900);
            Clock_Delay1ms(100);
        }
        else // center <= colisionDetectCenter
        {
            Motor_Forward(1900, 1900);
        }
        Clock_Delay1ms(15);
    }
}
