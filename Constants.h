/*----------------------------------------------------------------------------------------------------
 * Title: BatteryData.h
 * Authors: Nathaniel VerLee, 2020
 * Contributors: Ryan Heacock, Kurt Snieckus, 2020
 *
 * This file contains all of the register and other constants used by the system
----------------------------------------------------------------------------------------------------*/

#ifndef CONSTANTS_H
#define CONSTANTS_H

//----------------------------------------------------------------------------------------------------
// Constants
#define I2C_BQ769xxADDR         0x18
//----------------------------------
#define SETUP_SYS_CTRL1         0x10    //ADC Enabled
#define SETUP_SYS_CTRL2         0x40    //Coulomb Counter Enabled
#define SETUP_SYS_CTRL2_CHG_ON          0x41
#define SETUP_SYS_CTRL2_CHG_DSG_ON      0x43
#define SETUP_SYS_CTRL2_DSG_ON          0x42
#define SETUP_SYS_CTRL2_CHG_DSG_OFF     0x40

//----------------------------------
#define SETUP_PROTECT1          0x8D    //SCD=155A, 100uS
#define SETUP_PROTECT2          0x4F    //OCD=100A, 160uS
#define SETUP_PROTECT3          0xB0    //OV in 8S, UV in 8S
#define SETUP_OV_TRIP           0xFF    //Over Voltage Trip Threshold
#define SETUP_UV_TRIP           0x01    //Under Voltage Trip Threshold
//----------------------------------
#define REG_SYS_STAT            0x00    //CC_READY, RSVD, DEVICE_XREADY, OVRD_ALERT, UV, OV, SCD, OCD
#define REG_CEL_BAL1            0x01    //RSVD, RSVD, RSVD, CB <5:1>
#define REG_CEL_BAL2            0x02    //RSVD, RSVD, RSVD, CB <6:10>
#define REG_CEL_BAL3            0x03    //RSVD, RSVD, RSVD, CB <11:15>
#define REG_SYS_CTRL1           0x04    //LOAD_PRESANT, RSVDx2, ADC_EN, TEMP_SEL, RSVD, SHUT_A, SHUT_B
#define REG_SYS_CTRL2           0x05    //DELAY_DIS, CC_EN, CC_ONESHOT, RSVDx3, DSG_ON, CHG_ON
#define REG_PROTECT1            0x06    //RSNS, RSVDx3, SCD_DELAY (4:3), SCD_THRESH(2:0)
#define REG_PROTECT2            0x07    //RSVD, OCD_DELAY (6:4), OCD_THRESH (3:0)
#define REG_PROTECT3            0x08    //UV_DELAY (7:6), OV_DELAY (5:4), RSVDx4
#define REG_OV_TRIP             0x09    //OV_THRESH (7:0)
#define REGUV_TRIP              0x0A    //UV_THRESH (7:0)
//----------------------------------
#define REG_VCELL1              0x0C
#define REG_VCELL2              0x0E
#define REG_VCELL3              0x10
#define REG_VCELL4              0x12
#define REG_VCELL5              0x14
//----------------------------------
#define REG_VCELL6              0x16
#define REG_VCELL7              0x18
#define REG_VCELL8              0x1A
#define REG_VCELL9              0x1C
#define REG_VCELL10             0x1E
//----------------------------------
#define REG_VCELL11             0x20
#define REG_VCELL12             0x22
#define REG_VCELL13             0x24
#define REG_VCELL14             0x26
#define REG_VCELL15             0x28
//----------------------------------
#define REG_VBATT               0x2A
#define REG_TS1                 0x2C
#define REG_TS2                 0x2E
#define REG_TS3                 0x30
#define REG_CCREG               0x32
//----------------------------------
#define REG_ADCGAIN1            0x50
#define REG_ADCOFFSET           0x51
#define REG_ADCGAIN2            0x59


#endif
