/*----------------------------------------------------------------------------------------------------
 * Title: I2C_Handler.c
 * Authors: Nathaniel VerLee, 2020
 * Contributors: Ryan Heacock, Kurt Snieckus, 2020
 *
 * This file takes care of all I2C communication including initializations, reading and writing,
 * and any associated interrupt handler routines
----------------------------------------------------------------------------------------------------*/

//----------------------------------------------------------------------------------------------------
//Includes
#include <msp430.h>
#include <stdbool.h>
#include <stdint.h>
#include "Constants.h"
#include "I2C_Handler.h"

//----------------------------------------------------------------------------------------------------
//Variables

unsigned char I2CTXBuf[16];
unsigned char I2CRXBuf[32];

uint8_t TXByte_CT = 0;
uint8_t TXBuf_INDEX = 0;
uint8_t RXByte_CT = 0;
uint8_t RXBuf_INDEX = 0;
uint8_t NextTXReg = 0;

bool I2CBusy = true;

//----------------------------------------------------------------------------------------------------
//Enumerations
typedef enum I2CMode_enum{
    IDLE_MODE,
    NACK_MODE,
    TX_REG_ADDRESS_MODE,
    RX_REG_ADDRESS_MODE,
    TX_DATA_MODE,
    RX_DATA_MODE,
    SWITCH_TO_RX_MODE,
    SWITCH_TO_TX_MODE,
    TIMEOUT_MODE
} I2CMode_t;

I2CMode_t I2CMode = IDLE_MODE;

//----------------------------------------------------------------------------------------------------
void Init_I2C()
{
    // Configure USCI_B0 for I2C Master mode
    UCB0CTLW0 |= UCSWRST;                       // Software reset enabled
    UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC;     // I2C mode, Master mode, sync
    //UCB0CTLW1 |= UCASTP_2;                    // Automatic stop generated after UCB0TBCNT is reached
    UCB0BRW = 0x0008;                           // baudrate = SMCLK / 10
    UCB0I2CSA = I2C_BQ769xxADDR;                // Slave address
    UCB0CTL1 &= ~UCSWRST;                       // Clear software reset
    UCB0IE |=  UCNACKIE;                        // Enable UCB0 Interrupt
}

//----------------------------------------------------------------------------------------------------
// I2C Write Function
// Function takes the address to read from, the control register to read from, and the number of
// bytes to read back into the RX Buffer
bool I2C_Write(uint8_t Addr, uint8_t CtrlReg, uint8_t NumBytes)
{
    //Setup TX mode and capture control/register byte
    I2CMode=TX_REG_ADDRESS_MODE;
    NextTXReg = CtrlReg;

    //Setup all the counts for TX Mode
    TXByte_CT = NumBytes;
    TXBuf_INDEX = 0;
    RXByte_CT = 0;
    RXBuf_INDEX = 0;

    //Setup the I2C Peripheral for transmitting
    UCB0I2CSA = Addr;
    UCB0CTLW0 |= UCTR;                      // I2C Transmit Mode
    UCB0IFG &= ~(UCTXIFG + UCRXIFG);        // Clear any pending interrupts
    UCB0IE &= ~UCRXIE;                      // Disable RX interrupt
    UCB0IE |= UCTXIE;                       // Enable TX interrupt

    //Set I2C Busy flag and begin transmission
    I2CBusy = true;
    UCB0CTL1 |= UCTXSTT;                // I2C start condition

    //Going to try not going to sleep this time around, leave this commented out
    __bis_SR_register(GIE);   // General interrupt enable
    //Instead processor is awake and idle until message is sent.
    while(I2CBusy)
    {   __no_operation();   }

    //For now just return true, in the future return false if there were issues with the transmission
    return true;
}

//----------------------------------------------------------------------------------------------------
// I2C Read Function
// Function takes the address to write to, the control register to write, and the number of bytes
// following the control register to transmit from the TX Buffer Array
bool I2C_Read(uint8_t Addr, uint8_t CtrlReg, uint8_t NumBytes)
{
    return false;
}

//----------------------------------------------------------------------------------------------------
// I2C Interrupt Vector and associated flags
#pragma vector = USCI_B0_VECTOR
__interrupt void USCIB0_ISR(void)

{
    switch(__even_in_range(UCB0IV, USCI_I2C_UCBIT9IFG))
    {
    case USCI_NONE: break;                  // Vector 0: No interrupts
    case USCI_I2C_UCALIFG: break;           // Vector 2: ALIFG
    case USCI_I2C_UCNACKIFG:                // Vector 4: NACKIFG
    {
          UCB0CTLW0 |= UCTXSTP;                     // I2C stop condition
          //I2CBusy = false;
          //__bic_SR_register_on_exit(LPM0_bits); // Exit LPM0
          break;
    }

    case USCI_I2C_UCSTTIFG: break;          // Vector 6: STTIFG
    case USCI_I2C_UCSTPIFG: break;          // Vector 8: STPIFG
    case USCI_I2C_UCRXIFG3: break;          // Vector 10: RXIFG3
    case USCI_I2C_UCTXIFG3: break;          // Vector 14: TXIFG3
    case USCI_I2C_UCRXIFG2: break;          // Vector 16: RXIFG2
    case USCI_I2C_UCTXIFG2: break;          // Vector 18: TXIFG2
    case USCI_I2C_UCRXIFG1: break;          // Vector 20: RXIFG1
    case USCI_I2C_UCTXIFG1: break;          // Vector 22: TXIFG1

    //------------------------------------------------------------------------------------------
    //Deal with RX Interrupt
    case USCI_I2C_UCRXIFG0:                 // Vector 24: RXIFG0
        break;

    //------------------------------------------------------//----------------------------------
    //Deal with TX Interrupt
    case USCI_I2C_UCTXIFG0:                                 // Vector 26: TXIFG0

        switch(I2CMode)
        {
            case TX_REG_ADDRESS_MODE:
                UCB0TXBUF = NextTXReg;
                I2CMode = TX_DATA_MODE;
                break;
            case TX_DATA_MODE:
                if(TXByte_CT)
                {
                    UCB0TXBUF = I2CTXBuf[TXBuf_INDEX];
                    TXBuf_INDEX++;
                    TXByte_CT--;
                }
                else
                {
                    UCB0CTLW0 |= UCTXSTP;                   // Send stop condition
                    I2CMode = IDLE_MODE;
                    UCB0IE &= ~UCTXIE;                      // disable TX interrupt
                    I2CBusy = false;
                }
            default:
                __no_operation();
                break;
        }
        break;

    case USCI_I2C_UCBCNTIFG:                // Vector 28: BCNTIFG
        //P1OUT ^= BIT0;                        // Toggle LED on P1.0
        break;

    case USCI_I2C_UCCLTOIFG: break;         // Vector 30: clock low timeout
    {
          UCB0CTLW0 |= UCTXSTP;                     // I2C stop condition
          //__bic_SR_register_on_exit(LPM0_bits); // Exit LPM0
          //I2CBusy = false;
          break;
    }
    case USCI_I2C_UCBIT9IFG: break;         // Vector 32: 9th bit
    default: break;
    }
}
