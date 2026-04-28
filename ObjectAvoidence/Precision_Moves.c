#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "msp.h"
#include "Clock.h"
#include "Motor.h"
#include "Tachometer.h"
#include "Precision_Moves.h"
#include "ADC14.h"

#define PI                     3.141592653589793
#define WHEEL_DIAMETER_CM      7.0
#define ROBOT_WIDTH_CM         14.5
#define STEPS_PER_REV          360.0
#define MAX_PWM                14999

#define TURN_SAMPLE_MS         20
#define RPM_SAMPLE_MS          60

#define TURN_KP                4
#define TURN_MIN_PWM           1000

#define STRAIGHT_PHASE_KP      4
#define STRAIGHT_PHASE_KI_DIV  10
#define STRAIGHT_PHASE_LIMIT   800

#define MASTER_START_PWM       2200
#define MASTER_MIN_PWM         1200
#define MASTER_KP              2
#define MASTER_KI_DIV          16
#define MASTER_INT_LIMIT       1800

#define SLAVE_START_PWM        2000
#define SLAVE_MIN_PWM          1450
#define SLAVE_KP               3
#define SLAVE_KI_DIV           12
#define SLAVE_INT_LIMIT        2000
#define SLAVE_PWM_BIAS         250

#define REF_STEPS              654    /* approx 40 cm in encoder steps         */
#define MIN_STARTUP_MS         15     /* never delay less than this at startup */
#define MIN_START_PWM          1400   /* floor for the scaled start PWM        */
#define SLAVE_ABS_MIN_PWM      800    /* absolute floor for inner-wheel PWM    */

/* Set to 1 if Motor_Forward_RPM stopped early due to obstacle.
   Read in main.c via: extern int Motor_Forward_Obstacle;          */
int Motor_Forward_Obstacle = 0;

/* Returns 1 if center sensor sees a wall. Takes max of two reads
   to lean toward detection while motors are running.              */
static int sensor_wall_detected(void){
    uint32_t l1,c1,r1,l2,c2,r2;
    ADC_In17_14_16(&r1,&c1,&l1);
    ADC_In17_14_16(&r2,&c2,&l2);
    uint32_t center = (c1 > c2) ? c1 : c2;
    return (center > 8900);
}


static int32_t DistanceToSteps(double distance_cm){
    double stepLength = (PI * WHEEL_DIAMETER_CM) / STEPS_PER_REV;
    return (int32_t)((distance_cm / stepLength) + 0.5);
}

static int32_t TurnAngleToSteps(double angle_deg){
    return (int32_t)(((ROBOT_WIDTH_CM / WHEEL_DIAMETER_CM) * angle_deg) + 0.5);
}

static void ClampPWM(int32_t *pwm){
    if(*pwm < 0) *pwm = 0;
    if(*pwm > MAX_PWM) *pwm = MAX_PWM;
}

static double ComputeDistScale(int32_t targetSteps){
    double s = (double)targetSteps / (double)REF_STEPS;
    if(s > 1.0) s = 1.0;
    if(s < 0.0) s = 0.0;
    return s;
}

static int32_t ScaledStartPWM(double distScale){
    return MIN_START_PWM + (int32_t)((MASTER_START_PWM - MIN_START_PWM) * distScale);
}

static uint32_t ScaledStartupDelay(double distScale){
    uint32_t d = (uint32_t)(80.0 * distScale);
    if(d < MIN_STARTUP_MS) d = MIN_STARTUP_MS;
    return d;
}

static int32_t ScaledSlaveMinPWM(double innerToOuterRatio){
    int32_t dynMin = (int32_t)(MASTER_MIN_PWM * innerToOuterRatio + 0.5);
    if(dynMin < SLAVE_ABS_MIN_PWM) dynMin = SLAVE_ABS_MIN_PWM;
    return dynMin;
}

static void ActiveBrake_Right(uint16_t leftPWM, uint16_t rightPWM){
    Motor_Left(leftPWM, rightPWM);
    Clock_Delay1ms(20);
    Motor_Stop();
}

