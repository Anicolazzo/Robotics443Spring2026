/*
 * Simple Application Processor Example for BLE
 * Wireless controlled movement with obstacle notification
 * Author: John Tadrous
 * Organized and fixed
 */

#include <stdint.h>
#include <stdbool.h>
#include "msp.h"
#include "AP.h"
#include "CortexM.h"
#include "Clock.h"
#include "RobotLights.h"
#include "Motor.h"
#include "ADC14.h"

/* -------------------- BLE UUIDs -------------------- */
#define ROBOT_SERVICE_UUID        0xFFF0
#define MOVE_COMMAND_UUID         0xFFF1
#define SPEED_UUID                0xFFF2
#define LEFT_DISTANCE_UUID        0xFFF3
#define CENTER_DISTANCE_UUID      0xFFF4
#define RIGHT_DISTANCE_UUID       0xFFF5
#define OBJECT_AHEAD_UUID         0xFFF6

/* -------------------- Commands -------------------- */
#define MOVE_STOP                 0x00
#define MOVE_FORWARD              0x0F
#define MOVE_BACKWARD             0x0B
#define MOVE_LEFT                 0x0A
#define MOVE_RIGHT                0x0C

/* -------------------- Limits -------------------- */
#define MAX_SPEED                 0x2700

#define LEFT_BLOCK_9CM_ADC        11050
#define CENTER_BLOCK_9CM_ADC      11050
#define RIGHT_BLOCK_9CM_ADC       11050

#define LEFT_CLEAR_11CM_ADC       9420
#define CENTER_CLEAR_11CM_ADC     9420
#define RIGHT_CLEAR_11CM_ADC      9420

/* -------------------- Global BLE data -------------------- */
uint8_t moveCommand = MOVE_STOP;
uint8_t robotStatus = 0x00;      // 0x00 = clear, 0x01 = blocked
uint16_t robotSpeed = 5000;

uint32_t leftDistance = 0;
uint32_t centerDistance = 0;
uint32_t rightDistance = 0;

/* -------------------- Notification state -------------------- */
uint32_t usSinceLastObjectNotify = 1000;
bool resendBlockedForward = false;

/* -------------------- Helpers -------------------- */
static void clampSpeed(void)
{
    if (robotSpeed > MAX_SPEED)
    {
        robotSpeed = MAX_SPEED;
    }
}

static void updateDistances(void)
{
    ADC_In17_14_16(&rightDistance, &centerDistance, &leftDistance);
}

static bool obstacleWithin9cm(void)
{
    if ((leftDistance >= LEFT_BLOCK_9CM_ADC) ||
        (centerDistance >= CENTER_BLOCK_9CM_ADC) ||
        (rightDistance >= RIGHT_BLOCK_9CM_ADC))
    {
        return true;
    }
    return false;
}

static bool obstacleCleared11cm(void)
{
    if ((leftDistance < LEFT_CLEAR_11CM_ADC) &&
        (centerDistance < CENTER_CLEAR_11CM_ADC) &&
        (rightDistance < RIGHT_CLEAR_11CM_ADC))
    {
        return true;
    }
    return false;
}

static void stopRobot(void)
{
    Motor_Stop();
    moveCommand = MOVE_STOP;
}

static void sendObjectAheadNotification(uint8_t value)
{
    robotStatus = value;
    AP_SendNotification(0);              // first notify characteristic = CCCD 0
    usSinceLastObjectNotify = 0;
}

/* -------------------- Read callbacks -------------------- */
void readLeftDistance(void)
{
}

void readCenterDistance(void)
{
}

void readRightDistance(void)
{
}

/* -------------------- Write callbacks -------------------- */
void writeMoveCommand(void)
{
    clampSpeed();

    if (moveCommand == MOVE_STOP)
    {
        stopRobot();
    }
    else if (moveCommand == MOVE_FORWARD)
    {
        if (robotStatus == 0x00)
        {
            Motor_Forward(robotSpeed, robotSpeed);
        }
        else
        {
            stopRobot();
            resendBlockedForward = true;
        }
    }
    else if (moveCommand == MOVE_LEFT)
    {
        Motor_Left(robotSpeed, robotSpeed);
        Clock_Delay1ms(50);
        stopRobot();
    }
    else if (moveCommand == MOVE_RIGHT)
    {
        Motor_Right(robotSpeed, robotSpeed);
        Clock_Delay1ms(50);
        stopRobot();
    }
    else if (moveCommand == MOVE_BACKWARD)
    {
        Motor_Backward(robotSpeed, robotSpeed);
    }
    else
    {
        stopRobot();
    }
}

void writeRobotSpeed(void)
{
    clampSpeed();

    if (moveCommand == MOVE_FORWARD && robotStatus == 0x00)
    {
        Motor_Forward(robotSpeed, robotSpeed);
    }
    else if (moveCommand == MOVE_BACKWARD)
    {
        Motor_Backward(robotSpeed, robotSpeed);
    }
}

void notifyEnabled(void)
{
}

/* -------------------- Main -------------------- */
void main(void)
{
    volatile int r;

    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;
    Clock_Init48MHz();
    ADC0_InitSWTriggerCh17_14_16();
    MvtLED_Init();
    Motor_Init();

    r = AP_Init();

    moveCommand = MOVE_STOP;
    robotStatus = 0x00;
    robotSpeed = 5000;
    leftDistance = 0;
    centerDistance = 0;
    rightDistance = 0;
    resendBlockedForward = false;
    usSinceLastObjectNotify = 1000;

    AP_AddService(ROBOT_SERVICE_UUID);
    AP_AddCharacteristic(MOVE_COMMAND_UUID, 1, &moveCommand, 0x02, 0x08,
                         "moveCommand", 0, &writeMoveCommand);
    AP_AddCharacteristic(SPEED_UUID, 2, &robotSpeed, 0x02, 0x08,
                         "setSpeed", 0, &writeRobotSpeed);
    AP_AddCharacteristic(LEFT_DISTANCE_UUID, 4, &leftDistance, 0x01, 0x02,
                         "LeftDistance", &readLeftDistance, 0);
    AP_AddCharacteristic(CENTER_DISTANCE_UUID, 4, &centerDistance, 0x01, 0x02,
                         "CenterDistance", &readCenterDistance, 0);
    AP_AddCharacteristic(RIGHT_DISTANCE_UUID, 4, &rightDistance, 0x01, 0x02,
                         "RightDistance", &readRightDistance, 0);
    AP_AddNotifyCharacteristic(OBJECT_AHEAD_UUID, 1, &robotStatus,
                               "robotStatus", &notifyEnabled);
    AP_RegisterService();
    AP_StartAdvertisement();

    while (1)
    {
        AP_BackgroundProcess();

        updateDistances();
        Clock_Delay1us(100);
        usSinceLastObjectNotify += 100;

        if ((robotStatus == 0x00)  &&
            obstacleWithin9cm())
        {
            stopRobot();
            sendObjectAheadNotification(0x01);
        }

        if (resendBlockedForward &&
            (robotStatus == 0x01) &&
            (usSinceLastObjectNotify > 100))
        {
            sendObjectAheadNotification(0x01);
            resendBlockedForward = false;
        }

        if ((robotStatus == 0x01) &&
            (usSinceLastObjectNotify > 100) &&
            obstacleCleared11cm())
        {
            sendObjectAheadNotification(0x00);
        }
    }
}
