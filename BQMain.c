/*----------------------------------------------------------------------------------------------------
 * //---//---//---//---//---//---//---//---//---//---//---//---//---//---//---//---//---//---//---//
 * Project: BMS-LT Controller
 * Title: BQMain.c
 * Authors: Nathaniel VerLee, Matthew Pennock, 2020-2021
 * Contributors: Ryan Heacock, Kurt Snieckus, Matthew Pennock, 2020-2021
 *
 * This file deals will all high level program flow control and systemn level interrupts.
 * //---//---//---//---//---//---//---//---//---//---//---//---//---//---//---//---//---//---//---//
----------------------------------------------------------------------------------------------------*/

//----------------------------------------------------------------------------------------------------
// INCLUDES
#include <msp430.h>
#include <stdbool.h>
#include <stdint.h>
#include "QmathLib.h"
#include "IQmathLib.h"
#include "Constants.h"
#include "I2C_Handler.h"
#include "Fault_Handler.h"
#include "BatteryData.h"
#include "System.h"
#include "UART_Interface.h"
#include "Persistent.h"
#include "ParameterData.h"

//----------------------------------------------------------------------------------------------------
// CONSTANTS

//----------------------------------------------------------------------------------------------------
// ENUMS and associated variables
enum CellGroup {GroupNull=0, GroupA=1, GroupB=2, GroupC=3 };

enum SYS_State {DEEP_SLEEP, SYS_OFF, SYS_INIT, SYS_RUN};
unsigned int SYS_State = SYS_INIT;

uint8_t ButtonRet_PWR = NPRESSED;
uint8_t ButtonRet_FLT = NPRESSED;

paramResult_t CFGResult = FAILED_NULL;

//----------------------------------------------------------------------------------------------------
//Flow control flag variables

bool Flag_LEDBTN = false;
bool Flag_AFEALRT = false;
bool Flag_USRRST = false;
bool Flag_FAULT = false;

uint8_t PrevFETBits=0x03; // DSG_ON=BIT1, CHG_ON=BIT0
uint8_t FETBits=0x03; // DSG_ON=BIT1, CHG_ON=BIT0

uint8_t ClearBits=0x00;

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
unsigned int Cell_VMax = 0;
unsigned int Cell_VMin = 0;
signed int IMeasured = 0;
signed int IOffset = -170;

//----------------------------------------------------------------------------------------------------
// STRUCT INITS:

//Buttons:
extern Button_t BTN_PWR = {&P2IN, 2, 0, NPRESSED};
extern Button_t BTN_FLT = {&P2IN, 3, 0, NPRESSED};

//LEDs:
// LEDName = PXOUT, Pin_Red, Pin_Green, LED_Mode, LED_Color, Blinks_LIM, Blinks_CT
//#pragma PERSISTENT(LEDA);
//#pragma PERSISTENT(LEDB);

extern BiColorLED_t LEDA = {&P2OUT, 1, 0, LEDMode_STATIC, BiColor_OFF, BiColor_OFF, 1, 0, 0, 0};
extern BiColorLED_t LEDB = {&P4OUT, 1, 0, LEDMode_STATIC, BiColor_OFF, BiColor_OFF, 1, 0, 0, 0};


//----------------------------------------------------------------------------------------------------
// FUNCTION PROTOTYPES:
void Init_App(void);
void Init_Timers(void);
void Alert_Handler(void);
void Fault_Handler(void);

