#include "msp.h"
#include "Clock.h"
#include "ADC14.h"
#include <stdio.h>
// FOR ANYONE WHO SEES THIS EVERYTHING IS BACKWARDS ITS ALL INVERTED RIGHT IS LEFT AND LEFT IS RIGHT YOU  HAVE BEEN WARNED

/**
 * main.c
 */
void main(void)
{
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer
    Clock_Init48MHz();
    ADC0_InitSWTriggerCh17_14_16();

    uint32_t left;    // ch 17
    uint32_t center;  // ch 14
    uint32_t right;   // ch 16

    uint32_t sampleTable[30][3];

    int i;

    while (1)
    {
        // LEFT
        printf("PAUSE\n");
        for (i = 0; i < 30; i++)
        {
            ADC_In17_14_16(&left, &center, &right);
            sampleTable[i][0] = left;
            Clock_Delay1ms(10);
        }
        printf("PAUSE\n");

        // CENTER
        for (i = 0; i < 30; i++)
        {
            ADC_In17_14_16(&left, &center, &right);
            sampleTable[i][1] = center;
            Clock_Delay1ms(10);
        }
        printf("PAUSE\n");

       // RIGHt
        for (i = 0; i < 30; i++)
        {
            ADC_In17_14_16(&left, &center, &right);
            sampleTable[i][2] = right;
            Clock_Delay1ms(10);
        }
        printf("PAUSE\n");

        /* Print full table ONCE after all data collected */
        printf("Row |   Left   Center   Right\n");
        printf("----+--------------------------\n");
        for (i = 0; i < 30; i++)
        {
            printf("%3d | %6lu %8lu %7lu\n",
                   i,
                   (unsigned long)sampleTable[i][0],
                   (unsigned long)sampleTable[i][1],
                   (unsigned long)sampleTable[i][2]);
        }
    }
}
