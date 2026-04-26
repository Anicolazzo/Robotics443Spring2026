// BumpInt.c
// Runs on MSP432, interrupt version
// Provide low-level functions that interface bump switches on the robot.
// Daniel Valvano and Jonathan Valvano
// July 11, 2019

// Edited by J. Tadrous 06/02/2022

#include <stdint.h>
#include <stdio.h>
#include "msp.h"
#include "CortexM.h" // Global Interrupt Control
#include "Clock.h"
#include "Motor.h"
#include "RobotLights.h"

void (*rTask)(uint8_t); // Globalizing input function

void BumpInt_Init(void (*task)(uint8_t))
{
    rTask = task;

    DisableInterrupts();
    P4->SEL0 &= ~0xED;   // 1110 1101
    P4->SEL1 &= ~0xED;
    P4->DIR &= ~0xED;
    P4->REN |= 0xED;
    P4->OUT |= 0xED;
    P4->IES |= 0xED;
    P4->IFG &= ~0xED;
    P4->IE |= 0xED;
    NVIC->IP[9] = (NVIC->IP[9] & 0x00FFFFFF) | 0x40000000;
    NVIC->ISER[1] = 1 << 6;

    EnableInterrupts();

}

uint8_t Bump_Read(void) {
    Clock_Delay1ms(100);
    uint8_t raw = ~(P4->IN); // Invert so 1 = Pressed
    // Combine the bits into a 6-bit result (Bits 7,6,5,3,2,0)
    uint8_t result = 0;
    if(raw & 0x01) result |= 0x01; // Bump 0
    if(raw & 0x04) result |= 0x02; // Bump 1
    if(raw & 0x08) result |= 0x04; // Bump 2
    if(raw & 0x20) result |= 0x08; // Bump 3
    if(raw & 0x40) result |= 0x10; // Bump 4
    if(raw & 0x80) result |= 0x20; // Bump 5
    return result;
}
// we do not care about critical section/race conditions
// triggered on touch, falling edge
void PORT4_IRQHandler(void)
{
    rTask(Bump_Read()); // Execute task from high-level software

    P4->IFG &= ~0xED; // ACK all
}

//void BumpTask(uint8_t bumpData)
//{
//    printf("BumpData from task = %X\n", bumpData);
//    Motor_Stop();
//    Back_Lights_ON();
//    Motor_Backward(5000, 5000);
//    //Motor reverse
//    Clock_Delay1ms(400);
//    Back_Lights_OFF();
//    Motor_Stop();
//
//    if(bumpData <= 7)
//    {
//        Motor_Left(5000, 5000);
//    }
//    else
//    {
//        Motor_Right(5000, 5000);
//    }
//    Clock_Delay1ms(600);
//    Motor_Stop();
//    Motor_Forward(5000, 5000);
//    Front_Lights_ON();
//    // This runs WHEN a bump happens
//    // bumpData is 0–63
//    // Example: do nothing for now
//}
//
void BumpTask(uint8_t bumpData)
{
    printf("BumpData from task = %X\n", bumpData);
    Motor_Stop();


    Back_Lights_ON();
    Motor_Backward(5000, 5000);
    //Motor reverse
    Clock_Delay1ms(400);
    Back_Lights_OFF();
    Motor_Stop();
    printf("BumpData from PIN = %X\n", bumpData);
    printf("State 1 %X\n", bumpData & 0x07);
    printf("State 2 %X\n", bumpData & 0x38);
    printf("State shifted %X\n", (bumpData & 0x38) >> 3);
    // Head-on collision (front bumpers only)
    if (((bumpData & 0x04) == 0x04) && ((bumpData & 0x08) == 0x08))
    {
        Motor_Left(5000, 5000);     // deterministic direction
        Clock_Delay1ms(900);        // your 180° turn time
    }
    // Otherwise: your original left/right behavior
    else if (bumpData <= 7)
    {
        Motor_Left(5000, 5000);
        Clock_Delay1ms(600);
    }
    else
    {
        Motor_Right(5000, 5000);
        Clock_Delay1ms(600);
    }

    Motor_Stop();
    Motor_Forward(5000, 5000);
    Front_Lights_ON();
    // This runs WHEN a bump happens
    // bumpData is 0–63
    // Example: do nothing for now
}