static void ActiveBrake_Left(uint16_t leftPWM, uint16_t rightPWM){
    Motor_Right(leftPWM, rightPWM);
    Clock_Delay1ms(20);
    Motor_Stop();
}


void Motor_Precision_Right(int16_t speed, int32_t desiredSteps){
    int32_t startLeft, startRight, currentLeft, currentRight;
    int32_t movedLeft, movedRight, phaseError, avgMoved;
    int32_t leftPWM, rightPWM;

    if(desiredSteps <= 0) return;
    if(speed < 0) speed = 0;
    if(speed > MAX_PWM) speed = MAX_PWM;
    if(speed < TURN_MIN_PWM) speed = TURN_MIN_PWM;

    Tachometer_Get_Steps(&startLeft, &startRight);

    leftPWM = speed;
    rightPWM = speed;
    Motor_Right((uint16_t)leftPWM, (uint16_t)rightPWM);

    while(1){
        Clock_Delay1ms(TURN_SAMPLE_MS);
        Tachometer_Get_Steps(&currentLeft, &currentRight);

        movedLeft  = currentLeft  - startLeft;
        movedRight = currentRight - startRight;
        if(movedLeft  < 0) movedLeft  = -movedLeft;
        if(movedRight < 0) movedRight = -movedRight;

        avgMoved = (movedLeft + movedRight) / 2;
        if(avgMoved >= desiredSteps) break;

        phaseError = movedLeft - movedRight;
        leftPWM  = speed - (TURN_KP * phaseError);
        rightPWM = speed + (TURN_KP * phaseError);

        if(leftPWM  < TURN_MIN_PWM) leftPWM  = TURN_MIN_PWM;
        if(rightPWM < TURN_MIN_PWM) rightPWM = TURN_MIN_PWM;

        ClampPWM(&leftPWM);
        ClampPWM(&rightPWM);

        Motor_Right((uint16_t)leftPWM, (uint16_t)rightPWM);
    }

    ActiveBrake_Right((uint16_t)speed, (uint16_t)speed);
}

void Motor_Precision_Left(int16_t speed, int32_t desiredSteps){
    int32_t startLeft, startRight, currentLeft, currentRight;
    int32_t movedLeft, movedRight, phaseError, avgMoved;
    int32_t leftPWM, rightPWM;

    if(desiredSteps <= 0) return;
    if(speed < 0) speed = 0;
    if(speed > MAX_PWM) speed = MAX_PWM;
    if(speed < TURN_MIN_PWM) speed = TURN_MIN_PWM;

    Tachometer_Get_Steps(&startLeft, &startRight);

    leftPWM = speed;
    rightPWM = speed;
    Motor_Left((uint16_t)leftPWM, (uint16_t)rightPWM);

    while(1){
        Clock_Delay1ms(TURN_SAMPLE_MS);
        Tachometer_Get_Steps(&currentLeft, &currentRight);

        movedLeft  = currentLeft  - startLeft;
        movedRight = currentRight - startRight;
        if(movedLeft  < 0) movedLeft  = -movedLeft;
        if(movedRight < 0) movedRight = -movedRight;

        avgMoved = (movedLeft + movedRight) / 2;
        if(avgMoved >= desiredSteps) break;

        phaseError = movedLeft - movedRight;
        leftPWM  = speed - (TURN_KP * phaseError);
        rightPWM = speed + (TURN_KP * phaseError);

        if(leftPWM  < TURN_MIN_PWM) leftPWM  = TURN_MIN_PWM;
        if(rightPWM < TURN_MIN_PWM) rightPWM = TURN_MIN_PWM;

        ClampPWM(&leftPWM);
        ClampPWM(&rightPWM);

        Motor_Left((uint16_t)leftPWM, (uint16_t)rightPWM);
    }

    ActiveBrake_Left((uint16_t)speed, (uint16_t)speed);
}


