/*----------------------------------------------------------------------------------------------------
 * Title: ParameterData.h
 * Authors: Nathaniel VerLee, Matthew Pennock, 2020-2021
 * Contributors: Ryan Heacock, Kurt Snieckus, Matthew Pennock, 2020-2021
 *
 * This file is the header for persistent.c which stores  variable that need to be stored in FRAM
 * and not be re-initialized to zero during a system startup.
----------------------------------------------------------------------------------------------------*/

#ifndef PARAMETERDATA_H
#define PARAMETERDATA_H

#include <msp430.h>
#include <stdbool.h>
#include <stdint.h>
#include "QmathLib.h"
#include "IQmathLib.h"
#include <System.h>


//----------------------------------------------------------------------------------------------------
//ENUMERATIONS:

typedef enum
{
    TARGET_FRAM_DFLT0,
    TARGET_FRAM_DFLT1,
    TARGET_NFC_CFG0,
    TARGET_NFC_CFG1
}paramTarget_t;

typedef enum
{
    //Command codes are listed in ALPHABETIC ORDER here:

    //--------------------------------------------------
    //VPCG (Voltage Protection Command Group):

    //OVPF (Over Voltage Protection Feature):
    CODE_OVDC, CODE_OVDL,     //Over Voltage Threshold Clear, Latch
    CODE_OVRD,           //Over Voltage Reduction of Discharge
    CODE_OVTC, CODE_OVTL,     //Over Voltage Delay Clear, Latch

    //UVPF (Under Voltage Protection Feature):
    CODE_UVDC, CODE_UVDL,     //Under Voltage Threshold Clear, Latch
    CODE_UVRC,           //Under Voltage Reset Behavior
    CODE_UVTC, CODE_UVTL,     //Under Voltage Delay Clear, Latch
}paramCode_t;

//----------------------------------------------------------------------------------------------------
typedef enum
{
    ParType_NULL,
    ParType_IQ16_LUL,
    ParType_Q8_LUL,
    ParType_UINT_LUL,
    ParType_UINT_4OPTS,
    ParType_UINT_8OPTS,
    ParType_UINT_16OPTS,
    ParType_Q8_8OPTS,
    ParType_Q8_16OPTS,
    ParType_BOOL,
    ParType_RRMODE
}paramType_t;

//----------------------------------------------------------------------------------------------------
typedef enum
{
    PSTEP_1ST_PARCHAR,
    PSTEP_2ND_PARCHAR,
    PSTEP_3RD_PARCHAR,
    PSTEP_4TH_PARCHAR,
    PSTEP_EQUALS,
    PSTEP_1ST_VALCHAR,
    PSTEP_2ND_VALCHAR,
    PSTEP_3RD_VALCHAR,
    PSTEP_4TH_VALCHAR,
    PSTEP_5TH_VALCHAR,
    PSTEP_DELIM,
    PSTEP_SPACE
}parseState_t;

//----------------------------------------------------------------------------------------------------
typedef enum
{
    FAILED_NULL,
    FAILED_PARAMNAME,
    FAILED_EQUALSCHAR,
    FAILED_VALUEENTRY,
    FAILED_VALIDATION,
    FAILED_VALUELIM,
    FAILED_OPTIONLIM,
    FAILED_DELIM,
    FAILED_PARAMTYPEMM,   //MM="MisMatch"
    PASSED_NOCHECK,
    PASSED_CHECKED
}paramResult_t;

//----------------------------------------------------------------------------------------------------
//STRUCTS:

//----------------------------------------------------------------------------------------------------
typedef struct
{
    char Name[4];
    paramCode_t Code;
    paramType_t Type;
}paramList_s;

//----------------------------------------------------------------------------------------------------
typedef struct
{
    char Name[4];
    _iq16 Proposed;
    _iq16 Adopted;
    const _iq16 L_Lim;
    const _iq16 U_Lim;
}Param_IQ16_LUL_s;

//----------------------------------------------------------------------------------------------------
typedef struct
{
    char Name[4];
    _q8 Proposed;
    _q8 Adopted;
    _q8 L_Lim;
    _q8 U_Lim;
}Param_Q8_LUL_s;

//----------------------------------------------------------------------------------------------------
typedef struct
{
    char Name[4];
    unsigned int Proposed;
    unsigned int Adopted;
    const unsigned int L_Lim;
    const unsigned int U_Lim;
}Param_UINT_LUL_s;

//----------------------------------------------------------------------------------------------------
typedef struct
{
    char Name[4];
    unsigned int Proposed;
    unsigned int Adopted;
    const unsigned int Options[4];
}Param_UINT_4OPTS_s;

//----------------------------------------------------------------------------------------------------
//FUNCTION PROTOTYPES

bool ReadCFG(paramTarget_t target);

paramResult_t ProcessNextFRAMChar(char data);
paramResult_t ProcessNextNFCChar(char data);
paramResult_t LookupParamKey();
paramResult_t CheckValue();

//add the code in here as well later:
paramResult_t CheckParameter(paramType_t type);
paramResult_t AdoptParameter(paramType_t type);

paramResult_t CheckParam_IQ16_LUL(unsigned int code);
paramResult_t AdoptParam_IQ16_LUL(unsigned int code);

paramResult_t CheckParam_Q8_LUL(unsigned int code);
paramResult_t AdoptParam_Q8_LUL(unsigned int code);

paramResult_t CheckParam_UINT_LUL(unsigned int code);
paramResult_t AdoptParam_UINT_LUL(unsigned int code);

paramResult_t CheckParam_UINT_4OPTS(unsigned int code);
paramResult_t AdoptParam_UINT_4OPTS(unsigned int code);

paramResult_t CheckParam_BOOL(unsigned int code);
paramResult_t AdoptParam_BOOL(unsigned int code);

paramResult_t CheckParam_RRMODE(unsigned int code);
paramResult_t AdoptParam_RRMODE(unsigned int code);

int AtoI(char* str);
#endif /* PARAMETERDATA_H */
