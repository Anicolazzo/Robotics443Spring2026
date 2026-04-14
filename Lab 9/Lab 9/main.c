#include "msp.h"
#include "Clock.h"
#include "Motor.h"
#include "Tachometer.h"
#include "Precision_Moves.h"
#include "RobotLights.h"
#include "GPIO.h"
#include <stdbool.h>
#include <stdint.h>

volatile bool gStopMotion = false;

static void initSwitches(void){
    P1->SEL0 &= ~0x12;
    P1->SEL1 &= ~0x12;
    P1->DIR  &= ~0x12;
    P1->REN  |=  0x12;
    P1->OUT  |=  0x12;
}

static bool SW1_Pressed(void){
    return (P1->IN & 0x02) == 0;
}

static bool SW2_Pressed(void){
    return (P1->IN & 0x10) == 0;
}

int main(void){
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;
    Clock_Init48MHz();
    Motor_Init();
    Tachometer_Init();
    MvtLED_Init();
    GPIO_Init();
    initSwitches();

    Motor_Stop();

    while(1){
        bool sw1 = SW1_Pressed();
        bool sw2 = SW2_Pressed();

        if(sw1 || sw2){
            // debounce first detection
            Clock_Delay1ms(20);

            sw1 = SW1_Pressed();
            sw2 = SW2_Pressed();

            // if only one button is down, wait a little longer
            // to see if the other button gets pressed too
            if(sw1 ^ sw2){
                Clock_Delay1ms(150);   // increase this if needed

                sw1 = SW1_Pressed();
                sw2 = SW2_Pressed();
            }

            if(sw1 && sw2){
                while(SW1_Pressed() || SW2_Pressed()){}
                Clock_Delay1ms(20);
                Motor_Precision_StarCCW(1000, 40);
            }else if(sw1){
                while(SW1_Pressed()){}
                Clock_Delay1ms(20);
                Motor_Precision_CircleCCW(1000, 40);
            }else if(sw2){
                while(SW2_Pressed()){}
                Clock_Delay1ms(20);
                Motor_Precision_CircleCW(1000, 40);
            }
        }

        Clock_Delay1ms(10);
    }
}
