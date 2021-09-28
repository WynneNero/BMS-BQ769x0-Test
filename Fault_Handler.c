/*----------------------------------------------------------------------------------------------------
 * Title: Fault_Handler.c
 * Authors: Nathaniel VerLee, 2020-2021
 * Contributors: Ryan Heacock, Kurt Snieckus, Matthew Pennock, 2020-2021
 *
 * This file contains all the structures and functions for handling BMS faults
----------------------------------------------------------------------------------------------------*/

//----------------------------------------------------------------------------------------------------
// This file includes:
#include <msp430.h>
#include <stdbool.h>
#include <Fault_Handler.h>
#include <BatteryData.h>
#include <System.h>
#include <Constants.h>
#include <stdint.h>

//----------------------------------------------------------------------------------------------------
// A fault TRIP or CLEAR is "qualified" by determinging if a pariticlar input data over time is
// consistent with an actionable having occured. There are three ways that this happens:
// 1) An "AFE Threshold" (AFET) QyalType occurs on the AFE and the MCU simply looks at the
//    appropriate bit when I2C interrupt occurs. An AFET can only trip a fault, not clear it.
// 2) An "MCU Threshold" (MCUT) QualType may trip or clear a fauly by comparing an incoming data
//    value to a threshold and increments a counter when it is above the threshold
// 3) An "Auto-Reset/User-Reset" (ARUR) QualType is only for clearing a fault, and will try to do
//    so automatically for a fixed number of times before only resetting a fault with user
//    user intervention via the fault reset button
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
bool FaultHandler_AFE_MCU (FaultPair_AFE_MCU_t *pair, BiColorLED_t *led, unsigned int data)
{


    switch(pair->State)
    {
    case CLEARED:
        if(QualHandler_AFE(pair->Latch))
        {
            Set_LED_Blinks (led, pair->Fault_Color, pair->Fault_NumBlinks);
            pair->State=TRIPPED;
            return false;
        }
        break;

    case TRIPPED:
        pair->Clear->Value = data;
        if(QualHandler_MCU(pair->Clear))
        {
            pair->State=CLEARED;
            return true;
        }
        break;
    }
}

//----------------------------------------------------------------------------------------------------
// AFE Threshold Qualifier Handler
bool QualHandler_AFE (Qual_AFE_t *qual)
{
    if(Get_FaultBit(qual->BitNum))
    {   return true;    }
    else
    {   return false;   }
}

//----------------------------------------------------------------------------------------------------
// MCU Threshold Qualifier Handler
bool QualHandler_MCU (Qual_MCU_t *qual)
{
    switch(qual->Polarity)
    {
    case POSITIVE:
        if(qual->Value>qual->TripThresh)
        {   qual->QualedSample_CT++;    }
        else if(qual->Value<=qual->TripThresh)
        {   qual->QualedSample_CT=0;    }
        break;

    case NEGATIVE:
        if(qual->Value<qual->TripThresh)
        {   qual->QualedSample_CT++;    }
        else if(qual->Value>=qual->TripThresh)
        {   qual->QualedSample_CT=0;    }
        break;
    }

    if(qual->QualedSample_CT>qual->QualedSample_LIM)
    {   qual->QualedSample_CT=0;
        return true;    }
    else
    {   return false;   }


}
