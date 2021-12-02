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
#define PARAMFILELEN 450

#pragma PERSISTENT(FRAM_DFLT0);
static char FRAM_DFLT0[PARAMFILELEN] =      "OVTL=3.90; OVDL=8; "
                                            "OVTC=3.80; OVDC=8.0; "
                                            "OVRD=20; "

                                            "UVTL=2.80; UVDL=8; "
                                            "UVTC=2.90; UVDC=8; "
                                            "UVRC=50; "

                                            //"SCTD=22.25; SCDD=100; SCRD=3X; "
                                            //"OCTD=15.25; OCDD=160; OCRD=3X; "
                                            "BCTD=12.0; BCDD=5; "       //BCRD=4X;
                                            "MCTD=10.0; MCDD=60; "      //MCRD=5X;
                                            "BCTC=10.0; BCDC=5; "       //BCRC=4X;
                                            "MCTC=5.0; MCDC=60; \x03;"; //MCRC=5X;

                                            //"OTTC=100; OTTD=100; OTTS=120; OTTP=120; OTDP=10; OTRP=10X; "
                                            //"OTTB=50; OTDB=10; OTRB=IX; "
                                            //"UTTB=5; UTDB=60; UTRB=IX; "
                                            //"OTTA=50; OTDA=10; OTRA=IX; "
                                            //"UTTA=5; UTDA=60; UTRA=IX; ";


//This is one of two read only parameter files, for LiC Chemistry:
#pragma PERSISTENT(FRAM_DFLT1);
static char FRAM_DFLT1[PARAMFILELEN] =      "OVTL=5.20; OVDL=8; "
                                            "OVTC=4.10; OVDC=8.0; "
                                            "OVRD=20; "

                                            "UVTL=2.80; UVDL=8; "
                                            "UVTC=2.90; UVDC=8; "
                                            "UVRC=50; "

                                            //"SCTD=22.25; SCDD=100; SCRD=3X; "
                                            //"OCTD=15.25; OCDD=160; OCRD=3X; "
                                            "BCTD=12.0; BCDD=5; "       //BCRD=4X;
                                            "MCTD=10.0; MCDD=60; "      //MCRD=5X;
                                            "BCTC=10.0; BCDC=5; "       //BCRC=4X;
                                            "MCTC=5.0; MCDC=60; \x03;"; //MCRC=5X;

                                            //"OTTC=100; OTTD=100; OTTS=120; OTTP=120; OTDP=10; OTRP=10X; "
                                            //"OTTB=50; OTDB=10; OTRB=IX; "
                                            //"UTTB=5; UTDB=60; UTRB=IX; "
                                            //"OTTA=50; OTDA=10; OTRA=IX; "
                                            //"UTTA=5; UTDA=60; UTRA=IX; ";

unsigned int StreamIDX;
static unsigned int BufIDX = 0;
static char paramBuf[4];
static unsigned int codeBuf = 0;
static char valueBuf[6];

static parseState_t parseState;
static const int codeLen = 4;
static const int listLen = 18;

//Assignment of Command Codes to the stored Parameter List,
//Command codes are assigned in ALPHABETIC ORDER:
#pragma PERSISTENT(ParamLookup);
static paramList_s ParamLookup[]=
{
     {"BCDC", CODE_BCDC},   //Burst Current Delay in Charge             //Key 0
     {"BCDD", CODE_BCDD},   //Burst Current Delay in Discharge          //Key 1
     {"BCTC", CODE_BCTC},   //Burst Current Threshold in Charge         //Key 2
     {"BCTD", CODE_BCTD},   //Burst Current Threshold in Discharge      //Key 3

     {"MCDC", CODE_MCDC},   //Maximum Current Delay in Charge           //Key 4
     {"MCDD", CODE_MCDD},   //Maximum Current Delay in Discharge        //Key 5
     {"MCTC", CODE_MCTC},   //Maximum Current Threshold in Charge       //Key 6
     {"MCTD", CODE_MCTD},   //Maximum Current Threshold in Discharge    //Key 7

     {"OVDC", CODE_OVDC},   //Over Voltage Delay of Clear               //Key 8
     {"OVDL", CODE_OVDL},   //Over Voltage Delay of Latch               //Key 9
     {"OVRD", CODE_OVRD},   //Over Voltage Reduction of Discharge       //Key 10
     {"OVTC", CODE_OVTC},   //Over Voltage Threshold for Clear          //Key 11
     {"OVTL", CODE_OVTL},   //Over Voltage Threshold for Latch          //Key 12

     {"UVDC", CODE_UVDC},   //Under Voltage Delay of Clear              //Key 13
     {"UVDL", CODE_UVDL},   //Under Voltage Delay of Latch              //Key 14
     {"UVRC", CODE_UVRC},   //Under Voltage Reduction of Charge         //Key 15
     {"UVTC", CODE_UVTC},   //Under Voltage Threshold for Clear         //Key 16
     {"UVTL", CODE_UVTL},   //Under Voltage Threshold for Latch         //Key 17
};

