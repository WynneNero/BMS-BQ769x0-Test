/*----------------------------------------------------------------------------------------------------
 * Title: System.c
 * Authors: Nathaniel VerLee, 2020-2021
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

//----------------------------------------------------------------------------------------------------

static enum ColorState LEDA_Color = RED;
static enum ColorState LEDB_Color = RED;

//----------------------------------------------------------------------------------------------------



//----------------------------------------------------------------------------------------------------
// Configure GPIO
void Init_GPIO()
{
    // Configure Port 1 GPIO, I2C Pins on P1.2, P1.3:
    I2C_AFE_PSEL0 |= I2C_AFE_SDA | I2C_AFE_SCL;                  // I2C pins

    // Configure Port 1 GPIO, Interrupt on P1.1:
    I2C_ALRT1_POUT &= ~I2C_ALRT1;                         // Clear P1.4 output latch for a defined power-on state
    I2C_ALRT1_PDIR &= I2C_ALRT1;                          // Set P1.4 to input
    I2C_ALRT1_POUT |= I2C_ALRT1;                          // Pull up direction Don't Care
    I2C_ALRT1_PREN &= I2C_ALRT1;                          // P1.1 pull-up register disable

    //P1IES &= ~BIT1;                         // P1.1 Interrupt from Low-to-High
    //P1IE |= BIT1;                           // P1.1 Interrupt enabled
    //P1IFG &= ~BIT1;                         // P1.1 IFG cleared

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

//------------------------------//--------------------------------------------------------------------
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
void Set_LED_Color(int LED, int Color)
{
    switch(LED)
    {
    case LEDA:
        switch(Color)
        {
            case RED:
                LEDA_Color = RED;
                break; // If this isnt here it jumps to each case??
            case YELLOW:
                LEDA_Color = YELLOW;
                break;
            case GREEN:
                LEDA_Color = GREEN;
                break;
        }
        break;
    case LEDB:
        switch(Color)
        {
            case RED:
                LEDB_Color = RED;
                break; // If this isnt here it jumps to each case??
            case YELLOW:
                LEDB_Color = YELLOW;
                break;
            case GREEN:
                LEDB_Color = GREEN;
                break;
        }
        break;
    }
}

void Set_LED_State(int LED, int State)
{
    switch(LED)
    {
    case LEDA:
        switch(State)
        {
        case LED_OFF:
            LEDA_POUT &= ~LEDA_GRN;     // Green LED at P2.0 Off
            LEDA_POUT &= ~LEDA_RED;     // Red LED at P2.1 Off
            break;
        case LED_ON:
            switch(LEDA_Color)
            {
                case RED:
                    LEDA_POUT &= ~LEDA_GRN;     // Red LED at P2.1 Off
                    LEDA_POUT |= LEDA_RED;      // Red LED at P2.1 On
                    break;
                case YELLOW:
                    LEDA_POUT |= LEDA_GRN;      // Green LED at P2.0 On
                    LEDA_POUT |= LEDA_RED;      // Red LED at P2.1 On
                    break;
                case GREEN:
                    LEDA_POUT |= LEDA_GRN;      // Green LED at P2.0 On
                    LEDA_POUT &= ~LEDA_RED;     // Red LED at P2.1 Off
                    break;
            }
        }
        break;
    case LEDB:
        switch(State)
        {
        case LED_OFF:
            LEDB_POUT &= ~LEDB_GRN;     // Green LED at P2.0 Off
            LEDB_POUT &= ~LEDB_RED;     // Red LED at P2.1 Off
            break;
        case LED_ON:
            switch(LEDA_Color)
            {
                case RED:
                    LEDB_POUT &= ~LEDB_GRN;     // Red LED at P2.1 Off
                    LEDB_POUT |= LEDB_RED;      // Red LED at P2.1 On
                    break;
                case YELLOW:
                    LEDB_POUT |= LEDB_GRN;      // Green LED at P2.0 On
                    LEDB_POUT |= LEDB_RED;      // Red LED at P2.1 On
                    break;
                case GREEN:
                    LEDB_POUT |= LEDB_GRN;      // Green LED at P2.0 On
                    LEDB_POUT &= ~LEDB_RED;     // Red LED at P2.1 Off
                    break;
            }
        }
        break;
    }
}

void Setup_GateDriver(void)
{
    GTDRV_POUT &= ~GTDRV_BATMONEN;         // Clear output latch for a defined power-on state
    GTDRV_PDIR |= GTDRV_BATMONEN;          // Set to Output

    GTDRV_POUT &= ~GTDRV_CPEN;             // Clear output latch for a defined power-on state
    GTDRV_PDIR |= GTDRV_CPEN;              // Set to Output

    GTDRV_POUT &= ~GTDRV_PCHG;             // Clear output latch for a defined power-on state
    GTDRV_PDIR |= GTDRV_PCHG;              // Set to Output


}

void Set_ChargePump_On(void)
{
    GTDRV_POUT |= GTDRV_CPEN;      // Turn on the charge pump
}
void Set_ChargePump_Off(void)
{
    GTDRV_POUT &= ~GTDRV_CPEN;      // Turn on the charge pump
}

