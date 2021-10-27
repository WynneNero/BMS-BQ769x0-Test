/*----------------------------------------------------------------------------------------------------
 * Title: Fault_Handler.h
 * Authors: Nathaniel VerLee, 2020-2021
 * Contributors: Ryan Heacock, Kurt Snieckus, Matthew Pennock, 2020-2021
 *
 * This file outlines all the structures and functions for handling BMS faults
----------------------------------------------------------------------------------------------------*/

#ifndef FAULT_HANLDER_H
#define FAULT_HANLDER_H

//----------------------------------------------------------------------------------------------------
// This file includes:
#include <msp430.h>
#include <stdbool.h>
#include <stdint.h>
#include <System.h>


//----------------------------------------------------------------------------------------------------
// Enumerations

typedef enum
{
    TRIPPED,
    CLEARED
} FaultState_t;

typedef enum
{
    POSITIVE,
    NEGATIVE
} Polarity_t;

//----------------------------------------------------------------------------------------------------
// AFE Threshold Qualifier Type
typedef struct
{
    uint8_t BitNum;
    uint8_t SetupVal;
} Qual_AFE_t;

bool QualHandler_AFE (Qual_AFE_t *qual);

//----------------------------------------------------------------------------------------------------
// MCU Threshold Qualifier Type
typedef struct
{
    Polarity_t Polarity;
    unsigned int Value;
    unsigned int TripThresh;
    unsigned int QualedSample_CT;
    unsigned int QualedSample_LIM;
} Qual_MCU_t;

bool QualHandler_MCU (Qual_MCU_t *qual);

//----------------------------------------------------------------------------------------------------
// Auto-Retry/User-Reset Qualifier Type
typedef struct
{
    bool AutoRetry;
    uint8_t AutoInterval_CT;
    uint8_t AutoInterval_LIM;
    uint8_t Retry_CT;
    uint8_t Retry_LIM;
    bool NeedUserReset;
} Qual_AUR_t;

bool QualHandler_AUR (Qual_AUR_t *qual);

//----------------------------------------------------------------------------------------------------
// Main fault pair for referencing Latch/Clear structs
typedef struct
{
    FaultState_t State;
    Qual_AFE_t *Latch;
    Qual_MCU_t *Clear;
    const uint8_t ClearBit;

    unsigned int Fault_CT;
    unsigned int Fault_NumBlinks;
      BiColor_t Fault_Color;

} FaultPair_AFE_MCU_t;

bool FaultHandler_AFE_MCU (FaultPair_AFE_MCU_t *pair,
                           BiColorLED_t *led,
                           uint8_t *clearbits,
                           unsigned int data);

//----------------------------------------------------------------------------------------------------
// Main fault pair for referencing Latch/Clear structs
typedef struct
{
    FaultState_t State;
    Qual_AFE_t *Latch;
    Qual_AUR_t *Clear;
    uint8_t ClearBit;

    unsigned int Fault_CT;
    unsigned int Fault_NumBlinks;
    BiColor_t Fault_Color;

} FaultPair_AFE_AUR_t;

bool FaultHandler_AFE_AUR (FaultPair_AFE_AUR_t *pair,
                           BiColorLED_t *led,
                           uint8_t *clearbits);

//----------------------------------------------------------------------------------------------------
// Main fault pair for referencing Latch/Clear structs
typedef struct
{
    FaultState_t State;
    Qual_MCU_t *Latch;
    Qual_MCU_t *Clear;

    unsigned int Fault_CT;
    unsigned int Fault_NumBlinks;
    BiColor_t Fault_Color;

} FaultPair_MCU_MCU_t;

bool FaultHandler_MCU_MCU (FaultPair_MCU_MCU_t *pair,
                           BiColorLED_t *led,
                           unsigned int data);

//----------------------------------------------------------------------------------------------------
// Main fault pair for referencing Latch/Clear structs
typedef struct
{
    FaultState_t State;
    Qual_MCU_t *Latch;
    Qual_AUR_t *Clear;

    unsigned int Fault_CT;
    unsigned int Fault_NumBlinks;
    BiColor_t Fault_Color;

} FaultPair_MCU_AUR_t;

bool FaultHandler_MCU_AUR (FaultPair_MCU_AUR_t *pair,
                           BiColorLED_t *led,
                           bool clearflag,
                           signed int data);

#endif