#pragma PERSISTENT(ParamList_Q8_LUL);
static Param_Q8_LUL_s ParamList_Q8_LUL[]=
{
     {_Q8(0.0), _Q8(0.0), _Q8(3.4), _Q8(4.8)},      //OVTL Index 0
     {_Q8(0.0), _Q8(0.0), _Q8(3.3), _Q8(4.7)},      //OVTC Index 1
     {_Q8(0.0), _Q8(0.0), _Q8(1.0), _Q8(32.0)},     //OVDC Index 2

     {_Q8(0.0), _Q8(0.0), _Q8(2.4), _Q8(3.1)},      //UVTL Index 3
     {_Q8(0.0), _Q8(0.0), _Q8(2.5), _Q8(3.2)},      //UVTC Index 4
     {_Q8(0.0), _Q8(0.0), _Q8(1.0), _Q8(32.0)},     //UVDC Index 5

     {_Q8(0.0), _Q8(0.0), _Q8(5.0),  _Q8(20.0)},    //BCTD Index 6
     {_Q8(0.0), _Q8(0.0), _Q8(1.00), _Q8(30.00)},   //BCDD Index 7
     {_Q8(0.0), _Q8(0.0), _Q8(5.0),  _Q8(12.0)},    //MCTD Index 8
     {_Q8(0.0), _Q8(0.0), _Q8(5.00), _Q8(100.00)},  //MCDD Index 9

     {_Q8(0.0), _Q8(0.0), _Q8(4.00), _Q8(16.0)},    //BCTC Index 10
     {_Q8(0.0), _Q8(0.0), _Q8(1.00), _Q8(30.00)},   //BCDC Index 11
     {_Q8(0.0), _Q8(0.0), _Q8(2.00), _Q8(10.0)},    //MCTC Index 12
     {_Q8(0.0), _Q8(0.0), _Q8(5.00), _Q8(100.00)},  //MCDC Index 13
};

//static Param_Q8_LUL_s TestQStruct = {"OVTL", _Q8(0.0), _Q8(0.0), _Q8(3.4), _Q8(4.8)};

#pragma PERSISTENT(ParamList_UINT_LUL);
static Param_UINT_LUL_s ParamList_UINT_LUL[]=
{
     {0, 0, 5, 100},        //OVRD
     {0, 0, 5, 100},        //UVRC
};

#pragma PERSISTENT(ParamList_UINT_4OPTS);
static Param_UINT_4OPTS_s ParamList_UINT_4OPTS[]=
{
     {0, 0, {1,2,4,8}},     //OVDL
     {0, 0, {1,4,8,16}},    //UVDL
};


