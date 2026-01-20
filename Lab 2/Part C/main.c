#include "msp.h"
#include "Clock.h"
#include "Reflectance.h"
#include <stdio.h>
/* This project initializes the line sensor pins, activates the line sensor
 * and reads the pins to verify the status of the line sensor
 * Author: John Tadrous
 * Date: 05/24/2022
*/

uint8_t data;
uint32_t Time;


/**
 * main.c
 */
void main(void)
{
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer
	Clock_Init48MHz();
	Reflectance_Init();
	Time = 1000;
	while (1){
	    data = Reflectance_Read(Time);
	    printf("Data status 0x%X\n", data);
	    Clock_Delay1ms(500);
	}
}

