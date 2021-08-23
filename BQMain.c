/*----------------------------------------------------------------------------------------------------
 * Project: BMSHP Controller
 * Title: Main.c
 * Authors: Nathaniel VerLee, 2020-2021
 * Contributors: Ryan Heacock, Kurt Snieckus, Matthew Pennock, 2020-2021
 *
 * This file takes care of all I2C communication including initializations, reading and writing,
 * and any associated interrupt handler routines
----------------------------------------------------------------------------------------------------*/
#include <msp430.h>
#include <stdbool.h>
#include "Constants.h"
#include "I2C_Handler.h"
#include "BatteryData.h"
#include "System.h"

//----------------------------------------------------------------------------------------------------
//Constants

//----------------------------------------------------------------------------------------------------
// Enumerations and Defines
enum CellGroup {GroupNull=0, GroupA=1, GroupB=2, GroupC=3 }; //I dont remember what I was doing with this...

enum SYS_State {DEEP_SLEEP, SYS_OFF, SYS_INIT, SYS_RUN, FAULT_DSG, FAULT_CHG, FAULT_ALL};
int SYS_State = SYS_INIT;

enum ButtonState {NPRESSED, PRESSED, SHORT_PRESSED, LONG_PRESSED, LONG_IDLE};
int ButtonState = NPRESSED;

//typedef enum {PRESSED, DBOUNCE1, DBOUNCE2};

//----------------------------------------------------------------------------------------------------
//Variables and Defines
unsigned int VCell1 = 0;
bool BitCCReady = false;
bool BlinkBalance = false;
unsigned int testGIT;

unsigned int Mode = 0;
unsigned int Flag_PB60 = 0;

unsigned int LED_Blinks_CT = 0;
#define LED_Blinks_LIM 5

unsigned int Blink_Period_CT=0;
#define LED_ON_LIM 3
#define LED_OFF_LIM 9
#define Blink_Period_LIM 12

unsigned int Cycle_Period_CT = 0;
#define Cycle_Period_LIM 160

unsigned int BTNPWR_CT = 0;
#define BTN_PRESSED_LIM 3
#define BTN_LONGPRESS_LIM 80

unsigned int BTNPWR_Return = 0;

//----------------------------------------------------------------------------------------------------
//Function prototypes
void Sys_Init(void);
unsigned int Button_Handler(void);
void LED_Handler(void);
void Alert_Handler(void);

//----------------------------------------------------------------------------------------------------
int main(void)
{
    // MCU Startup Initialization:
    Init_GPIO();
    Init_Sys();
    Init_I2C();

    // AFE and System State Initialization:
    Sys_Init();

    Set_LED_Color(LEDA, RED);

    TB0CTL |= MC_1;

    while (1)
    {
        __bis_SR_register(LPM0_bits|GIE);   // Enter LPM0 w/ interrupt

        //LED_Handler();
        BTNPWR_Return = Button_Handler();

        if(BTNPWR_Return==SHORT_PRESSED)
        {   Set_LED_State(LEDA, LED_ON); }
        if(BTNPWR_Return==LONG_PRESSED)
        {   Set_LED_State(LEDA, LED_OFF); }

    }
}

//----------------------------------------------------------------------------------------------------
void Sys_Init(void)
{
    WDTCTL = WDTPW | WDTHOLD;

    //Blink Green LED60 on system initialization:
    //Set_LEDA_Green();
    Set_LED_Color(LEDA, GREEN);
    Set_LED_State(LEDA, LED_ON);
    __delay_cycles(100000);
    Set_LED_State(LEDA, LED_OFF);
    __delay_cycles(100000);

    //Setup for BQ769x0:
    __delay_cycles(100000);
    Init_BMSConfig();
    __delay_cycles(100000);

    //Blink Green LED60 again on AFE config:
    Set_LED_State(LEDA, LED_ON);
    __delay_cycles(100000);
    Set_LED_State(LEDA, LED_OFF);
    __delay_cycles(100000);

    // Configure Timer_A for button debounce
    TB0CTL = TBSSEL_1 | TBCLR | TBIE;      // ACLK, count mode, clear TBR, enable interrupt
    TB0CCR0 = 1000;
    TB0CCR1 = 524;
    TB0CCTL1 = CCIE;

    Set_ChargePump_On();

    __delay_cycles(750000);


}

