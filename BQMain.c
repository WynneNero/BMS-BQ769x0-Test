/*----------------------------------------------------------------------------------------------------
 * Project: BMSHP Controller
 * Title: Main.c
 * Authors: Nathaniel VerLee, Matthew Pennock, 2020-2021
 * Contributors: Ryan Heacock, Kurt Snieckus, Matthew Pennock, 2020-2021
 *
 * This file takes care of all I2C communication including initializations, reading and writing,
 * and any associated interrupt handler routines
----------------------------------------------------------------------------------------------------*/
#include <msp430.h>
#include <stdbool.h>
#include "Constants.h"
#include "I2C_Handler.h"
#include "Fault_Handler.h"
#include "BatteryData.h"
#include "System.h"
#include "UART_Interface.h"

//----------------------------------------------------------------------------------------------------
//Constants

//----------------------------------------------------------------------------------------------------
// Enumerations and associated variables
enum CellGroup {GroupNull=0, GroupA=1, GroupB=2, GroupC=3 };

enum SYS_State {DEEP_SLEEP, SYS_OFF, SYS_INIT, SYS_RUN};
unsigned int SYS_State = SYS_INIT;

enum ButtonState {NPRESSED, PRESSED, SHORT_PRESSED, LONG_PRESSED, LONG_IDLE};
unsigned int ButtonState = NPRESSED;


//----------------------------------------------------------------------------------------------------
//Flow control flag variables

bool Flag_LEDBTN = false;
bool Flag_AFEALRT = false;
bool Flag_USRRST = false;
bool Flag_FAULT = false;

bool BTNA_LongPress = false;

bool Flag_NCHG = false;
bool Flag_NDSG = false;

uint8_t PrevFETBits=0x03; // DSG_ON=BIT1, CHG_ON=BIT0
uint8_t FETBits=0x03; // DSG_ON=BIT1, CHG_ON=BIT0

uint8_t ClearBits=0x00;


//typedef enum {PRESSED, DBOUNCE1, DBOUNCE2};

//----------------------------------------------------------------------------------------------------
//Variables and Definitions

//LEDs
unsigned int Blink_Period_CT    =0;
unsigned int Cycle_Period_CT    =0;
#define Cycle_Period_LIM        160
//Buttons
unsigned int BTNPWR_CT = 0;
#define BTN_PRESSED_LIM 3
#define BTN_LONGPRESS_LIM 80
unsigned int BTNPWR_Return = 0;
//Counter to check ALERT pin as a backup to edge interrupt
unsigned int SYS_Checkin_CT = 0;
#define SYS_Checkin_LIM 16

//Cell Voltages
unsigned int Cell_VMin = 0;

//----------------------------------------------------------------------------------------------------
// Struct Initializations:

//LEDs:
// LEDName = PXOUT, Pin_Red, Pin_Green, LED_Mode, LED_Color, Blinks_LIM, Blinks_CT
//#pragma PERSISTENT(LEDA);
//#pragma PERSISTENT(LEDB);
extern BiColorLED_t LEDA = {&P2OUT, 1, 0, LEDMode_STATIC, BiColor_OFF, 1, 0, 0};
extern BiColorLED_t LEDB = {&P4OUT, 1, 0, LEDMode_STATIC, BiColor_OFF, 1, 0, 0};

//Faults:
static Qual_AFE_t OVP_Latch = {2, 0x00};
static Qual_MCU_t OVP_Clear = {NEGATIVE, 0x2329, 0x2328  , 0, 20};
static FaultPair_AFE_MCU_t OVP_Pair =  {CLEARED, &OVP_Latch, &OVP_Clear, BIT3, 0, 7, BiColor_GREEN};

static Qual_AFE_t UVP_Latch = {3, 0x00};
static Qual_MCU_t UVP_Clear = {POSITIVE, 0x1771, 0x1770, 0, 20};
static FaultPair_AFE_MCU_t UVP_Pair =  {CLEARED, &UVP_Latch, &UVP_Clear, BIT3, 0, 7, BiColor_RED};


//----------------------------------------------------------------------------------------------------
//Function prototypes
void Init_App(void);

unsigned int Button_Handler(void);
//void LED_Handler(int Mode);

void Alert_Handler(void);
void System_Handler(void);
void Fault_Handler(void);

