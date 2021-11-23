/*----------------------------------------------------------------------------------------------------
 * Title: ParameterData.c
 * Authors: Nathaniel VerLee, Matthew Pennock, 2020-2021
 * Contributors: Ryan Heacock, Kurt Snieckus, Matthew Pennock, 2020-2021
 *
 * This file stores most of the persistent variable that need to be stored in FRAM and not be
 * re-initialized to zero during a system startup.
----------------------------------------------------------------------------------------------------*/

#include <msp430.h>
#include <stdbool.h>
#include <stdint.h>
#include <System.h>
#include <ParameterData.h>
//----------------------------------------------------------------------------------------------------
//VARIABLE DECLARATIONS:

//Onboard Default Config Files, Command codes are listed in SYSTEM ORDER:
//This is one of two read only parameter files, for LiFePO4 Chemistry:
#define PARAMFILELEN 100
static const char FRAM_DFLT0[PARAMFILELEN] =    "OVTL=3.90; OVDL=8; "
                                                "OVTC=3.80; OVDC=8; "
                                                "OVRD=20; "


                                                "UVTL=2.80; UVDL=8;"
                                                "UVTC=2.90; UVDC=8;"
                                                "UVRC=50; ";

//This is one of two read only parameter files, for LiC Chemistry:
static const char FRAM_DFLT1[PARAMFILELEN] =    "OVTL=4.20; OVDL=8; "
                                                "OVTC=4.10; OVDC=8; "
                                                "OVRD=20; "

                                                "UVTL=2.80; UVDL=8; "
                                                "UVTC=2.90; UVDC=8; "
                                                "UVRC=50; ";

static char paramBuf[4];
static paramType_t typeBuf = ParType_NULL;
static int codeBuf = 0;
static char valueBuf[4];

static parseState_t parseState;
static const int codeLen = 4;
static const int listLen = 10;

//Assignment of Command Code struct pointers to the stored Parameter List,
//Command codes are assigned in ALPHABETIC ORDER:
static paramList_s ParamLookup[]=
{
     {"OVDC", CODE_OVDC, ParType_Q8_LUL},        //Over Voltage Delay of Clear          //Key 0
     {"OVDL", CODE_OVDL, ParType_Q8_LUL},        //Over Voltage Delay of Latch          //Key 1
     {"OVRD", CODE_OVRD, ParType_UINT8_LUL},     //Over Voltage Delay of Latch          //Key 2
     {"OVTC", CODE_OVTC, ParType_Q8_LUL},        //Over Voltage Threshold for Clear     //Key 3
     {"OVTL", CODE_OVTL, ParType_Q8_LUL},        //Over Voltage Threshold for Latch     //Key 4

     {"UVDC", CODE_UVDC, ParType_Q8_LUL},        //Under Voltage Delay of Clear         //Key 5
     {"UVDL", CODE_UVDL, ParType_Q8_LUL},        //Under Voltage Delay of Latch         //Key 6
     {"UVRD", CODE_UVRC, ParType_UINT8_LUL},     //Under Voltage Delay of Latch         //Key 7
     {"UVTC", CODE_UVTC, ParType_Q8_LUL},        //Under Voltage Threshold for Clear    //Key 8
     {"UVTL", CODE_UVTL, ParType_Q8_LUL},        //Under Voltage Threshold for Latch    //Key 9
};

//static Param_Q8_ULL_s ParamList_Q8_ULL[]=
//{
//     {3.90,
//};


bool ReadCFG_FRAM(paramTarget_t target)
{
    unsigned int IDX;

    switch(target)
    {
    case TARGET_FRAM_DFLT0:
        for (IDX=0; IDX<PARAMFILELEN; IDX++)
        {   ProcessNextChar(FRAM_DFLT0[IDX]);   }
        break;
    case TARGET_FRAM_DFLT1:
        for (IDX=0; IDX<PARAMFILELEN; IDX++)
        {   ProcessNextChar(FRAM_DFLT1[IDX]);   }
        break;
    }
}

