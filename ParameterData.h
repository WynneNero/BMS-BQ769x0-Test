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

    CODE_BCDC,
    CODE_BCDD,
    CODE_BCTC,
    CODE_BCTD,

    CODE_MCDC,
    CODE_MCDD,
    CODE_MCTC,
    CODE_MCTD,

    CODE_OCDD,
    CODE_OCTD,

    //OVPF (Over Voltage Protection Feature):
    CODE_OVDC,  //Over Voltage Delay to Clear
    CODE_OVDL,  //Over Voltage Delay to Latch
    CODE_OVRD,  //Over Voltage Reduction of Discharge
    CODE_OVTC,  //Over Voltage Threshold for Clear
    CODE_OVTL,  //Over Voltage Threshold for Latch

    CODE_SCDD,
    CODE_SCTD,

    CODE_SRRS,

    //UVPF (Under Voltage Protection Feature):
    CODE_UVDC,  //Under Voltage Threshold Clear
    CODE_UVDL,  //Under Voltage Threshold Latch
    CODE_UVRC,  //Under Voltage Reset Behavior
    CODE_UVTC,  //Under Voltage Threshold for Clear
    CODE_UVTL,  //Under Voltage Threshold for Latch
}paramCode_t;

typedef enum
{
    CTry_MR,
    CTry_3X,
    CTry_4X,
    CTry_5X,
    CTry_7X,
    CTry_10X
}CurrTry_t;

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
    FAILED_PARAMEQUALS,
    FAILED_PARAMVALUE,
    FAILED_PARAMVALID,
    FAILED_PARAMDELIM,
    PASSED_PARAM,
    PASSED_ETX
}paramResult_t;

//----------------------------------------------------------------------------------------------------
//STRUCTS:

//----------------------------------------------------------------------------------------------------
typedef struct
{
    char Name[4];
    paramCode_t Code;
}paramList_s;

//----------------------------------------------------------------------------------------------------
typedef struct
{
    _iq16 Proposed;
    _iq16 Adopted;
     const _iq16 L_Lim;
     const _iq16 U_Lim;
}Param_IQ16_LUL_s;

//----------------------------------------------------------------------------------------------------
typedef struct
{
    _q8 Proposed;
    _q8 Adopted;
    const _q8 L_Lim;
    const _q8 U_Lim;
}Param_Q8_LUL_s;

//----------------------------------------------------------------------------------------------------
typedef struct
{
    unsigned int Proposed;
    unsigned int Adopted;
    const unsigned int L_Lim;
    const unsigned int U_Lim;
}Param_UINT_LUL_s;

//----------------------------------------------------------------------------------------------------
typedef struct
{
    unsigned int Proposed;
    unsigned int Adopted;
    const unsigned int Options[2];
}Param_UINT_2OPTS_s;

//----------------------------------------------------------------------------------------------------
typedef struct
{
    unsigned int Proposed;
    unsigned int Adopted;
    const unsigned int Options[4];
}Param_UINT_4OPTS_s;

//----------------------------------------------------------------------------------------------------
typedef struct
{
    unsigned int Proposed;
    unsigned int Adopted;
    const unsigned int Options[8];
}Param_UINT_8OPTS_s;

//----------------------------------------------------------------------------------------------------
typedef struct
{
    unsigned int Proposed;
    unsigned int Adopted;
    const _q8 Options[8];
}Param_Q8_8OPTS_s;

//----------------------------------------------------------------------------------------------------
typedef struct
{
    unsigned int Proposed;
    unsigned int Adopted;
    const _q8 Options[16];
}Param_Q8_16OPTS_s;

/*----------------------------------------------------------------------------------------------------
typedef struct
{
    CurrTry_t Proposed;
    CurrTry_t Adopted;
    const unsigned int Options[6]; //MR, 3X, 4X, 5X, 7X, 10X
}Param_UINT_IOPTS_s;*/

/*----------------------------------------------------------------------------------------------------
typedef struct
{
    unsigned int Proposed;
    unsigned int Adopted;
    const unsigned int Options[6]; //MR, 3X, 4X, 5X, 7X, 10X
}Param_UINT_TOPTS_s;*/

//----------------------------------------------------------------------------------------------------
//FUNCTION PROTOTYPES

paramResult_t ReadCFG(paramTarget_t target);

paramResult_t ProcessNextChar(char data);
paramResult_t LookupParamKey();

paramResult_t CheckParameter(paramCode_t code);
paramResult_t AdoptParameter(paramCode_t code);

paramResult_t CheckParam_IQ16_LUL(unsigned int index);
paramResult_t AdoptParam_IQ16_LUL(unsigned int index);

paramResult_t CheckParam_Q8_LUL(unsigned int index);
paramResult_t AdoptParam_Q8_LUL(unsigned int index);

paramResult_t CheckParam_UINT_LUL(unsigned int index);
paramResult_t AdoptParam_UINT_LUL(unsigned int index);

paramResult_t CheckParam_UINT_2OPTS(unsigned int index);
paramResult_t AdoptParam_UINT_2OPTS(unsigned int index);

paramResult_t CheckParam_UINT_4OPTS(unsigned int index);
paramResult_t AdoptParam_UINT_4OPTS(unsigned int index);

paramResult_t CheckParam_UINT_8OPTS(unsigned int index);
paramResult_t AdoptParam_UINT_8OPTS(unsigned int index);

paramResult_t CheckParam_Q8_8OPTS(unsigned int index);
paramResult_t AdoptParam_Q8_8OPTS(unsigned int index);

paramResult_t CheckParam_Q8_16OPTS(unsigned int index);
paramResult_t AdoptParam_Q8_16OPTS(unsigned int index);

paramResult_t CheckParam_BOOL(unsigned int index);
paramResult_t AdoptParam_BOOL(unsigned int index);

paramResult_t CheckParam_RRMODE(unsigned int index);
paramResult_t AdoptParam_RRMODE(unsigned int index);

int AtoI(char* str);
#endif /* PARAMETERDATA_H */