void Motor_Forward_RPM(uint16_t leftRPM, uint16_t rightRPM,
                       int32_t desiredLSteps, int32_t desiredRSteps){
    int32_t startLSteps, startRSteps;
    int32_t prevLSteps, prevRSteps, curLSteps, curRSteps;
    uint32_t prevLTime, prevRTime, curLTime, curRTime;
    int32_t movedL, movedR, avgMoved, avgDesiredSteps;
    int32_t deltaLSteps, deltaRSteps;
    uint32_t deltaLTime, deltaRTime;
    uint16_t actualLRPM, actualRRPM, avgActualRPM, avgTargetRPM;
    int32_t errAvg, integAvg = 0;
    int32_t phaseError, phaseInteg = 0, phaseCorrection;
    int32_t basePWM, leftPWM, rightPWM;
    int iterCount = 0;

    Motor_Forward_Obstacle = 0;

    if(desiredLSteps <= 0 || desiredRSteps <= 0) return;
    if(leftRPM == 0 || rightRPM == 0) return;

    avgTargetRPM    = (leftRPM + rightRPM) / 2;
    avgDesiredSteps = (desiredLSteps + desiredRSteps) / 2;

    double   distScale  = ComputeDistScale(avgDesiredSteps);
    int32_t  startPWM   = ScaledStartPWM(distScale);
    uint32_t startDelay = ScaledStartupDelay(distScale);

    basePWM  = startPWM;
    leftPWM  = basePWM;
    rightPWM = basePWM;

    if(leftPWM  < MASTER_MIN_PWM) leftPWM  = MASTER_MIN_PWM;
    if(rightPWM < MASTER_MIN_PWM) rightPWM = MASTER_MIN_PWM;

    Tachometer_Get_SpaceTime(&startLSteps, &startRSteps, &prevLTime, &prevRTime);
    prevLSteps = startLSteps;
    prevRSteps = startRSteps;

    Motor_Forward((uint16_t)leftPWM, (uint16_t)rightPWM);
    Clock_Delay1ms(startDelay);

    while(1){
        Clock_Delay1ms(RPM_SAMPLE_MS);

        /* Poll sensor every 3 iterations (~180ms). If wall detected,
           stop immediately and set flag for main.c to handle.        */
        iterCount++;
        if(iterCount >= 3){
            iterCount = 0;
            if(sensor_wall_detected()){
                Motor_Stop();
                Motor_Forward_Obstacle = 1;
                return;
            }
        }

        Tachometer_Get_SpaceTime(&curLSteps, &curRSteps, &curLTime, &curRTime);

        movedL = curLSteps - startLSteps;
        movedR = curRSteps - startRSteps;
        if(movedL < 0) movedL = -movedL;
        if(movedR < 0) movedR = -movedR;

        avgMoved = (movedL + movedR) / 2;
        if(avgMoved >= avgDesiredSteps) break;

        deltaLSteps = curLSteps - prevLSteps;
        deltaRSteps = curRSteps - prevRSteps;
        if(deltaLSteps < 0) deltaLSteps = -deltaLSteps;
        if(deltaRSteps < 0) deltaRSteps = -deltaRSteps;

        deltaLTime = curLTime - prevLTime;
        deltaRTime = curRTime - prevRTime;

        actualLRPM = (deltaLSteps == 0 || deltaLTime == 0) ? 0
            : (uint16_t)(((uint64_t)deltaLSteps * 20000000ULL) / deltaLTime);

        actualRRPM = (deltaRSteps == 0 || deltaRTime == 0) ? 0
            : (uint16_t)(((uint64_t)deltaRSteps * 20000000ULL) / deltaRTime);

        if(actualLRPM == 0 || actualRRPM == 0){
            prevLSteps = curLSteps;
            prevRSteps = curRSteps;
            prevLTime  = curLTime;
            prevRTime  = curRTime;
            continue;
        }

        avgActualRPM = (actualLRPM + actualRRPM) / 2;
        errAvg = (int32_t)avgTargetRPM - (int32_t)avgActualRPM;

        integAvg += errAvg;
        if(integAvg >  MASTER_INT_LIMIT) integAvg =  MASTER_INT_LIMIT;
        if(integAvg < -MASTER_INT_LIMIT) integAvg = -MASTER_INT_LIMIT;

        basePWM += (MASTER_KP * errAvg) + (integAvg / MASTER_KI_DIV);
        if(basePWM < MASTER_MIN_PWM) basePWM = MASTER_MIN_PWM;
        if(basePWM > MAX_PWM)        basePWM = MAX_PWM;

        phaseError  = movedL - movedR;
        phaseInteg += phaseError;
        if(phaseInteg >  MASTER_INT_LIMIT) phaseInteg =  MASTER_INT_LIMIT;
        if(phaseInteg < -MASTER_INT_LIMIT) phaseInteg = -MASTER_INT_LIMIT;

        phaseCorrection = (STRAIGHT_PHASE_KP * phaseError)
                        + (phaseInteg / STRAIGHT_PHASE_KI_DIV);

        if(phaseCorrection >  STRAIGHT_PHASE_LIMIT) phaseCorrection =  STRAIGHT_PHASE_LIMIT;
        if(phaseCorrection < -STRAIGHT_PHASE_LIMIT) phaseCorrection = -STRAIGHT_PHASE_LIMIT;

        leftPWM  = basePWM - phaseCorrection;
        rightPWM = basePWM + phaseCorrection;

        if(leftPWM  < MASTER_MIN_PWM) leftPWM  = MASTER_MIN_PWM;
        if(rightPWM < MASTER_MIN_PWM) rightPWM = MASTER_MIN_PWM;
        if(leftPWM  > MAX_PWM)        leftPWM  = MAX_PWM;
        if(rightPWM > MAX_PWM)        rightPWM = MAX_PWM;

        Motor_Forward((uint16_t)leftPWM, (uint16_t)rightPWM);

        prevLSteps = curLSteps;
        prevRSteps = curRSteps;
        prevLTime  = curLTime;
        prevRTime  = curRTime;
    }

    Motor_Stop();
}


