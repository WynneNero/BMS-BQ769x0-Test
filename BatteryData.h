/*----------------------------------------------------------------------------------------------------
 * Title: BatteryData.h
 * Authors: Nathaniel VerLee, 2020
 * Contributors: Ryan Heacock, Kurt Snieckus, 2020
 *
 * This file organizes the structure of data for the battery system
 * and provides appropriate functions for accessing and modifying it
----------------------------------------------------------------------------------------------------*/

#ifndef BATDATA_H
#define BATDATA_H

//----------------------------------------------------------------------------------------------------
// This file includes:
#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>
#include <Constants.h>

//----------------------------------------------------------------------------------------------------
// Function Prototypes
//----------------------------------
// Initializations and Checks
void Init_BMSConfig(void);
void Init_BMSProtect(void);
bool Check_BMSConfig(void);
bool Check_BMSProtec(void);
//----------------------------------
// Status Register
unsigned char Update_SysStat(void);
void Set_CHG_On(void);
void Set_CHG_DSG_On(void);
void Set_DSG_On(void);
void Set_CHG_DSG_Off(void);
void Clear_SysStat(void);
void Clear_CCReady(void);
unsigned char GetByte_SysStat(void);
bool GetBit_CCReady(void);
bool GetBit_OvrdAlert(void);
bool Get_Fault(void);
bool Get_FaultBit(uint8_t bit);
bool GetBit_UV(void);
bool GetBit_OV(void);
bool GetBit_SCD(void);
bool GetBit_OCD(void);
//----------------------------------
// Set CHG and DSG MOSFETs


//----------------------------------
// Cell voltage registers
void Update_VCells(unsigned char Group);
unsigned int Get_VCell_ADC(unsigned char CellNum);
float Get_VCell_Dec(unsigned char CellNum);
//----------------------------------
// Battery voltage register
void Update_VBatt(void);


//----------------------------------
// Temp Sensor registers
void Update_TSReg(void);
unsigned int GetNum_TS_Cnt(unsigned char TempNum);
//----------------------------------
// Coulomb Counter registers
void Update_CCReg(void);
int Get_CCVal_ADC(void);
float Get_CCVal_Dec(void);

//----------------------------------
// Cell balance registers
void Set_CellBal(unsigned char Group, unsigned char Cell, bool Enable);

//----------------------------------------------------------------------------------------------------
// Global Variables

#endif