paramResult_t ReadCFG(paramTarget_t target)
{
    volatile paramResult_t result;

    switch(target)
    {
    case TARGET_FRAM_DFLT0:
        for (StreamIDX=0; StreamIDX<PARAMFILELEN; StreamIDX++)
        {
            result = ProcessNextChar(FRAM_DFLT0[StreamIDX]);
            if(result!=PASSED_PARAM)    //If parsing failed OR stopped
            {   return result;  }       //leave this loop
        }
        return result;
    case TARGET_FRAM_DFLT1:
        for (StreamIDX=0; StreamIDX<PARAMFILELEN; StreamIDX++)
        {
            result = ProcessNextChar(FRAM_DFLT1[StreamIDX]);
            if(result!=PASSED_PARAM)    //If parsing failed OR stopped
            {   return result;  }       //leave this loop
        }
        return result;
    }
    return FAILED_NULL;
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
    /*case PSTEP_PARCHAR:
        if(data>=0x41 && data<=0x5A)        //Is character a Capital Alpha?
        {
            paramBuf[BufIDX]=data;          //Then put it in the param name buffer
            if(BufIDX<3)
            {
                BufIDX++;                   //Go to the next character in next call
                return PASSED_NOCHECK;
            }
            else
            {
                parseState=PSTEP_EQUALS;    //Go to the next state
                BufIDX=0;                   //Reset the buffer index
                return PASSED_NOCHECK;
            }
        }
        else
        {
            BufIDX=0;
            return FAILED_PARAMNAME;
        }*/

    //------------------------------------------------------------------------------------------
    case PSTEP_1ST_PARCHAR:
        if(data>=0x41 && data<=0x5A)        //Is character a Capital Alpha?
        {   paramBuf[0]=data;               //Then put it in the param name buffer
            parseState=PSTEP_2ND_PARCHAR;   //Go to the next character in next call
            return PASSED_PARAM;      }
        else if(data==0x03)
        {   return PASSED_ETX;      }
        else
        {   parseState=PSTEP_1ST_PARCHAR;   //Reinit state for another attempt
            return FAILED_PARAMNAME;    }

    //------------------------------------------------------------------------------------------
    case PSTEP_2ND_PARCHAR:
        if(data>=0x41 && data<=0x5A)        //Is character a Capital Alpha?
        {   paramBuf[1]=data;               //Then put it in the param name buffer
            parseState=PSTEP_3RD_PARCHAR;   //Go to the next character in next call
            return PASSED_PARAM;      }
        else
        {   parseState=PSTEP_1ST_PARCHAR;   //Reinit state for another attempt
            return FAILED_PARAMNAME;    }

    //------------------------------------------------------------------------------------------
    case PSTEP_3RD_PARCHAR:
        if(data>=0x41 && data<=0x5A)        //Is character a Capital Alpha?
        {   paramBuf[2]=data;               //Then put it in the param name buffer
            parseState=PSTEP_4TH_PARCHAR;   //Go to the next character in next call
            return PASSED_PARAM;      }
        else
        {   parseState=PSTEP_1ST_PARCHAR;   //Reinit state for another attempt
            return FAILED_PARAMNAME;    }

    //------------------------------------------------------------------------------------------
    case PSTEP_4TH_PARCHAR:
        if(data>=0x41 && data<=0x5A)        //Is character a Capital Alpha?
        {   paramBuf[3]=data;               //Then put it in the param name buffer
            parseState=PSTEP_EQUALS;        //Go to the next character in next call
            return PASSED_PARAM;      }
        else
        {   parseState=PSTEP_1ST_PARCHAR;   //Reinit state for another attempt
            return FAILED_PARAMNAME;    }

     //------------------------------------------------------------------------------------------
    case PSTEP_EQUALS:
        if(data==0x3D)                      //Is character a '='?
        {   //Then trigger a key lookup
            parseState=PSTEP_1ST_VALCHAR;   //Get ready for value characters next
            return LookupParamKey();    }
        else
        {   parseState=PSTEP_1ST_PARCHAR;   //Reinit state for another attempt
            return FAILED_PARAMEQUALS;    }

    //------------------------------------------------------------------------------------------
    /*case PSTEP_VALCHAR:
        if((data>=0x30 && data<=0x39) || data==0x2E)    //Is character '1' through '9' or '.'?
        {
            valueBuf[BufIDX]=data;
            BufIDX++;
            return PASSED_NOCHECK;
        }
        else if(data==0x3B)                 //Is the character a';' delimiter?
        {
            valueBuf[BufIDX]=0x00;               //Terminate the string with null char
            BufIDX=0;
            CheckParameter((paramCode_t)codeBuf);        //Check the param against validation data
            parseState=PSTEP_SPACE;         //Found the ';' delimiter early, jump to space char
            return PASSED_NOCHECK;
        }
        else
        {
            parseState=PSTEP_PARCHAR;   //Reinit state for another attempt
            BufIDX=0;
            return FAILED_VALUEENTRY;
        }*/

    //------------------------------------------------------------------------------------------
    case PSTEP_1ST_VALCHAR:
        if((data>=0x30 && data<=0x39) || data==0x2E)    //Is character '1' through '9' or '.'?
        {   valueBuf[0]=data;
            parseState=PSTEP_2ND_VALCHAR;
            return PASSED_PARAM;       }
        else
        {   parseState=PSTEP_1ST_PARCHAR;   //Reinit state for another attempt
            return FAILED_PARAMVALUE;    }

    //------------------------------------------------------------------------------------------
    case PSTEP_2ND_VALCHAR:
        if((data>=0x30 && data<=0x39) || data==0x2E)    //Is character '1' through '9' or '.'?
        {   valueBuf[1]=data;
            parseState=PSTEP_3RD_VALCHAR;
            return PASSED_PARAM;      }
        else if(data==0x3B)                 //Is the character a';' delimiter?
        {   valueBuf[1]=0x00;               //Terminate the string with null char
            parseState=PSTEP_SPACE;         //Found the ';' delimiter early, jump to space char
            return CheckParameter((paramCode_t)codeBuf);   //Check the param against validation data
        }
        else
        {   parseState=PSTEP_1ST_PARCHAR;   //Reinit state for another attempt
            return FAILED_PARAMVALUE;   }

    //------------------------------------------------------------------------------------------
    case PSTEP_3RD_VALCHAR:
        if((data>=0x30 && data<=0x39) || data==0x2E)    //Is character '1' through '9' or '.'?
        {   valueBuf[2]=data;
            parseState=PSTEP_4TH_VALCHAR;
            return PASSED_PARAM;      }
        else if(data==0x3B)                 //Is the character a';' delimiter?
        {   valueBuf[2]=0x00;               //Terminate the string with null char
            parseState=PSTEP_SPACE;         //Found the ';' delimiter early, jump to space char
            return CheckParameter((paramCode_t)codeBuf);    //Check the param against validation data
        }
        else
        {   parseState=PSTEP_1ST_PARCHAR;   //Reinit state for another attempt
            return FAILED_PARAMVALUE;   }

    //------------------------------------------------------------------------------------------
    case PSTEP_4TH_VALCHAR:
        if((data>=0x30 && data<=0x39) || data==0x2E)    //Is character '1' through '9' or '.'?
        {   valueBuf[3]=data;
            parseState=PSTEP_5TH_VALCHAR;
            return PASSED_PARAM;      }
        else if(data==0x3B)                 //Is the character a';' delimiter?
        {   valueBuf[3]=0x00;               //Terminate the string with null char
            parseState=PSTEP_SPACE;         //Found the ';' delimiter early, jump to space char
            return CheckParameter((paramCode_t)codeBuf);    //Check the param against validation data
        }
        else
        {   parseState=PSTEP_1ST_PARCHAR;   //Reinit state for another attempt
            return FAILED_PARAMVALUE;   }

    //------------------------------------------------------------------------------------------
    case PSTEP_5TH_VALCHAR:
        if((data>=0x30 && data<=0x39) || data==0x2E)    //Is character '1' through '9' or '.'?
        {   valueBuf[4]=data;
            parseState=PSTEP_DELIM;
            return PASSED_PARAM;      }
        if(data==0x3B)                      //Is the character a';' delimiter?
        {   valueBuf[4]=0x00;               //Terminate the string with null char
            parseState=PSTEP_SPACE;         //Found the ';' delimiter early, jump to space char
            return CheckParameter((paramCode_t)codeBuf);    //Check the param against validation data
        }
        else
        {   parseState=PSTEP_1ST_PARCHAR;   //Reinit state for another attempt
            return FAILED_PARAMVALUE;   }

    //------------------------------------------------------------------------------------------
    case PSTEP_DELIM:
        if(data==0x3B)                      //Is the character a';' delimiter?
        {   valueBuf[5]=0x00;

            parseState=PSTEP_SPACE;         //Found the ';' delimiter early, jump to space char
            return CheckParameter((paramCode_t)codeBuf);    //Check the param against validation data
        }
        else
        {   parseState=PSTEP_1ST_PARCHAR;   //Reinit state for another attempt
            return FAILED_PARAMDELIM;   }

    //------------------------------------------------------------------------------------------
    case PSTEP_SPACE:
        if(data==0x20)                      //Is the character a ' ' (Space)?
        {
            parseState=PSTEP_1ST_PARCHAR;   //Space is there, ok
            return PASSED_PARAM;      }
        else if(data==0x03)
        {   return PASSED_ETX;      }
        if(data>=0x41 && data<=0x5A)        //Is character a Capital Alpha? (missed the space)
        {   paramBuf[0]=data;               //Then put it in the param name buffer
            parseState=PSTEP_2ND_PARCHAR;   //Go to the next character in next call
            return PASSED_PARAM;      }
        else
        {   parseState=PSTEP_1ST_PARCHAR;   //Reinit state for another attempt
            return FAILED_PARAMDELIM;    }

    }
    return PASSED_PARAM;
}

