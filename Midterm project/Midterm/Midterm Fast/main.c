#include "msp.h"
#include "Motor.h"
#include "Clock.h"
#include "RobotLights.h"
#include "ADC14.h"

// ------------------------------------------------------------
// IR sensor thresholds  (higher ADC value = closer object)
// ------------------------------------------------------------
#define ADC_6_INCHES   7500 // actual 6 7500
#define ADC_3_INCHES   11400
#define ADC_4_INCHES   10300
#define ADC_8_INCHES   6100
#define ADC_5_INCHES   8900
#define GAP_THRESHOLD  5000   // below this = no wall / open gap

#define MS_PER_INCH    45     // tune for your surface/speed
#define CRUISE_DUTY    5000
#define TURN_DUTY      2200
#define NUDGE_DUTY     2200

// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------
static void motorStopHard(void)
{
    Motor_Backward(10000, 10000);
    Clock_Delay1ms(70);
    Motor_Stop();
    Clock_Delay1ms(30);
}

static void motorStraightLine(void)
{
    Motor_Forward(CRUISE_DUTY, CRUISE_DUTY);
}

// ADC_In17_14_16: ch17=right, ch14=center, ch16=left
static void readSensors(uint32_t *left, uint32_t *right, uint32_t *center)
{
    uint32_t l = 0, r = 0, c = 0;
    uint32_t tl, tr, tc;
    int i;
    for (i = 0; i < 3; i++)
    {
        ADC_In17_14_16(&tr, &tc, &tl);
        r += tr;
        c += tc;
        l += tl;
        Clock_Delay1ms(10);
    }
    *left = l / 3;
    *right = r / 3;
    *center = c / 3;
}

static void turnRight90(void)
{
    Motor_Stop();
    Clock_Delay1ms(50);
    Motor_Right(TURN_DUTY, TURN_DUTY);
    Clock_Delay1ms(1200);
    Motor_Stop();
    Clock_Delay1ms(100);
}

static void turnLeft90(void)
{
    Motor_Stop();
    Clock_Delay1ms(50);
    Motor_Left(TURN_DUTY, TURN_DUTY);
    Clock_Delay1ms(1250);
    Motor_Stop();
    Clock_Delay1ms(100);
}

// ------------------------------------------------------------
// Main
// ------------------------------------------------------------
void main(void)
{
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;
    Clock_Init48MHz();
    MvtLED_Init();
    Motor_Init();
    ADC0_InitSWTriggerCh17_14_16();

    uint32_t left, right, center;
    Clock_Delay1ms(1000);
    motorStraightLine();
    while (1)
    {
        readSensors(&left, &right, &center);

        int wallFront = (center > ADC_5_INCHES);
        int wallFrontFar = (center < ADC_8_INCHES);
        int gapRight = (right < GAP_THRESHOLD);
        int RightToClose = (right > ADC_4_INCHES);
        int RightToFar = (right < ADC_6_INCHES);
        int LeftToClose = (left > ADC_4_INCHES);

        if ((wallFront && gapRight))
        {
            motorStopHard();
            turnRight90();
            if (wallFrontFar)
            {
                Motor_Forward(8000, 8000);
            }
            else
                motorStraightLine();
        }
        else if (wallFront && !gapRight)
        {
            motorStopHard();
            turnLeft90();
            if (wallFrontFar)
            {
                Motor_Forward(8000, 8000);
            }
            else
                motorStraightLine();
        }
        else if (gapRight)
        {
            motorStraightLine();
            int i;
            for (i = 0; i < 8; i++)
            {

                readSensors(&left, &right, &center);
                wallFront = (center > ADC_5_INCHES);
                RightToClose = (right > ADC_4_INCHES);
                LeftToClose = (left > ADC_4_INCHES);
                if (LeftToClose)
                {
                    Motor_Right(5000, 5000);
                    Clock_Delay1ms(10);
                    motorStraightLine();
                }
                else if (RightToClose)
                {
                    Motor_Left(5000, 5000);
                    Clock_Delay1ms(10);
                    if (wallFrontFar)
                    {
                        Motor_Forward(8000, 8000);
                    }
                    else
                        motorStraightLine();
                }
                if (wallFront)
                {
                    Motor_Stop();
                    break;
                }
                Clock_Delay1ms(MS_PER_INCH);
            }
            readSensors(&left, &right, &center);
            if (right < GAP_THRESHOLD)
            {
                motorStopHard();
                turnRight90();
                motorStraightLine();
                Clock_Delay1ms(60);
            }
            else
            {
                Motor_Left(5000, 5000);
                Clock_Delay1ms(10);
                if (wallFrontFar)
                {
                    Motor_Forward(8000, 8000);
                }
                else
                    motorStraightLine();
            }
        }
        else if (RightToClose)
        {
            Motor_Left(5000, 5000);
            Clock_Delay1ms(10);
            if (wallFrontFar)
            {
                Motor_Forward(8000, 8000);
            }
            else
                motorStraightLine();
        }
        else if (RightToFar || LeftToClose)
        {
            Motor_Right(5000, 5000);
            Clock_Delay1ms(10);
            if (wallFrontFar)
            {
                Motor_Forward(8000, 8000);
            }
            else
                motorStraightLine();
        }
        else
        {
            if (wallFrontFar)
            {
                Motor_Forward(8000, 8000);
            }
            else
                motorStraightLine();
        }
        Clock_Delay1ms(5);
    }
}
