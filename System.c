/*----------------------------------------------------------------------------------------------------
 * Title: System.c
 * Authors: Nathaniel VerLee, Matthew Pennock, 2020-2021
 * Contributors: Ryan Heacock, Kurt Snieckus, Matthew Pennock, 2020-2021
 *
 * This file organizes the general system features like LEDs, buttons, Temp Sensors, Etc
 * and provides appropriate functions for accessing and modifying them
----------------------------------------------------------------------------------------------------*/

//----------------------------------------------------------------------------------------------------
// This file includes:
#include <msp430.h>
#include <stdbool.h>
#include <System.h>
#include <Constants.h>
#include <stdint.h>
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
// Constants
const unsigned int Cycle_LIM        =160;
const unsigned int Blink_ONLIM      =1;
const unsigned int Blink_PeriodLIM  =12;

//----------------------------------------------------------------------------------------------------
//Lookup table to convert `BiColor_t` to `ColorState_t`
static const ColorState_t ColorMap[4] =
{
    {0, 0}, // Red
    {1, 0}, // Red
    {1, 1}, // Yellow
    {0, 1}, // Green
};

//----------------------------------------------------------------------------------------------------
// Configure GPIO
void Init_GPIO()
{
    // Configure Port 1 GPIO, I2C Pins on P1.2, P1.3:
    I2C_AFE_PSEL0 |= I2C_AFE_SDA | I2C_AFE_SCL;                  // I2C pins

    // Configure Port 1 GPIO, Interrupt on P1.1:
    I2C_ALRT1_POUT &= ~I2C_ALRT1;                         // Clear P1.4 output latch for a defined power-on state
    I2C_ALRT1_PDIR &= ~I2C_ALRT1;                          // Set P1.4 to input
    I2C_ALRT1_POUT &= ~I2C_ALRT1;                          // Pull up direction Don't Care
    I2C_ALRT1_PREN |= I2C_ALRT1;                          // P1.1 pull-up register disable

    P1IES &= ~BIT1;                         // P1.1 Interrupt from Low-to-High
    P1IE |= BIT1;                           // P1.1 Interrupt enabled
    P1IFG &= ~BIT1;                         // P1.1 IFG cleared

    DBUGOUT_POUT &= ~DBUGOUT_1;
    DBUGOUT_PDIR |= DBUGOUT_1;
    DBUGOUT_POUT &= ~DBUGOUT_2;
    DBUGOUT_PDIR |= DBUGOUT_2;

    Setup_Buttons();
    Setup_LEDs();
    Setup_GateDriver();
}

//----------------------------------------------------------------------------------------------------
void Init_Sys()
{
    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;
}

//------------------------------//--------------------------------------------------------------------
void Setup_LEDs(void)
{
    //Enable the LEDs with the low side enable FET
    LEDEN_POUT &= ~LEDEN;
    LEDEN_PDIR |= LEDEN;
    LEDEN_POUT |= LEDEN;              // Turn the Enable FET on

    // Configure Port 2 GPIO
    LEDA_POUT &= ~LEDA_GRN;             // Clear P2.0 output latch for a defined power-on state
    LEDA_PDIR |= LEDA_GRN;              // P2.0 Set to Output
    LEDA_POUT &= ~LEDA_RED;             // Clear P2.1 output latch for a defined power-on state
    LEDA_PDIR |= LEDA_RED;              // P2.1 Set to Output

    // Configure Port 4 GPIO
    LEDB_POUT &= ~LEDB_GRN;             // Clear P4.0 output latch for a defined power-on state
    LEDB_PDIR |= LEDB_GRN;              // 4.0 Set to Output
    LEDB_POUT &= ~LEDB_RED;             // Clear P4.1 output latch for a defined power-on state
    LEDB_PDIR |= LEDB_RED;              // P4.1 Set to Output
}

//----------------------------------------------------------------------------------------------------
void Setup_Buttons(void)
{
    // Configure Port 2 GPIO, Button on P2.2:

    BTNPWR_POUT &= ~BTNPWR;     // Clear Button Pin output latch for a defined power-on state
    BTNPWR_PDIR &= ~BTNPWR;     // Set Button Pin to Input
    BTNPWR_POUT |= BTNPWR;      // Set Button Pin Pullup resistor
    BTNPWR_PREN |= BTNPWR;      // Enable Button Pin Pullup/down Resistor

    //BTNPWR_IES |= BTNPWR;            // Set Button Pin interrupt from High-to-Low
    //BTNPWR_IE |= BTNPWR;             // Button Pin interrupt enabled
    //BTNPWR_IFG &= ~BTNPWR;           // Button Pin interrupt flag cleared
}



