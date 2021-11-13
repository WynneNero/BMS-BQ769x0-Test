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

#include <msp430.h>
#include <stdbool.h>
#include <stdint.h>

//----------------------------------------------------------------------------------------------------
// Function Prototypes
void Init_I2C();
bool I2C_Write(uint8_t Addr, uint8_t CtrlReg, uint8_t NumBytes);
bool I2C_Read(uint8_t Addr, uint8_t CtrlReg, uint8_t NumBytes);
bool I2C_Read_Ctrl2(uint8_t Addr, uint8_t CtrlReg, uint8_t CtrlReg2, uint8_t NumBytes);

//----------------------------------------------------------------------------------------------------
// Global Variables
extern unsigned char I2CTXBuf[16];
extern unsigned char I2CRXBuf[32];

#endif
