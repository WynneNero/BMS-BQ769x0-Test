/*----------------------------------------------------------------------------------------------------
 * Title: BatteryData.c
 * Authors: Nathaniel VerLee, 2020
 * Contributors: Ryan Heacock, Kurt Snieckus, 2020
 *
 * This file organizes the structure of data for the battery system
 * and provides appropriate functions for accessing and modifying it
----------------------------------------------------------------------------------------------------*/

//----------------------------------------------------------------------------------------------------
// This file includes:
#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>
#include "Constants.h"
#include "I2C_Handler.h"
#include "BatteryData.h"
#include "UART_Interface.h"

//----------------------------------------------------------------------------------------------------
// Constants
const unsigned char NUMCELLS = 8;

//----------------------------------------------------------------------------------------------------
// Enumerations and Defines
enum CellGroup {GroupNull=0, GroupA=1, GroupB=2, GroupC=3 };
#define ONEGROUP 10
// This array defines which cells actually contain valid data and the number of cells in use:
const uint8_t NumPositions = 10;
const uint8_t NumCells = 8;
// Cell AFE Position:      CELL1 CELL2 CELL3 CELL4  CELL5 CELL6 CELL7 CELL8 CELL9  CELL10
const bool CellActive[] = {true, true, true, false, true, true, true, true, false, true};
//const unsigned int



//----------------------------------------------------------------------------------------------------
// Variables
unsigned char StatReg;
unsigned int CellADCVals[15];
signed int CCVal = 0;
unsigned char CellIndex=0;

void Set_CHG_DSG_Bits(uint8_t fetbits)
{
    //fetbits&=!(BIT7+BIT6+BIT5+BIT4+BIT3+BIT2); // Mask all bits off except

    I2CTXBuf[0]=SETUP_SYS_CTRL2_CHG_DSG_OFF|fetbits;
    I2C_Write(I2C_BQ769xxADDR, REG_SYS_CTRL2, 1);
}

//----------------------------------------------------------------------------------------------------
// Configure the BQ769x0 in the desired manner and confirm
void Init_BMSConfig(void)
{
    I2CTXBuf[0]=SETUP_SYS_CTRL2;
    I2C_Write(I2C_BQ769xxADDR, REG_SYS_CTRL2, 1);           //Enable Coulomb Counting and Alert
    I2C_Read(I2C_BQ769xxADDR, REG_SYS_CTRL2, 1);            //Confirm Proper Sys Config

    I2CTXBuf[0]=SETUP_PROTECT1;
    I2CTXBuf[1]=SETUP_PROTECT2;
    I2CTXBuf[2]=SETUP_PROTECT3;
    //I2CTXBuf[3]=SETUP_OV_TRIP;
    //I2CTXBuf[4]=SETUP_UV_TRIP;
    I2C_Write(I2C_BQ769xxADDR, REG_PROTECT1, 3);           //Setup OCP and SCP Thresholds

    Update_SysStat();
    Clear_SysStat();
}


//----------------------------------------------------------------------------------------------------
// Update Status Register
unsigned char Update_SysStat(void)
{
    I2C_Read(I2C_BQ769xxADDR, REG_SYS_STAT, 1);
    StatReg = I2CRXBuf[0];
    return StatReg;
}

//----------------------------------------------------------------------------------------------------
// Clear the Status Register by setting all bit that were set to 1 back to 1
// (Which will actually clear them, somewhat confusing)
void Clear_SysStat(void)
{
    I2CTXBuf[0]=StatReg;
    I2C_Write(I2C_BQ769xxADDR, REG_SYS_STAT, 1);     //Clear the System Status Register
}

//----------------------------------------------------------------------------------------------------
// Clear just the CCReady bit in the Status Register by setting only that bit to 1
// (Which will actually clear it, somewhat confusing)
void Clear_CCReady(void)
{
    I2CTXBuf[0]=BIT7;
    I2C_Write(I2C_BQ769xxADDR, REG_SYS_STAT, 1);     //Clear the System Status Register
}



//----------------------------------------------------------------------------------------------------
bool GetBit_CCReady(void)
{   return (StatReg & BIT7) != 0;   }

//----------------------------------------------------------------------------------------------------
//bool Get_Fault(void)
//{   return ((StatReg & (BIT5+BIT3+BIT2+BIT1+BIT0)) != 0);     }
//wow, the parens around statreg and the bits matters....

//----------------------------------------------------------------------------------------------------
bool Get_FaultBit(uint8_t bit)
{   return ((StatReg&0x0F) & (0x01<<bit));    }

