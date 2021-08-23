/*----------------------------------------------------------------------------------------------------
 * Title: I2C_Handler.h
 * Authors: Nathaniel VerLee, 2020
 * Contributors: Ryan Heacock, Kurt Snieckus, 2020
 *
 * This file takes care of all I2C communication including initializations, reading and writing,
 * and any associated interrupt handler routines
----------------------------------------------------------------------------------------------------*/

#ifndef I2C_HANDLER_H
#define I2C_HANDLER_H

//----------------------------------------------------------------------------------------------------
// Function Prototypes
void Init_I2C();
void I2C_Write(unsigned char Addr, unsigned int Reg, unsigned int NumBytes);
void I2C_Read(unsigned char Addr, unsigned int Reg, unsigned int NumBytes);

//----------------------------------------------------------------------------------------------------
// Global Variables
extern unsigned char I2CTXBuf[16];
extern unsigned char I2CRXBuf[32];

#endif