//----------------------------------------------------------------------------------------------------
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
            //typeBuf=ParamLookup[listIndex].Type;
            return PASSED_PARAM;
        }
    }

    return FAILED_PARAMNAME;
}

//----------------------------------------------------------------------------------------------------
paramResult_t CheckParameter(paramCode_t code)
{
    unsigned int index = 0;

    switch(code)
    {
    case CODE_BCDC:
        index = 11;
        return CheckParam_Q8_LUL(index);
    case CODE_BCDD:
        index = 7;
        return CheckParam_Q8_LUL(index);
    case CODE_BCTC:
        index = 10;
        return CheckParam_Q8_LUL(index);
    case CODE_BCTD:
        index = 6;
        return CheckParam_Q8_LUL(index);

    case CODE_MCDC:
        index = 13;
        return CheckParam_Q8_LUL(index);
    case CODE_MCDD:
        index = 9;
        return CheckParam_Q8_LUL(index);
    case CODE_MCTC:
        index = 12;
        return CheckParam_Q8_LUL(index);
    case CODE_MCTD:
        index = 8;
        return CheckParam_Q8_LUL(index);

    case CODE_OVDC:
        index = 2;
        return CheckParam_Q8_LUL(index);
    case CODE_OVDL:
        index = 0;
        return CheckParam_UINT_4OPTS(index);
    case CODE_OVRD:
        index = 0;
        return CheckParam_UINT_LUL(index);
    case CODE_OVTC:
        index = 1;
        return CheckParam_Q8_LUL(index);
    case CODE_OVTL:
        index = 0;
        return CheckParam_Q8_LUL(index);

    case CODE_UVDC:
        index = 5;
        return CheckParam_Q8_LUL(index);
    case CODE_UVDL:
        index = 1;
        return CheckParam_UINT_4OPTS(index);
    case CODE_UVRC:
        index = 1;
        return CheckParam_UINT_LUL(index);
    case CODE_UVTC:
        index = 4;
        return CheckParam_Q8_LUL(index);
    case CODE_UVTL:
        index = 3;
        return CheckParam_Q8_LUL(index);

    default:
        return FAILED_PARAMVALID;
    }
}

