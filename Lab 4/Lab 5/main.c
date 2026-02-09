#include "msp.h"
#include "BumpInt.h"
#include "Reflectance.h"
#include "Motor.h"
#include "Clock.h"
#include "RobotLights.h"
/**
 * main.c
 */

typedef enum {
    LINE_LOST   = 0x00,
    LINE_RIGHT  = 0x01,
    LINE_LEFT   = 0x02,
    LINE_CENTER = 0x03,
    LINE_INIT   = 0x10
} LineState;

void main(void)
{
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer
    BumpInt_Init(BumpTask);
    Clock_Init48MHz();
    MvtLED_Init();
    Motor_Init();
    Reflectance_Init();
    LineState previousState = LINE_INIT;
    int i = 0;

    while (1)
    {
        LineState state = (LineState)Reflectance_Center(1000);

        switch (state)
        {
        case LINE_CENTER: // Center
            Motor_Forward(2500, 2500);
            previousState = LINE_CENTER;
            break;

        case LINE_LEFT:
            Motor_Left(2500, 2500);
            previousState = LINE_LEFT;
            //Clock_Delay1ms(200);
            break; // Left

        case LINE_RIGHT: // Rights
            Motor_Right(2500, 2500);
            previousState = LINE_RIGHT;
            //Clock_Delay1ms(300);
            break;

        case LINE_LOST: // off the line
            //Motor_Stop();

            // If we lost it coming from center, treat as end-of-line (per lab) :contentReference[oaicite:0]{index=0}
            if (previousState == LINE_CENTER)
            {
                Motor_Stop();
            }
            else if (previousState == LINE_LEFT) // we were correcting LEFT last, so keep sweeping LEFT until we reacquire
            {
                for (i = 0; i < 180; i++)
                {
                    Motor_Left(2500, 2500);
                    Clock_Delay1ms(5);

                    LineState c = (LineState)Reflectance_Center(1000);
                    if (c == LINE_CENTER || c == LINE_LEFT || c == LINE_RIGHT)
                    {
                        Motor_Stop();
                        break;
                    }
                }
            }
            else if (previousState == LINE_RIGHT) // we were correcting RIGHT last, so keep sweeping RIGHT until we reacquire
            {
                for (i = 0; i < 180; i++)
                {
                    Motor_Right(2500, 2500);
                    Clock_Delay1ms(5);

                    LineState c = (LineState)Reflectance_Center(1000);
                    if (c == LINE_CENTER || c == LINE_LEFT || c == LINE_RIGHT)
                    {
                        Motor_Stop();
                        break;
                    }
                }
            }
            else
            {
                Motor_Stop();
            }

            previousState = LINE_LOST;
            break;

        default:
            break;
        }

        Clock_Delay1ms(1);
    }
}