static void Motor_Forward_CircleCCW(uint16_t rightRPM, double innerToOuterRatio,
                                    int32_t centerTargetSteps){
    int32_t startLSteps, startRSteps;
    int32_t prevLSteps, prevRSteps, curLSteps, curRSteps;
    uint32_t prevLTime, prevRTime, curLTime, curRTime;
    int32_t movedL, movedR, centerMovedSteps;
    int32_t deltaLSteps, deltaRSteps;
    uint32_t deltaLTime, deltaRTime;
    uint16_t actualLRPM, actualRRPM, targetLRPM;
    int32_t errMaster, errSlave;
    int32_t integMaster = 0, integSlave = 0;
    int32_t leftPWM, rightPWM, baseLeftPWM;

    if(centerTargetSteps <= 0 || rightRPM == 0) return;
    if(innerToOuterRatio <= 0.0) return;
    if(innerToOuterRatio >= 1.0) innerToOuterRatio = 0.95;

    int32_t slaveMinPWM = ScaledSlaveMinPWM(innerToOuterRatio);

    double   distScale   = ComputeDistScale(centerTargetSteps);
    int32_t  mStartPWM   = ScaledStartPWM(distScale);
    int32_t  sStartPWM   = (int32_t)(mStartPWM * innerToOuterRatio) + SLAVE_PWM_BIAS;
    uint32_t startDelay  = ScaledStartupDelay(distScale);

    rightPWM = mStartPWM;
    leftPWM  = sStartPWM;

    if(rightPWM < MASTER_MIN_PWM) rightPWM = MASTER_MIN_PWM;
    if(leftPWM  < slaveMinPWM)    leftPWM  = slaveMinPWM;

    Tachometer_Get_SpaceTime(&startLSteps, &startRSteps, &prevLTime, &prevRTime);
    prevLSteps = startLSteps;
    prevRSteps = startRSteps;

    Motor_Forward((uint16_t)leftPWM, (uint16_t)rightPWM);
    Clock_Delay1ms(startDelay);

    while(1){
        Clock_Delay1ms(RPM_SAMPLE_MS);

        Tachometer_Get_SpaceTime(&curLSteps, &curRSteps, &curLTime, &curRTime);

        movedL = curLSteps - startLSteps;
        movedR = curRSteps - startRSteps;
        if(movedL < 0) movedL = -movedL;
        if(movedR < 0) movedR = -movedR;

        centerMovedSteps = (movedL + movedR) / 2;
        if(centerMovedSteps >= centerTargetSteps) break;

        deltaLSteps = curLSteps - prevLSteps;
        deltaRSteps = curRSteps - prevRSteps;
        if(deltaLSteps < 0) deltaLSteps = -deltaLSteps;
        if(deltaRSteps < 0) deltaRSteps = -deltaRSteps;

        deltaLTime = curLTime - prevLTime;
        deltaRTime = curRTime - prevRTime;

        actualLRPM = (deltaLSteps == 0 || deltaLTime == 0) ? 0
            : (uint16_t)(((uint64_t)deltaLSteps * 20000000ULL) / deltaLTime);

        actualRRPM = (deltaRSteps == 0 || deltaRTime == 0) ? 0
            : (uint16_t)(((uint64_t)deltaRSteps * 20000000ULL) / deltaRTime);

        if(actualLRPM == 0 || actualRRPM == 0){
            prevLSteps = curLSteps;
            prevRSteps = curRSteps;
            prevLTime  = curLTime;
            prevRTime  = curRTime;
            continue;
        }

        errMaster    = (int32_t)rightRPM - (int32_t)actualRRPM;
        integMaster += errMaster;
        if(integMaster >  MASTER_INT_LIMIT) integMaster =  MASTER_INT_LIMIT;
        if(integMaster < -MASTER_INT_LIMIT) integMaster = -MASTER_INT_LIMIT;

        rightPWM += (MASTER_KP * errMaster) + (integMaster / MASTER_KI_DIV);
        if(rightPWM < MASTER_MIN_PWM) rightPWM = MASTER_MIN_PWM;
        if(rightPWM > MAX_PWM)        rightPWM = MAX_PWM;

        targetLRPM = (uint16_t)((actualRRPM * innerToOuterRatio) + 0.5);

        baseLeftPWM = (int32_t)(rightPWM * innerToOuterRatio) + SLAVE_PWM_BIAS;
        if(baseLeftPWM < slaveMinPWM) baseLeftPWM = slaveMinPWM;

        errSlave    = (int32_t)targetLRPM - (int32_t)actualLRPM;
        integSlave += errSlave;
        if(integSlave >  SLAVE_INT_LIMIT) integSlave =  SLAVE_INT_LIMIT;
        if(integSlave < -SLAVE_INT_LIMIT) integSlave = -SLAVE_INT_LIMIT;

        leftPWM = baseLeftPWM + (SLAVE_KP * errSlave) + (integSlave / SLAVE_KI_DIV);
        if(leftPWM < slaveMinPWM) leftPWM = slaveMinPWM;
        if(leftPWM > MAX_PWM)     leftPWM = MAX_PWM;

        Motor_Forward((uint16_t)leftPWM, (uint16_t)rightPWM);

        prevLSteps = curLSteps;
        prevRSteps = curRSteps;
        prevLTime  = curLTime;
        prevRTime  = curRTime;
    }

    Motor_Stop();
}


