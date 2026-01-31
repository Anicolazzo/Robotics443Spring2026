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

uint8_t Bump_Read(void)
{
    // write this as part of Lab 4
    // Read current state of 6 switches
    // Returns a 6-bit positive logic result (0 to 63)
    // bit 5 Bump5
    // bit 4 Bump4
    // bit 3 Bump3
    // bit 2 Bump2
    // bit 1 Bump1
    // bit 0 Bump0
    uint8_t data;

    data = (P4->IFG) & 0xED;
    data = ((data & 0x80) >> 2) | ((data & 0x40) >> 2) | ((data & 0x20) >> 2)
            | ((data & 0x08) >> 1) | ((data & 0x04) >> 1) | ((data & 0x01));
    return data;
}
// we do not care about critical section/race conditions
// triggered on touch, falling edge
void PORT4_IRQHandler(void)
{
    rTask(Bump_Read()); // Execute task from high-level software
    P4->IFG &= ~0xED; // ACK all
}

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

    if(bumpData <= 7)
    {
        Motor_Left(5000, 5000);
    }
    else
    {
        Motor_Right(5000, 5000);
    }
    Clock_Delay1ms(600);
    Motor_Stop();
    Motor_Forward(5000, 5000);
    Front_Lights_ON();
    // This runs WHEN a bump happens
    // bumpData is 0–63
    // Example: do nothing for now
}

