/*----------------------------------------------------------------------------------------------------
 * Title: Persistent.c
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
#include <Persistent.h>
#include <Fault_Handler.h>
#include "Constants.h"

#pragma PERSISTENT(OVP_Latch);
#pragma PERSISTENT(OVP_Clear);
#pragma PERSISTENT(OVP_Pair);
Qual_AFE_t OVP_Latch = {2, 0x00};
Qual_MCU_t OVP_Clear = {NEGATIVE, 0x2329, 0x2328  , 0, 20};
FaultPair_AFE_MCU_t OVP_Pair =  {CLEARED, &OVP_Latch, &OVP_Clear, 0, BIT2,
                                        0, 7, BiColor_GREEN};
#pragma PERSISTENT(UVP_Latch);
#pragma PERSISTENT(UVP_Clear);
#pragma PERSISTENT(UVP_Pair);
Qual_AFE_t UVP_Latch = {3, 0x00};
Qual_MCU_t UVP_Clear = {POSITIVE, 0x1771, 0x1770, 0, 20};
FaultPair_AFE_MCU_t UVP_Pair =  {CLEARED, &UVP_Latch, &UVP_Clear, 0, BIT3,
                                        0, 7, BiColor_RED};

#pragma PERSISTENT(SCPD_Latch);
#pragma PERSISTENT(SCPD_Clear);
#pragma PERSISTENT(SCPD_Pair);
Qual_AFE_t SCPD_Latch = {0, 0x00};
Qual_AUR_t SCPD_Clear = {false, 0, 40, 0, 3, false};;
FaultPair_AFE_AUR_t SCPD_Pair =  {CLEARED, &SCPD_Latch, &SCPD_Clear, 0, BIT1,
                                         0, 6, BiColor_RED};

#pragma PERSISTENT(OCPD_Latch);
#pragma PERSISTENT(OCPD_Clear);
#pragma PERSISTENT(OCPD_Pair);
Qual_AFE_t OCPD_Latch = {0, 0x00};
Qual_AUR_t OCPD_Clear = {false, 0, 40, 0, 3, false};;
FaultPair_AFE_AUR_t OCPD_Pair =  {CLEARED, &OCPD_Latch, &OCPD_Clear, 0, BIT0,
                                         0, 5, BiColor_RED};

#pragma PERSISTENT(BCPD_Latch);
#pragma PERSISTENT(BCPD_Clear);
#pragma PERSISTENT(BCPD_Pair);
Qual_MCU_t BCPD_Latch = {NEGATIVE, 0x0000, BCPD_Thresh, 0, 4};
Qual_AUR_t BCPD_Clear = {false, 0, 40, 0, 3, false};
FaultPair_MCU_AUR_t BCPD_Pair =  {CLEARED, &BCPD_Latch, &BCPD_Clear, 0,
                                         0, 4, BiColor_RED};

#pragma PERSISTENT(MCPD_Latch);
#pragma PERSISTENT(MCPD_Clear);
#pragma PERSISTENT(MCPD_Pair);
Qual_MCU_t MCPD_Latch = {NEGATIVE, 0x0000, MCPD_Thresh, 0, 40};
Qual_AUR_t MCPD_Clear = {false, 0, 40, 0, 3, false};
FaultPair_MCU_AUR_t MCPD_Pair =  {CLEARED, &MCPD_Latch, &MCPD_Clear, 0,
                                         0, 3, BiColor_RED};

#pragma PERSISTENT(BCPC_Latch);
#pragma PERSISTENT(BCPC_Clear);
#pragma PERSISTENT(BCPC_Pair);
Qual_MCU_t BCPC_Latch = {POSITIVE, 0x0000, BCPC_Thresh, 0, 4};
Qual_AUR_t BCPC_Clear = {false, 0, 40, 0, 3, false};
FaultPair_MCU_AUR_t BCPC_Pair =  {CLEARED, &BCPC_Latch, &BCPC_Clear, 0,
                                         0, 4, BiColor_GREEN};

#pragma PERSISTENT(MCPC_Latch);
#pragma PERSISTENT(MCPC_Clear);
#pragma PERSISTENT(MCPC_Pair);
Qual_MCU_t MCPC_Latch = {POSITIVE, 0x0000, MCPC_Thresh, 0, 40};
Qual_AUR_t MCPC_Clear = {false, 0, 40, 0, 3, false};
FaultPair_MCU_AUR_t MCPC_Pair =  {CLEARED, &MCPC_Latch, &MCPC_Clear, 0, 3, BiColor_GREEN};