static void Motor_Forward_CircleCW(uint16_t leftRPM, double innerToOuterRatio,
                                   int32_t centerTargetSteps){
    int32_t startLSteps, startRSteps;
    int32_t prevLSteps, prevRSteps, curLSteps, curRSteps;
    uint32_t prevLTime, prevRTime, curLTime, curRTime;
    int32_t movedL, movedR, centerMovedSteps;
    int32_t deltaLSteps, deltaRSteps;
    uint32_t deltaLTime, deltaRTime;
    uint16_t actualLRPM, actualRRPM, targetRRPM;
    int32_t errMaster, errSlave;
    int32_t integMaster = 0, integSlave = 0;
    int32_t leftPWM, rightPWM, baseRightPWM;

    if(centerTargetSteps <= 0 || leftRPM == 0) return;
    if(innerToOuterRatio <= 0.0) return;
    if(innerToOuterRatio >= 1.0) innerToOuterRatio = 0.95;

    int32_t slaveMinPWM = ScaledSlaveMinPWM(innerToOuterRatio);

    double   distScale   = ComputeDistScale(centerTargetSteps);
    int32_t  mStartPWM   = ScaledStartPWM(distScale);
    int32_t  sStartPWM   = (int32_t)(mStartPWM * innerToOuterRatio) + SLAVE_PWM_BIAS;
    uint32_t startDelay  = ScaledStartupDelay(distScale);

    leftPWM  = mStartPWM;
    rightPWM = sStartPWM;

    if(leftPWM  < MASTER_MIN_PWM) leftPWM  = MASTER_MIN_PWM;
    if(rightPWM < slaveMinPWM)    rightPWM = slaveMinPWM;

    Tachometer_Get_SpaceTime(&startLSteps, &startRSteps, &prevLTime, &prevRTime);
    prevLSteps = startLSteps;
    prevRSteps = startRSteps;

    Motor_Forward((uint16_t)leftPWM, (uint16_t)rightPWM);
    Clock_Delay1ms(startDelay);

    while(1){
        Clock_Delay1ms(RPM_SAMPLE_MS);

        Tachometer_Get_SpaceTime(&curLSteps, &curRSteps, &curLTime, &curRTime);

        movedL = curLSteps - startLSteps;
        movedR = curRSteps - startRSteps;
        if(movedL < 0) movedL = -movedL;
        if(movedR < 0) movedR = -movedR;

        centerMovedSteps = (movedL + movedR) / 2;
        if(centerMovedSteps >= centerTargetSteps) break;

        deltaLSteps = curLSteps - prevLSteps;
        deltaRSteps = curRSteps - prevRSteps;
        if(deltaLSteps < 0) deltaLSteps = -deltaLSteps;
        if(deltaRSteps < 0) deltaRSteps = -deltaRSteps;

        deltaLTime = curLTime - prevLTime;
        deltaRTime = curRTime - prevRTime;

        actualLRPM = (deltaLSteps == 0 || deltaLTime == 0) ? 0
            : (uint16_t)(((uint64_t)deltaLSteps * 20000000ULL) / deltaLTime);

        actualRRPM = (deltaRSteps == 0 || deltaRTime == 0) ? 0
            : (uint16_t)(((uint64_t)deltaRSteps * 20000000ULL) / deltaRTime);

        if(actualLRPM == 0 || actualRRPM == 0){
            prevLSteps = curLSteps;
            prevRSteps = curRSteps;
            prevLTime  = curLTime;
            prevRTime  = curRTime;
            continue;
        }

        errMaster    = (int32_t)leftRPM - (int32_t)actualLRPM;
        integMaster += errMaster;
        if(integMaster >  MASTER_INT_LIMIT) integMaster =  MASTER_INT_LIMIT;
        if(integMaster < -MASTER_INT_LIMIT) integMaster = -MASTER_INT_LIMIT;

        leftPWM += (MASTER_KP * errMaster) + (integMaster / MASTER_KI_DIV);
        if(leftPWM < MASTER_MIN_PWM) leftPWM = MASTER_MIN_PWM;
        if(leftPWM > MAX_PWM)        leftPWM = MAX_PWM;

        targetRRPM = (uint16_t)((actualLRPM * innerToOuterRatio) + 0.5);

        baseRightPWM = (int32_t)(leftPWM * innerToOuterRatio) + SLAVE_PWM_BIAS;
        if(baseRightPWM < slaveMinPWM) baseRightPWM = slaveMinPWM;

        errSlave    = (int32_t)targetRRPM - (int32_t)actualRRPM;
        integSlave += errSlave;
        if(integSlave >  SLAVE_INT_LIMIT) integSlave =  SLAVE_INT_LIMIT;
        if(integSlave < -SLAVE_INT_LIMIT) integSlave = -SLAVE_INT_LIMIT;

        rightPWM = baseRightPWM + (SLAVE_KP * errSlave) + (integSlave / SLAVE_KI_DIV);
        if(rightPWM < slaveMinPWM) rightPWM = slaveMinPWM;
        if(rightPWM > MAX_PWM)     rightPWM = MAX_PWM;

        Motor_Forward((uint16_t)leftPWM, (uint16_t)rightPWM);

        prevLSteps = curLSteps;
        prevRSteps = curRSteps;
        prevLTime  = curLTime;
        prevRTime  = curRTime;
    }

    Motor_Stop();
}

