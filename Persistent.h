/*----------------------------------------------------------------------------------------------------
 * Title: Persistent.h
 * Authors: Nathaniel VerLee, Matthew Pennock, 2020-2021
 * Contributors: Ryan Heacock, Kurt Snieckus, Matthew Pennock, 2020-2021
 *
 * This file is the header for persistent.c which stores  variable that need to be stored in FRAM
 * and not be re-initialized to zero during a system startup.
----------------------------------------------------------------------------------------------------*/

#ifndef PERSISTENT_H
#define PERSISTENT_H

#include <msp430.h>
#include <stdbool.h>
#include <stdint.h>
#include <System.h>
#include <Fault_Handler.h>

extern Qual_AFE_t OVP_Latch;            //Change to OVPR (Over Voltage PRotection)
extern Qual_MCU_t OVP_Clear;
extern FaultPair_AFE_MCU_t OVP_Pair;

extern Qual_AFE_t UVP_Latch;            //Change to UVPR (Under Voltage PRotection)
extern Qual_MCU_t UVP_Clear;
extern FaultPair_AFE_MCU_t UVP_Pair;

extern Qual_AFE_t SCPD_Latch;
extern Qual_AUR_t SCPD_Clear;
extern FaultPair_AFE_AUR_t SCPD_Pair;

extern Qual_AFE_t OCPD_Latch;
extern Qual_AUR_t OCPD_Clear;
extern FaultPair_AFE_AUR_t OCPD_Pair;

extern Qual_MCU_t BCPD_Latch;
extern Qual_AUR_t BCPD_Clear;
extern FaultPair_MCU_AUR_t BCPD_Pair;

extern Qual_MCU_t MCPD_Latch;
extern Qual_AUR_t MCPD_Clear;
extern FaultPair_MCU_AUR_t MCPD_Pair;

extern Qual_MCU_t BCPC_Latch;
extern Qual_AUR_t BCPC_Clear;
extern FaultPair_MCU_AUR_t BCPC_Pair;

extern Qual_MCU_t MCPC_Latch;
extern Qual_AUR_t MCPC_Clear;
extern FaultPair_MCU_AUR_t MCPC_Pair;

#endif /* PERSISTENT_H */