//----------------------------------------------------------------------------------------------------
//This function takes in characters as the ReadCFG() commands iterate through their strings. Each
//new character triggers a new subsequent state as well as a determination of string validity for
//that character. Once it has gathered up a parameter name or a parameter value, it triggers
//a function call to process those entries.
paramResult_t ProcessNextChar(char data)
{
    switch(parseState)
    {
    //------------------------------------------------------------------------------------------
    case PSTEP_1ST_PARCHAR:
        if(data>=0x41 && data<=0x5A)        //Is character a Capital Alpha?
        {   paramBuf[0]=data;               //Then put it in the param name buffer
            parseState=PSTEP_2ND_PARCHAR;   //Go to the next character in next call
            return PASSED_NOCHECK;      }
        else
        {   parseState=PSTEP_1ST_PARCHAR;   //Reinit state for another attempt
            return FAILED_PARAMNAME;    }

    //------------------------------------------------------------------------------------------
    case PSTEP_2ND_PARCHAR:
        if(data>=0x41 && data<=0x5A)        //Is character a Capital Alpha?
        {   paramBuf[1]=data;               //Then put it in the param name buffer
            parseState=PSTEP_3RD_PARCHAR;   //Go to the next character in next call
            return PASSED_NOCHECK;      }
        else
        {   parseState=PSTEP_1ST_PARCHAR;   //Reinit state for another attempt
            return FAILED_PARAMNAME;    }

    //------------------------------------------------------------------------------------------
    case PSTEP_3RD_PARCHAR:
        if(data>=0x41 && data<=0x5A)        //Is character a Capital Alpha?
        {   paramBuf[2]=data;               //Then put it in the param name buffer
            parseState=PSTEP_4TH_PARCHAR;   //Go to the next character in next call
            return PASSED_NOCHECK;      }
        else
        {   parseState=PSTEP_1ST_PARCHAR;   //Reinit state for another attempt
            return FAILED_PARAMNAME;    }

    //------------------------------------------------------------------------------------------
    case PSTEP_4TH_PARCHAR:
        if(data>=0x41 && data<=0x5A)        //Is character a Capital Alpha?
        {   paramBuf[3]=data;               //Then put it in the param name buffer
            parseState=PSTEP_EQUALS;        //Go to the next character in next call
            return PASSED_NOCHECK;      }
        else
        {   parseState=PSTEP_1ST_PARCHAR;   //Reinit state for another attempt
            return FAILED_PARAMNAME;    }

     //------------------------------------------------------------------------------------------
    case PSTEP_EQUALS:
        if(data==0x3D)                      //Is character a '='?
        {
            LookupParamKey();               //Then trigger a key lookup
            parseState=PSTEP_1ST_VALCHAR;   //Get ready for value characters next
            return PASSED_NOCHECK;      }
        else
        {   parseState=PSTEP_1ST_PARCHAR;   //Reinit state for another attempt
            return FAILED_EQUALSCHAR;    }

    //------------------------------------------------------------------------------------------
    case PSTEP_1ST_VALCHAR:
        if((data>=0x30 && data<=0x39) || data==0x2E)    //Is character '1' through '9' or '.'?
        {   valueBuf[0]=data;
            parseState=PSTEP_2ND_VALCHAR;
            return PASSED_NOCHECK;       }
        else
        {   parseState=PSTEP_1ST_PARCHAR;   //Reinit state for another attempt
            return FAILED_VALUEENTRY;    }

    //------------------------------------------------------------------------------------------
    case PSTEP_2ND_VALCHAR:
        if((data>=0x30 && data<=0x39) || data==0x2E)    //Is character '1' through '9' or '.'?
        {   valueBuf[1]=data;
            parseState=PSTEP_3RD_VALCHAR;
            return PASSED_NOCHECK;      }
        else if(data==0x3B)                 //Is the character a';' delimiter?
        {   valueBuf[1]=0x00;               //If a value is shorter than the alloted 4 chars,
            valueBuf[2]=0x00;               //Pad out the remaining characters to null chars
            valueBuf[3]=0x00;
            parseState=PSTEP_SPACE;         //Found the ';' delimiter early, jump to space char
            return PASSED_NOCHECK;      }
        else
        {   parseState=PSTEP_1ST_PARCHAR;   //Reinit state for another attempt
            return FAILED_VALUEENTRY;   }

    //------------------------------------------------------------------------------------------
    case PSTEP_3RD_VALCHAR:
        if((data>=0x30 && data<=0x39) || data==0x2E)    //Is character '1' through '9' or '.'?
        {   valueBuf[2]=data;
            parseState=PSTEP_4TH_VALCHAR;
            return PASSED_NOCHECK;      }
        else if(data==0x3B)                 //Is the character a';' delimiter?
        {   valueBuf[2]=0x00;               //Pad out remaining chars to null
            valueBuf[3]=0x00;
            parseState=PSTEP_SPACE;         //Found the ';' delimiter early, jump to space char
            return PASSED_NOCHECK;      }
        else
        {   parseState=PSTEP_1ST_PARCHAR;   //Reinit state for another attempt
            return FAILED_VALUEENTRY;   }

    //------------------------------------------------------------------------------------------
    case PSTEP_4TH_VALCHAR:
        if((data>=0x30 && data<=0x39) || data==0x2E)    //Is character '1' through '9' or '.'?
        {   valueBuf[3]=data;
            parseState=PSTEP_DELIM;
            return PASSED_NOCHECK;      }
        else if(data==0x3B)                 //Is the character a';' delimiter?
        {   valueBuf[3]=0x00;               //Pad out remaining chars to null
            parseState=PSTEP_SPACE;         //Found the ';' delimiter early, jump to space char
            return PASSED_NOCHECK;      }
        else
        {   parseState=PSTEP_1ST_PARCHAR;   //Reinit state for another attempt
            return FAILED_VALUEENTRY;   }

    //------------------------------------------------------------------------------------------
    case PSTEP_DELIM:
        if(data==0x3B)                      //Is the character a';' delimiter?
        {   CheckParameter(typeBuf);               //Trigger checking value against parameter
            parseState=PSTEP_SPACE;         //Found the ';' delimiter early, jump to space char
            return PASSED_NOCHECK;      }
        else
        {   parseState=PSTEP_1ST_PARCHAR;   //Reinit state for another attempt
            return FAILED_VALUEENTRY;   }

    //------------------------------------------------------------------------------------------
    case PSTEP_SPACE:
        if(data==0x20)                      //Is the character a ' ' (Space)?
        {
            parseState=PSTEP_1ST_PARCHAR;   //Space is there, ok
            return PASSED_NOCHECK;      }
        if(data>=0x41 && data<=0x5A)        //Is character a Capital Alpha? (missed the space)
        {   paramBuf[0]=data;               //Then put it in the param name buffer
            parseState=PSTEP_2ND_PARCHAR;   //Go to the next character in next call
            return PASSED_NOCHECK;      }
        else
        {   parseState=PSTEP_1ST_PARCHAR;   //Reinit state for another attempt
            return FAILED_PARAMNAME;    }

    }
    return PASSED_NOCHECK;
}