void Motor_Precision_CircleCCW(uint16_t speed, uint16_t radius){
    double Ri, Ro, ratio, centerLength;
    uint16_t outerRPM;
    int32_t centerSteps;

    if(radius < 8 || speed < 350) return;

    Ri = ((double)radius) - (ROBOT_WIDTH_CM / 2.0);
    Ro = ((double)radius) + (ROBOT_WIDTH_CM / 2.0);
    if(Ri <= 0.0) return;

    ratio        = Ri / Ro;
    centerLength = 2.0 * PI * (double)radius;
    centerSteps  = DistanceToSteps(centerLength);
    outerRPM     = (uint16_t)((speed * Ro / (double)radius) + 0.5);

    Motor_Forward_CircleCCW(outerRPM, ratio, centerSteps);
}

void Motor_Precision_CircleCW(uint16_t speed, uint16_t radius){
    double Ri, Ro, ratio, centerLength;
    uint16_t outerRPM;
    int32_t centerSteps;

    if(radius < 8 || speed < 350) return;

    Ri = ((double)radius) - (ROBOT_WIDTH_CM / 2.0);
    Ro = ((double)radius) + (ROBOT_WIDTH_CM / 2.0);
    if(Ri <= 0.0) return;

    ratio        = Ri / Ro;
    centerLength = 2.0 * PI * (double)radius;
    centerSteps  = DistanceToSteps(centerLength);
    outerRPM     = (uint16_t)((speed * Ro / (double)radius) + 0.5);

    Motor_Forward_CircleCW(outerRPM, ratio, centerSteps);
}

