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
// Enumerations

//----------------------------------------------------------------------------------------------------
// Function Prototypes

//------------------------------------------------------------------------------------------
// Initializations and Checks
void Init_BMSConfig(void);
uint8_t Compose_Protect1();
uint8_t Compose_Protect2();
uint8_t Compose_Protect3();

void Init_BMSProtect(void);
bool Check_BMSConfig(void);
bool Check_BMSProtect(void);
//------------------------------------------------------------------------------------------
// Status Register
unsigned char Update_SysStat(void);
void Clear_SysStat(void);
void Clear_CCReady(void);

unsigned char GetByte_SysStat(void);
bool GetBit_CCReady(void);
bool GetBit_OvrdAlert(void);

void Clear_FaultBits(uint8_t bits);
bool Get_Fault(void);
bool Get_FaultBit(uint8_t bit);

//------------------------------------------------------------------------------------------
// Set CHG and DSG MOSFETs
void Set_CHG_DSG_Bits(uint8_t fetbits);

//------------------------------------------------------------------------------------------
// Cell and battery voltage registers
void Update_VCells(unsigned char Group);
unsigned int Get_VCell_ADC(unsigned char CellNum);
unsigned int Get_VCell_Max(void);
unsigned int Get_VCell_Min(void);
float Get_VCell_Dec(unsigned char CellNum);
void Update_VBatt(void);

//------------------------------------------------------------------------------------------
// Coulomb Counter registers
int Update_CCReg(void);
int Get_CCVal_ADC(void);
float Get_CCVal_Dec(void);

//------------------------------------------------------------------------------------------
// Temp Sensor registers
void Update_TSReg(void);
unsigned int GetNum_TS_Cnt(unsigned char TempNum);

//------------------------------------------------------------------------------------------
// Cell balance registers
void Set_CellBal(unsigned char Group, unsigned char Cell, bool Enable);

#endif