//----------------------------------------------------------------------------------------------------
void Clear_FaultBits(uint8_t bits)
{
    I2CTXBuf[0]=bits;
    I2C_Write(I2C_BQ769xxADDR, REG_SYS_STAT, 1);
}

//----------------------------------------------------------------------------------------------------
unsigned char GetByte_SysStat()
{   return StatReg; }

//----------------------------------------------------------------------------------------------------
// Update Cell Voltages from I2C Buffer
void Update_VCells(unsigned char Group)
{
    unsigned int GroupReg;

    switch (Group)
    {
        case GroupA:
        {   CellIndex=0;
            GroupReg=REG_VCELL1;
            break;              }
        case GroupB:
        {   CellIndex=5;
            GroupReg=REG_VCELL6;
            break;              }
        case GroupC:
        {   CellIndex=10;
            GroupReg=REG_VCELL11;
            break;              }
        case GroupNull:
        {   break;              }
        default:
        {   break;              }
    }

    I2C_Read(I2C_BQ769xxADDR, GroupReg, ONEGROUP);

    CellADCVals[CellIndex] = (I2CRXBuf[0] << 8) + I2CRXBuf[1];
    CellADCVals[CellIndex+1] = (I2CRXBuf[2] << 8) + I2CRXBuf[3];
    CellADCVals[CellIndex+2] = (I2CRXBuf[4] << 8) + I2CRXBuf[5];
    CellADCVals[CellIndex+3] = (I2CRXBuf[6] << 8) + I2CRXBuf[7];
    CellADCVals[CellIndex+4] = (I2CRXBuf[8] << 8) + I2CRXBuf[9];

    //printf("CellADCVals: "); unsigned int i=0; for (i=0; i<NUMCELLS; i++) printf("%i,", CellADCVals[i]); printf("\n");

}

//----------------------------------------------------------------------------------------------------
//Get
void Set_CellBal(unsigned char Group, unsigned char Cell, bool Enable)
{
    char GroupReg;

    switch (Group)
    {
        case GroupA:
        {   GroupReg=REG_CEL_BAL1;
            break;              }
        case GroupB:
        {   GroupReg=REG_CEL_BAL2;
            break;              }
        case GroupC:
        {   GroupReg=REG_CEL_BAL3;
            break;              }
        case GroupNull:
        {   break;              }
        default:
        {   break;              }
    }

    if(Enable==true)
    {   I2CTXBuf[0]=0x01;       }
    if(Enable==false)
    {   I2CTXBuf[0]=0x00;       }
    I2C_Write(I2C_BQ769xxADDR, GroupReg, ONEGROUP);
}

//----------------------------------------------------------------------------------------------------
// Get Cell Voltage in ADC Counts
unsigned int Get_VCell_ADC(unsigned char CellNum)
{
    return CellADCVals[CellNum];
}

//----------------------------------------------------------------------------------------------------
// Get Cell Voltage in ADC Counts
unsigned int Get_VCell_Max(void)
{
    unsigned int Max=0;
    unsigned int CT=0;

    for(CT=0; CT<NumPositions; CT++)
    {
        if(CellActive[CT]==true)
        {
            if(CellADCVals[CT]>Max)
            {   Max=CellADCVals[CT];    }
        }
    }
    return Max;
}

//----------------------------------------------------------------------------------------------------
// Get Cell Voltage in ADC Counts
unsigned int Get_VCell_Min(void)
{
    unsigned int Min=9999;
    unsigned int CT=0;

    for(CT=0; CT<NumPositions; CT++)
    {
        if(CellActive[CT]==true)
        {
            if(CellADCVals[CT]<Min)
            {   Min=CellADCVals[CT];    }
        }
    }
    return Min;
}

//----------------------------------------------------------------------------------------------------
// Compute Cell Voltage in Decimal
float Get_VCell_Dec(unsigned char CellNum)
{
    return CellADCVals[CellNum]*0.000382;
}

//----------------------------------------------------------------------------------------------------
int Update_CCReg(void)
{
    I2C_Read(I2C_BQ769xxADDR, REG_CCREG, 2);
    CCVal = (I2CRXBuf[0] << 8) + I2CRXBuf[1];
    //if(1<<15&CCVal)
    //{   CCVal=-1*((~CCVal)+1);      }
    return CCVal;
}

//----------------------------------------------------------------------------------------------------
int Get_CCVal_ADC(void)
{
    return CCVal;
}
