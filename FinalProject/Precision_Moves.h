/*
 * Precision_Moves.h
 *
 *  Created on: Feb 14, 2023
 *      Author: John Tadrous
 */

#ifndef PRECISION_MOVES_H_
#define PRECISION_MOVES_H_


void Motor_Precision_Right(int16_t speed, int32_t desiredSteps);
void Motor_Precision_Left(int16_t speed, int32_t desiredSteps);
void Motor_Precision_CircleCCW(uint16_t speed, uint16_t radius);
void Motor_Forward_RPM(uint16_t leftRPM, uint16_t rightRPM, int32_t desiredLSteps, int32_t desiredRSteps);
void Motor_Precision_CircleCW(uint16_t speed, uint16_t radius);
void Motor_Precision_StarCCW(uint16_t speed, uint16_t sideLength);
static int32_t DistanceToSteps(double distance_cm);
static int32_t TurnAngleToSteps(double angle_deg);
#endif /* PRECISION_MOVES_H_ */