paramResult_t LookupParamKey()
{
    unsigned int listIndex = 0;
    unsigned int codeIndex = 0;

    while(codeIndex<codeLen && listIndex<listLen)
    {
        if(ParamLookup[listIndex].Name[codeIndex] == paramBuf[codeIndex])
        {   codeIndex++;    }
        else if(ParamLookup[listIndex].Name[codeIndex] < paramBuf[codeIndex])
        {   listIndex++;    }
        if(codeIndex==4)
        {
            codeBuf=listIndex;
            typeBuf=ParamLookup[listIndex].Type;
            return PASSED_NOCHECK;
        }
    }

    return FAILED_PARAMNAME;
}

paramResult_t CheckParameter(paramType_t type)
{
    switch(type)
    {
    //OVPR Group:
    case ParType_Q8_LUL:
        return CheckParam_Q8_LUL(codeBuf);
    case ParType_UINT8_LUL:
        return CheckParam_UINT8_LUL(codeBuf);
    }
    return FAILED_VALIDATION;
}

paramResult_t CheckParam_Q8_LUL(paramCode_t code)
{

    switch(code)
    {
    //OVPR Group:
    case CODE_OVDC:
        //if()
        return PASSED_CHECKED;
    case CODE_OVDL:
        return PASSED_CHECKED;
    case CODE_OVTC:
        return PASSED_CHECKED;
    case CODE_OVTL:
        return PASSED_CHECKED;

    //UVPR Group:
    case CODE_UVDC:
        return PASSED_CHECKED;
    case CODE_UVDL:
        return PASSED_CHECKED;
    case CODE_UVTC:
        return PASSED_CHECKED;
    case CODE_UVTL:
        return PASSED_CHECKED;
    default:
        return FAILED_PARAMTYPEMM;
    }
}


paramResult_t CheckParam_UINT8_LUL(paramCode_t code)
{
    switch(code)
    {
    case CODE_OVRD:
        return PASSED_CHECKED;
    case CODE_UVRC:
        return PASSED_CHECKED;
    default:
        return FAILED_PARAMTYPEMM;
    }
}