//----------------------------------------------------------------------------------------------------
int main(void)
{
    // MCU Startup Initialization:
      Init_GPIO();
    Init_Sys();
    Init_I2C();

    // AFE and System State Initialization:
    Init_App();
    __delay_cycles(100000);

    TB0CTL |= MC_1;

    Init_UART();

    while (1)
    {
        __bis_SR_register(LPM0_bits|GIE);   // Enter LPM0 w/ interrupt
        __delay_cycles(10);
        //__bic_SR_register(GIE); // Disable global interrupts


        //------------------------------------------------------------------------------------------
        // Main Operational State machine is called here after 0.25S Coulomb counter aquisition *OR*
        // If a protection is triggered
        if(Flag_AFEALRT)
        {
            //HERE IS ITS!!! The grandiose state machine that calls all the shots:
            switch(SYS_State)
            {
            //--------------------------------------------------------------------------------
            case SYS_INIT:

                Update_SysStat();
                Clear_SysStat();

                Set_LED_Static(&LEDB, BiColor_OFF);
                Set_LED_Blinks(&LEDA, BiColor_YELLOW, 1);

                SYS_State=SYS_RUN;
                SYS_Checkin_CT=0;

                break;
            //--------------------------------------------------------------------------------
            case SYS_RUN:

                Update_SysStat();

                Update_VCells(GroupA);
                Update_VCells(GroupB);

                Cell_VMin = Get_VCell_ADC(7);

                Alert_Handler();
                Fault_Handler();
                SYS_Checkin_CT=0;

                if(Flag_USRRST)
                {   Flag_USRRST=false;  }

                SYS_Checkin_CT=0;
                break;

            }
            DBUGOUT_POUT &= ~DBUGOUT_2;
            Flag_AFEALRT=false;
        }


        //------------------------------------------------------------------------------------------
        // Both button and LED state machines are called here when Timer B0 wakes
        if(Flag_LEDBTN)
        {

            //DBUGOUT_POUT |= DBUGOUT_2;
            __delay_cycles(10);

            //LED Blink handlers called here to do blinks if called for:
            LED_BlinkHandler(&LEDA, Cycle_Period_CT);
            LED_BlinkHandler(&LEDB, Cycle_Period_CT);

            if(Cycle_Period_CT>Cycle_Period_LIM)
            {   Cycle_Period_CT=0;  }

            BTNPWR_Return = Button_Handler();
            if(BTNPWR_Return==SHORT_PRESSED)
            {   __delay_cycles(10);     }
            if(BTNPWR_Return==LONG_PRESSED)
            {   Flag_USRRST=true;       }
            // This acts as a backup if for some reason the system misses the ALERT interrupt,
            // also convenient when it is masked during debugging:
            if((I2C_ALRT1_PIN|=I2C_ALRT1) && (SYS_Checkin_CT>SYS_Checkin_LIM))
            {   Flag_AFEALRT = true; }

            DBUGOUT_POUT &= ~DBUGOUT_1;
            Flag_LEDBTN = false;
        }

    }
}

//----------------------------------------------------------------------------------------------------
void Init_App(void)
{
    WDTCTL = WDTPW | WDTHOLD;

    //Blink Green LED60 on system initialization:
    Set_LED_Static(&LEDA, BiColor_RED);
    __delay_cycles(100000);
    Set_LED_Static(&LEDA, BiColor_OFF);
    __delay_cycles(100000);
    Set_LED_Static(&LEDA, BiColor_GREEN);
    __delay_cycles(100000);
    Set_LED_Static(&LEDA, BiColor_OFF);
    __delay_cycles(100000);

    //Setup for BQ769x0:
    __delay_cycles(100000);
    Init_BMSConfig();
    __delay_cycles(100000);
    Set_ChargePump_On();
    Set_CHG_DSG_Bits(BIT1+BIT0);

    //Blink Green LED60 again on AFE config:
    Set_LED_Static(&LEDB, BiColor_RED);
    __delay_cycles(100000);
    Set_LED_Static(&LEDB, BiColor_OFF);
    __delay_cycles(100000);
    Set_LED_Static(&LEDB, BiColor_GREEN);
    __delay_cycles(100000);
    Set_LED_Static(&LEDB, BiColor_OFF);
    __delay_cycles(100000);



    // Configure Timer_A for button debounce
    TB0CTL = TBSSEL_1 | TBCLR | TBIE;      // ACLK, count mode, clear TBR, enable interrupt
    TB0CCR0 = 1000;
    TB0CCR1 = 524;
    TB0CCTL1 = CCIE;



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
void Fault_Handler(void)
{
    FaultHandler_AFE_MCU(&UVP_Pair, &LEDB, &ClearBits, Cell_VMin);


    //Protections which inhibit CHG FET:
    //if(OVP_Pair.State==TRIPPED)
    //{   FETBits |= BIT0;    }
    //else if(OVP_Pair.State==CLEARED)
    //{   FETBits &= ~BIT0;   }


    //Protections which inhibit DSG FET:
    if(UVP_Pair.State==TRIPPED)
    {   FETBits &= ~BIT1;                   }
    else if(UVP_Pair.State==CLEARED)
    {   FETBits |= BIT1;                    }

    //If you do the same type of statement for things that trip both FETs you will override previous
    //states, so include the statements for things like OTPB in BOTH CHG and DSG inhibit statements

    //If there is a change, set the FET bits:

    if(ClearBits!=0x00)
    {   Clear_FaultBits(ClearBits);
        ClearBits=0x00;                     }


    if(FETBits!=PrevFETBits)
    {   Set_CHG_DSG_Bits(FETBits);
        PrevFETBits=FETBits;                }

    if(FETBits==(BIT1+BIT0))
    {   Set_LED_Static(&LEDB, BiColor_OFF); }

}


//----------------------------------------------------------------------------------------------------
//Handle incoming alerts on the I2C Interrupt line
void Alert_Handler()
{
    Update_SysStat();

    if(GetBit_CCReady())
    {   Clear_CCReady();    }
}


//----------------------------------------------------------------------------------------------------
// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    P1IFG &= ~BIT1;                             // Clear P1.1 IFG
    DBUGOUT_POUT |= DBUGOUT_2;
    Flag_AFEALRT=true;
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
            DBUGOUT_POUT |= DBUGOUT_1;
            LEDA.Blink_PeriodCT++;
            LEDB.Blink_PeriodCT++;
            Cycle_Period_CT++;
            SYS_Checkin_CT++;
            Flag_LEDBTN = true;
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