//----------------------------------------------------------------------------------------------------
void Setup_GateDriver(void)
{
    GTDRV_POUT &= ~GTDRV_BATMONEN;         // Clear output latch for a defined power-on state
    GTDRV_PDIR |= GTDRV_BATMONEN;          // Set to Output

    GTDRV_POUT &= ~GTDRV_CPEN;             // Clear output latch for a defined power-on state
    GTDRV_PDIR |= GTDRV_CPEN;              // Set to Output

    GTDRV_POUT &= ~GTDRV_PCHG;             // Clear output latch for a defined power-on state
    GTDRV_PDIR |= GTDRV_PCHG;              // Set to Output


}

//----------------------------------------------------------------------------------------------------
void Set_ChargePump_On(void)
{
    GTDRV_POUT |= GTDRV_CPEN;      // Turn on the charge pump
}

//----------------------------------------------------------------------------------------------------
void Set_ChargePump_Off(void)
{
    GTDRV_POUT &= ~GTDRV_CPEN;      // Turn on the charge pump
}

//----------------------------------------------------------------------------------------------------
// Set a bit within a memory register
void Register_Bit_Set(volatile unsigned char *reg, unsigned int bit, unsigned int value)
{
    if (value)
    {   *reg |= 1 << bit;       }
    else
    {   *reg &= ~(1 << bit);    }
}

//----------------------------------------------------------------------------------------------------
// Modify the LED mode - this actually needs to then do something?
void Set_LED_Static (BiColorLED_t *led, BiColor_t color)
{
    led->LED_Mode = LEDMode_STATIC;

    const ColorState_t *state = &ColorMap[color];
    Register_Bit_Set(led->PXOUT, led->Pin_Red, state->red);
    Register_Bit_Set(led->PXOUT, led->Pin_Green, state->green);

    return;
}

//----------------------------------------------------------------------------------------------------
// This definitly needs to do something, which may or may not interact with the handler function
// if the LED is set to blink
void Set_LED_Blinks (BiColorLED_t *led, BiColor_t color, unsigned int blinks)
{
    led->LED_Mode = LEDMode_BLINK;
    led->Next_Color = color;
    led->Next_LIM = blinks;
}

//----------------------------------------------------------------------------------------------------
void LED_BlinkHandler(BiColorLED_t *led, unsigned int cycleCT)
{

    const ColorState_t *state = &ColorMap[led->LED_Color];

    switch(led->LED_Mode)
    {
    case LEDMode_BLINK:


        //If the blink period counter is greater than the on time limit, turn off the LED:
        if(led->Blink_PeriodCT>Blink_ONLIM)
        {
            //Turn off the LED (Set both pins to off):
            Register_Bit_Set(led->PXOUT, led->Pin_Red, 0);
            Register_Bit_Set(led->PXOUT, led->Pin_Green, 0);
        }
        //If the blink count is less than the number of blinks
        if(led->Blinks_CT<led->Blinks_LIM)
        {
            //And if the period count is greater than the limit
            if(led->Blink_PeriodCT>Blink_PeriodLIM)
            {
                //Set the LED
                Register_Bit_Set(led->PXOUT, led->Pin_Red, state->red);
                Register_Bit_Set(led->PXOUT, led->Pin_Green, state->green);
                //Reset the period count and increment the number of blinks
                led->Blink_PeriodCT=0;
                led->Blinks_CT++;
            }
        }

        if(cycleCT>Cycle_LIM)
        {
            Register_Bit_Set(led->PXOUT, led->Pin_Red, state->red);
            Register_Bit_Set(led->PXOUT, led->Pin_Green, state->green);
            led->Blinks_CT=0;
            led->Blinks_LIM=led->Next_LIM;  //Load in new buffered blink count limit
            led->LED_Color=led->Next_Color; //load in new buffered color
        }

        break;

    case LEDMode_STATIC:
        break;
    }
}