//----------------------------------------------------------------------------------------------------
unsigned int Button_Handler(void)
{
    switch (ButtonState)
    {
        case NPRESSED:
            if((BTNPWR_PIN&=BTNPWR)==0)
            {   BTNPWR_CT++;
                ButtonState=PRESSED;        }
            return NPRESSED;

        case PRESSED:
            if((BTNPWR_PIN&=BTNPWR)==0 && BTNPWR_CT>=BTN_PRESSED_LIM)
            {   BTNPWR_CT++;
                ButtonState=SHORT_PRESSED;  }
            else if((BTNPWR_PIN&=BTNPWR)==0)
            {   BTNPWR_CT++;                }
            else if(BTNPWR_PIN|=~BTNPWR)
            {   BTNPWR_CT=0;                }
            return NPRESSED;

        case SHORT_PRESSED:
            if((BTNPWR_PIN&=BTNPWR)==0 && BTNPWR_CT>=BTN_LONGPRESS_LIM)
            {   BTNPWR_CT++;
                ButtonState=LONG_PRESSED;
                return LONG_PRESSED;    }
            else if((BTNPWR_PIN&=BTNPWR)==0)
            {   BTNPWR_CT++;                }
            else if(BTNPWR_PIN|=~BTNPWR)
            {   BTNPWR_CT=0;
                ButtonState=NPRESSED;
                return SHORT_PRESSED;       }
            return NPRESSED;

        case LONG_PRESSED:
            if((BTNPWR_PIN&=BTNPWR)==0)
            {   return NPRESSED;            }
            else if(BTNPWR_PIN|=~BTNPWR)
            {   BTNPWR_CT=0;
                ButtonState=NPRESSED;
                return NPRESSED;            }
    }
    return NPRESSED;
}

//----------------------------------------------------------------------------------------------------
void LED_Handler(void)
{

    if(Blink_Period_CT>LED_ON_LIM)
    {
        //Set_LEDA_Off();
        Set_LED_State(LEDA, LED_OFF);
    }

    if(LED_Blinks_CT<LED_Blinks_LIM)
    {

        if(Blink_Period_CT>Blink_Period_LIM)
        {
            //Set_LEDA_Red();
            Set_LED_State(LEDA, LED_ON);
            Blink_Period_CT=0;
            LED_Blinks_CT++;
        }
    }

    if(Cycle_Period_CT>Cycle_Period_LIM)
    {
        //Set_LEDA_Red();
        Set_LED_State(LEDA, LED_ON);
        Cycle_Period_CT=0;
        LED_Blinks_CT=0;
    }
}

//----------------------------------------------------------------------------------------------------
//Handle incoming alerts on the I2C Interrupt line
void Alert_Handler(void)
{

}



//----------------------------------------------------------------------------------------------------
// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    P1IFG &= ~BIT1;                             // Clear P1.1 IFG
    __bic_SR_register_on_exit(LPM0_bits);       // Exit LPM3
}

//----------------------------------------------------------------------------------------------------
// Port 2 interrupt service routine
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
    //P2IFG &= ~BIT2;                              // Clear P2.2 IFG
    //TB0CTL |= MC_1;                              // Start the timer
    //__bic_SR_register_on_exit(LPM0_bits);      // Back to Bed
}

//----------------------------------------------------------------------------------------------------
// Timer0_B3 Interrupt Vector (TBIV) handler
#pragma vector=TIMER0_B1_VECTOR
__interrupt void TIMER0_B1_ISR(void)
{
    switch(__even_in_range(TB0IV,TB0IV_TBIFG))
    {
        case TB0IV_NONE:
            break;                               // No interrupt
        case TB0IV_TBCCR1:
            //Flag_PB60=Flag_PB60+1;
            //LED_Period_CT++;
            Blink_Period_CT++;
            Cycle_Period_CT++;
            __bic_SR_register_on_exit(LPM0_bits);
            break;
        case TB0IV_TBCCR2:
            break;                               // CCR2 not used
        case TB0IV_TBIFG:
            break;
        default:
            break;
    }
}