void Motor_Precision_StarCCW(uint16_t speed, uint16_t sideLength){
    int i;
    int32_t lineSteps, turn108Steps, turn36Steps;
    uint16_t turnPWM36;
    uint16_t turnPWM108;

    if(sideLength < 20 || speed < 400) return;

    lineSteps    = DistanceToSteps((double)sideLength);
    turn108Steps = TurnAngleToSteps(72.0);
    turn36Steps  = TurnAngleToSteps(144.0);

    turnPWM36  = speed + 900;
    turnPWM108 = speed + 700;

    if(turnPWM36  < 1800) turnPWM36  = 1800;
    if(turnPWM108 < 1700) turnPWM108 = 1700;

    if(turnPWM36  > 3200) turnPWM36  = 3200;
    if(turnPWM108 > 3000) turnPWM108 = 3000;

    for(i = 0; i < 5; i++){
        Motor_Forward_RPM(speed, speed, lineSteps, lineSteps);
        Clock_Delay1ms(50);

        Motor_Precision_Left(turnPWM36, turn36Steps);
        Clock_Delay1ms(40);

        Motor_Forward_RPM(speed, speed, lineSteps, lineSteps);
        Clock_Delay1ms(50);

        Motor_Precision_Right(turnPWM108, turn108Steps);
        Clock_Delay1ms(40);
    }
}