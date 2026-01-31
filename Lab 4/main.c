#include "msp.h"
#include "BumpInt.h"

/**
 * main.c
 */
void main(void)
{
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer
	BumpInt_Init(BumpTask);
	Clock_Init48MHz();
	MvtLED_Init();
	Motor_Init();
	while(1)
	{

	}
}