//----------------------------------------------------------------------------------------------------
paramResult_t CheckParam_Q8_LUL(unsigned int index)
{
    ParamList_Q8_LUL[index].Proposed = _atoQ(valueBuf);
    volatile _q8 Test = ParamList_Q8_LUL[index].Proposed;
    volatile _q8 LLIM = ParamList_Q8_LUL[index].L_Lim;
    volatile _q8 ULIM = ParamList_Q8_LUL[index].U_Lim;
    if(Test>LLIM && Test<ULIM)
    {   return PASSED_PARAM;  }
    else
    {   return FAILED_PARAMVALID; }
}

//----------------------------------------------------------------------------------------------------
paramResult_t CheckParam_UINT_LUL(unsigned int index)
{
    ParamList_UINT_LUL[index].Proposed = AtoI(valueBuf);
    volatile unsigned int Test = ParamList_UINT_LUL[index].Proposed;
    volatile unsigned int LLIM = ParamList_UINT_LUL[index].L_Lim;
    volatile unsigned int ULIM = ParamList_UINT_LUL[index].U_Lim;
    if(Test>LLIM && Test<ULIM)
    {   return PASSED_PARAM;  }
    else
    {   return FAILED_PARAMVALID; }
}

//----------------------------------------------------------------------------------------------------
paramResult_t CheckParam_UINT_4OPTS(unsigned int index)
{
    unsigned int index2 = 0;
    volatile unsigned int Test = AtoI(valueBuf);

    for(index2=0; index2<4; index2++)
    {
        if(Test==ParamList_UINT_4OPTS[index].Options[index2])
        {   ParamList_UINT_4OPTS[index].Proposed = index2;
            return PASSED_PARAM;                              }
    }
    return FAILED_PARAMVALID;
}

//----------------------------------------------------------------------------------------------------
// A simple atoi() function
int AtoI(char* str)
{
    // Initialize result
    unsigned int res = 0;

    // Iterate through all characters of input string and update result
    // take ASCII character of corresponding digit and subtract the code from '0' to get numerical
    // value and multiply res by 10 to shuffle digits left to update running total
    unsigned int i=0;
    for (i = 0; str[i] != '\0'; ++i)
        res = res * 10 + str[i] - '0';

    // return result.
    return res;
}