//----------------------------------------------------------------------------------------------------
//---//---//---//---//---//---//---//---//---//---//---//---//---//---//---//---//---//---//---//---//
// BQMAIN:
//---//---//---//---//---//---//---//---//---//---//---//---//---//---//---//---//---//---//---//---//
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

    Init_Timers();
    TB0CTL |= MC_1;

    //Init_UART();



    //A = _IQ16mpy(X, Y);

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
            Alert_Handler();

            Update_VCells(GroupA);
            Update_VCells(GroupB);
            Update_TSReg();

            Cell_VMax = Get_VCell_Max();
            Cell_VMin = Get_VCell_Min();

            Fault_Handler();

            SYS_Checkin_CT=0;

            DBUGOUT_POUT &= ~DBUGOUT_2;
            Flag_AFEALRT=false;
        }

        //------------------------------------------------------------------------------------------
        // Both button and LED state machines are called here when Timer B0 wakes
        if(Flag_LEDBTN)
        {
            //----------------------------------------------------------------------
            //Debug calls:
            //DBUGOUT_POUT |= DBUGOUT_2;
              __delay_cycles(10);

            //----------------------------------------------------------------------
            //Button press handler calls:
            ButtonRet_PWR = Button_Handler(&BTN_PWR);
            ButtonRet_FLT = Button_Handler(&BTN_FLT);

            //----------------------------------------------------------------------
            //LED Blink handler calls:
            LED_BlinkHandler(&LEDA, Cycle_Period_CT);
            LED_BlinkHandler(&LEDB, Cycle_Period_CT);

            if(Cycle_Period_CT>Cycle_Period_LIM)
            {   Cycle_Period_CT=0;  }

            if(ButtonRet_PWR==SHORT_PRESSED)
            {   __delay_cycles(1);     }
            if(ButtonRet_PWR==LONG_PRESSED)
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
// Initialize the "app" running in the main loop.
void Init_App(void)
{
    WDTCTL = WDTPW | WDTHOLD;

    //Blink Green LED60 on system initialization:
    Set_LED_Static(&LEDA, BiColor_RED);
    __delay_cycles(100000);
    Set_LED_Static(&LEDA, BiColor_OFF);
    __delay_cycles(100000);
    Set_LED_Static(&LEDA, BiColor_YELLOW);
    __delay_cycles(100000);
    Set_LED_Static(&LEDA, BiColor_OFF);
    __delay_cycles(100000);
    Set_LED_Static(&LEDA, BiColor_GREEN);
    __delay_cycles(100000);
    Set_LED_Static(&LEDA, BiColor_OFF);
    __delay_cycles(100000);

    //Setup for BQ769x0:
    __delay_cycles(100000);
    CFGResult = ReadCFG(TARGET_FRAM_DFLT0);
    Init_BMSConfig();
    Set_ChargePump_On();
    __delay_cycles(100000);
    Set_CHG_DSG_Bits(BIT1+BIT0);

    //Blink Green LED60 again on AFE config:
    Set_LED_Static(&LEDB, BiColor_RED);
    __delay_cycles(100000);
    Set_LED_Static(&LEDB, BiColor_OFF);
    __delay_cycles(100000);
    Set_LED_Static(&LEDB, BiColor_YELLOW);
    __delay_cycles(100000);
    Set_LED_Static(&LEDB, BiColor_OFF);
    __delay_cycles(100000);
    Set_LED_Static(&LEDB, BiColor_GREEN);
    __delay_cycles(100000);
    Set_LED_Static(&LEDB, BiColor_OFF);
    __delay_cycles(100000);
}

//----------------------------------------------------------------------------------------------------
void Init_Timers(void)
{
    // Configure Timer_A for button debounce
    TB0CTL = TBSSEL_1 | TBCLR | TBIE;      // ACLK, count mode, clear TBR, enable interrupt
    TB0CCR0 = 1000;
    TB0CCR1 = 524;
    TB0CCTL1 = CCIE;
}

//----------------------------------------------------------------------------------------------------
//Handle incoming alerts on the I2C Interrupt line
void Alert_Handler()
{
    Update_SysStat();

    if(GetBit_CCReady())
    {   //First get the Coulomb counter here, then clear
        IMeasured = Update_CCReg();
        //Clear_CCReady();
        IMeasured-=IOffset;
    }

    Clear_SysStat();

    if(IMeasured>IDBLINK1 && IMeasured<ICBLINK1)
    {   Set_LED_Blinks(&LEDA, BiColor_YELLOW, 1);  }

    else if(IMeasured>IDBLINK2 && IMeasured<IDBLINK1)
    {   Set_LED_Blinks(&LEDA, BiColor_RED, 1);   }
    else if(IMeasured>IDBLINK3 && IMeasured<IDBLINK2)
    {   Set_LED_Blinks(&LEDA, BiColor_RED, 2);   }
    else if(IMeasured>IDBLINK4 && IMeasured<IDBLINK3)
    {   Set_LED_Blinks(&LEDA, BiColor_RED, 3);   }
    else if(IMeasured>IDBLINK5 && IMeasured<IDBLINK4)
    {   Set_LED_Blinks(&LEDA, BiColor_RED, 4);   }
    else if(IMeasured>IDBLINK6 && IMeasured<IDBLINK5)
    {   Set_LED_Blinks(&LEDA, BiColor_RED, 5);   }
    else if(IMeasured>IDBLINK7 && IMeasured<IDBLINK6)
    {   Set_LED_Blinks(&LEDA, BiColor_RED, 6);   }

    if(IMeasured<ICBLINK2 && IMeasured>ICBLINK1)
    {   Set_LED_Blinks(&LEDA, BiColor_GREEN, 1);   }
    else if(IMeasured<ICBLINK3 && IMeasured>ICBLINK2)
    {   Set_LED_Blinks(&LEDA, BiColor_GREEN, 2);   }
    else if(IMeasured<ICBLINK4 && IMeasured>ICBLINK3)
    {   Set_LED_Blinks(&LEDA, BiColor_GREEN, 3);   }
    else if(IMeasured<ICBLINK5 && IMeasured>ICBLINK4)
    {   Set_LED_Blinks(&LEDA, BiColor_GREEN, 4);   }
    else if(IMeasured<ICBLINK6 && IMeasured>ICBLINK5)
    {   Set_LED_Blinks(&LEDA, BiColor_GREEN, 5);   }
    else if(IMeasured<ICBLINK7 && IMeasured>ICBLINK6)
    {   Set_LED_Blinks(&LEDA, BiColor_GREEN, 6);   }
}

//----------------------------------------------------------------------------------------------------
// Deal with all of the faults one by one, then adjust FETs accordingly
void Fault_Handler(void)
{
    //Respective fault handlers, priority from lowest to highest for LED indication. Faults that
    //are lower priority will be masked from user LED indication by higher priority faults, but
    //will still properly protect when tripped. There are 15 total Protections:

    //----------------------------------------------------------------------
    //MCU-AUR Current in charge protections
    FaultHandler_MCU_AUR(&MCPC_Pair, &LEDB, Flag_USRRST, IMeasured);        //Priority 15
    FaultHandler_MCU_AUR(&BCPC_Pair, &LEDB, Flag_USRRST, IMeasured);        //Priority 14
    //MCU-AUR Current in discharge protections
    FaultHandler_MCU_AUR(&MCPD_Pair, &LEDB, Flag_USRRST, IMeasured);        //Priority 13
    FaultHandler_MCU_AUR(&BCPD_Pair, &LEDB, Flag_USRRST, IMeasured);        //Priority 12

    //----------------------------------------------------------------------
    //MCU Based Battery Over/Under Temperature Protections
    //FaultHandler_MCU_MCU(&OTPC_Pair, &LEDB, TCFET                         //Priority 11
    //FaultHandler_MCU_MCU(&OTPD_Pair, &LEDB, TDFET                         //Priority 10
    //FaultHandler_MCU_MCU(&OTPS_Pair, &LEDB, TRSense                       //Priority 9
    //FaultHandler_MCU_MCU(&UTPP_Pair, &LEDB, TPCB)                         //Priority 8
    //FaultHandler_MCU_MCU(&OTPP_Pair, &LEDB, TPCB)                         //Priority 7
    //FaultHandler_MCU_MCU(&UTPB_Pair, &LEDB, TBattery)                     //Priority 6
    //FaultHandler_MCU_MCU(&OTPB_Pair, &LEDB, TBattery)                     //Priority 5

    //----------------------------------------------------------------------
    //AFE-AUR Current in discharge protections
    FaultHandler_AFE_AUR(&OCPD_Pair, &LEDB, Flag_USRRST, IMeasured);        //Priority 4
    FaultHandler_AFE_AUR(&SCPD_Pair, &LEDB, Flag_USRRST, IMeasured);        //Priority 3

    //----------------------------------------------------------------------
    //AFE-MCU Voltage Protections
    //FaultHandler_MCU_MCU(&CUBN, &LEDB, VDelta_Neg)
    //FaultHandler_MCU_MCU(&CUBP, &LEDB, VDelta_Pos)
    FaultHandler_AFE_MCU(&UVP_Pair, &LEDB, &ClearBits, Cell_VMin);          //Priority 2
    FaultHandler_AFE_MCU(&OVP_Pair, &LEDB, &ClearBits, Cell_VMax);          //Priority 1

    //----------------------------------------------------------------------
    //Now deal with the outcome of the faults:

    if(Flag_USRRST)
    {   Flag_USRRST=false;  }

    //Protections which inhibit CHG FET:
    if(OVP_Pair.State==TRIPPED)
    {   FETBits &= ~BIT0;                   }
    else if(OVP_Pair.State==CLEARED)
    {   FETBits |= BIT0;                    }

    //Protections which inhibit DSG FET:

    if(MCPC_Pair.State==TRIPPED || BCPC_Pair.State==TRIPPED || MCPD_Pair.State==TRIPPED || BCPD_Pair.State==TRIPPED ||
       OCPD_Pair.State==TRIPPED || SCPD_Pair.State==TRIPPED || UVP_Pair.State==TRIPPED)
    {   FETBits &= ~BIT1;                   }

    if(MCPC_Pair.State==CLEARED && BCPC_Pair.State==CLEARED && MCPD_Pair.State==CLEARED && BCPD_Pair.State==CLEARED &&
       OCPD_Pair.State==CLEARED && SCPD_Pair.State==CLEARED && UVP_Pair.State==CLEARED)
    {   FETBits |= BIT1;                    }

    //If you do the same type of statement for things that trip both FETs you will override previous
    //states, so include the statements for things like OTPB in BOTH CHG and DSG inhibit statements

    ///For AFE Drive protection Latching, write 1 to clear if respective faults were recovered from
    //if(ClearBits!=0x00)
    //{   Clear_FaultBits(ClearBits);
    //    ClearBits=0x00;                     }

    //If there is a change in CHG or DSG, set the respective FET bits:
    if(FETBits!=PrevFETBits)
    {   Set_CHG_DSG_Bits(FETBits);
        PrevFETBits=FETBits;                }

    //Also clear the fault LED upon recover from all faults:
    if(FETBits==(BIT1+BIT0))
    {   Set_LED_Static(&LEDB, BiColor_OFF); }

}

//----------------------------------------------------------------------------------------------------
// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    P1IFG &= ~BIT1;                             // Clear P1.1 IFG
    DBUGOUT_POUT |= DBUGOUT_2;
    Flag_AFEALRT=true;
    DBUGOUT_POUT &= ~DBUGOUT_2;
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
            DBUGOUT_POUT &= ~DBUGOUT_1;
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


