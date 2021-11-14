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
#include <Fault_Handler.h>

#pragma PERSISTENT(OVP_Latch);
#pragma PERSISTENT(OVP_Clear);
#pragma PERSISTENT(OVP_Pair);
//#pragma PERSISTENT(OVP_TripsCT);
//unsigned int OVP_TripsCT=0;
Qual_AFE_t OVP_Latch = {2, 0x00};
Qual_MCU_t OVP_Clear = {NEGATIVE, 0x2329, 0x2328  , 0, 20};
FaultPair_AFE_MCU_t OVP_Pair =  {CLEARED, &OVP_Latch, &OVP_Clear, 0, BIT2,
                                        0, 7, BiColor_GREEN};
