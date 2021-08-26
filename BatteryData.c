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
#include <stdbool.h>
#include "Constants.h"
#include "I2C_Handler.h"
#include "BatteryData.h"

//----------------------------------------------------------------------------------------------------
// Constants
const unsigned char NUMCELLS = 8;

//----------------------------------------------------------------------------------------------------
// Enumerations and Defines
enum CellGroup {GroupNull=0, GroupA=1, GroupB=2, GroupC=3 };
#define ONEGROUP 10

//----------------------------------------------------------------------------------------------------
// Variables
unsigned char StatReg;
unsigned int CellADCVals[15];
unsigned char CellIndex=0;

void Set_CHG_On(void)
{
    I2CTXBuf[0]=SETUP_SYS_CTRL2_DSG_ON;
    I2C_Write(I2C_BQ769xxADDR, REG_SYS_CTRL2, 1);
}
void Set_CHD_DSG_On(void)
{
    I2CTXBuf[0]=SETUP_SYS_CTRL2_CHG_DSG_ON ;
    I2C_Write(I2C_BQ769xxADDR, REG_SYS_CTRL2, 1);
}
void Set_DSG_On(void)
{
    I2CTXBuf[0]=SETUP_SYS_CTRL2_CHG_ON;
    I2C_Write(I2C_BQ769xxADDR, REG_SYS_CTRL2, 1);
}
void Set_CHG_DSG_Off(void)
{
    I2CTXBuf[0]=SETUP_SYS_CTRL2_CHG_DSG_OFF;
    I2C_Write(I2C_BQ769xxADDR, REG_SYS_CTRL2, 1);
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
// Configure the BQ769x0 in the desired manner and confirm
void Init_BMSConfig(void)
{
    I2CTXBuf[0]=SETUP_SYS_CTRL2;
    I2C_Write(I2C_BQ769xxADDR, REG_SYS_CTRL2, 1);           //Enable Coulomb Counting and Alert
    I2C_Read(I2C_BQ769xxADDR, REG_SYS_CTRL2, 1);            //Confirm Proper Sys Config

    Update_SysStat();
    Clear_SysStat();
}

//----------------------------------------------------------------------------------------------------
bool GetBit_CCReady(void)
{   return (StatReg & BIT7) != 0;   }

//----------------------------------------------------------------------------------------------------
bool Get_Fault(void)
{   return ((StatReg & (BIT5+BIT3+BIT2+BIT1+BIT0)) != 0);     }
//wow, the parents around statreg and the bits matters....

//----------------------------------------------------------------------------------------------------
bool GetBit_OV(void)
{   return (StatReg & BIT2) != 0;   }

//----------------------------------------------------------------------------------------------------
bool GetBit_UV(void)
{   return (StatReg & BIT3) != 0;   }

//----------------------------------------------------------------------------------------------------
bool GetBit_OCD(void)
{   return (StatReg & BIT0) != 0;   }

//----------------------------------------------------------------------------------------------------
bool GetBit_SCD(void)
{   return (StatReg & BIT1) != 0;   }


//----------------------------------------------------------------------------------------------------
unsigned char GetByte_SysStat()
{   return StatReg; }

//----------------------------------------------------------------------------------------------------
// Update Cell Voltages from I2C Buffer
void Update_VCells(unsigned char Group)
{
    char GroupReg;

    switch (Group)
    {
        case GroupA:
        {
            CellIndex=0;
            GroupReg=REG_VCELL1;
            break;
        }
        case GroupB:
        {
            CellIndex=5;
            GroupReg=REG_VCELL6;
            break;
        }
        case GroupC:
        {
            CellIndex=10;
            GroupReg=REG_VCELL11;
            break;
        }
        case GroupNull:
        {
            break;
        }
        default:
        {
            break;
        }
    }

    I2C_Read(I2C_BQ769xxADDR, GroupReg, ONEGROUP);

    CellADCVals[CellIndex] = (I2CRXBuf[0] << 8) + I2CRXBuf[1];
    CellADCVals[CellIndex+1] = (I2CRXBuf[2] << 8) + I2CRXBuf[3];
    CellADCVals[CellIndex+2] = (I2CRXBuf[4] << 8) + I2CRXBuf[5];
    CellADCVals[CellIndex+3] = (I2CRXBuf[6] << 8) + I2CRXBuf[7];
    CellADCVals[CellIndex+4] = (I2CRXBuf[8] << 8) + I2CRXBuf[9];
}

//----------------------------------------------------------------------------------------------------
//Get
void Set_CellBal(unsigned char Group, unsigned char Cell, bool Enable)
{
    char GroupReg;

    switch (Group)
    {
        case GroupA:
        {
            GroupReg=REG_CEL_BAL1;
            break;
        }
        case GroupB:
        {
            GroupReg=REG_CEL_BAL2;
            break;
        }
        case GroupC:
        {
            GroupReg=REG_CEL_BAL3;
            break;
        }
        case GroupNull:
        {
            break;
        }
        default:
        {
            break;
        }
    }

    if(Enable==true)
    {
        I2CTXBuf[0]=0x01;
    }
    if(Enable==false)
    {
        I2CTXBuf[0]=0x00;
    }
    I2C_Write(I2C_BQ769xxADDR, GroupReg, ONEGROUP);
}

//----------------------------------------------------------------------------------------------------
// Get Cell Voltage in ADC Counts
unsigned int Get_VCell_ADC(unsigned char CellNum)
{
    return CellADCVals[CellNum];
}

//----------------------------------------------------------------------------------------------------
// Compute Cell Voltage in Decimal
float Get_VCell_Dec(unsigned char CellNum)
{
    return CellADCVals[CellNum]*0.000382;
}
