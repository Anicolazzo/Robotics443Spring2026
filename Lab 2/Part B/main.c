#include <RobotLights.h>
#include "msp.h"
#include "Clock.h"

/**
 * main.c
 */
void main(void)
{
    uint8_t count=0;
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer
	Clock_Init48MHz();
	MvtLED_Init();
	while(1){
	    if (count==5){
	        Front_Lights_ON();
	        Back_Lights_ON();
	        Clock_Delay1ms(1000);
	        Front_Lights_OFF();
	        Back_Lights_OFF();
	        Clock_Delay1ms(1000);
	        count=0;
	    }
	    Front_Lights_ON();
	    Clock_Delay1ms(200);
	    Front_Lights_OFF();
	    Back_Lights_ON();
	    Clock_Delay1ms(200);
	    Back_Lights_OFF();
	    Clock_Delay1ms(200);
        count++;
	}
}
