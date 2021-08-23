/*----------------------------------------------------------------------------------------------------
 * Title: I2C_Handler.c
 * Authors: Nathaniel VerLee, 2020
 * Contributors: Ryan Heacock, Kurt Snieckus, 2020
 *
 * This file takes care of all I2C communication including initializations, reading and writing,
 * and any associated interrupt handler routines
----------------------------------------------------------------------------------------------------*/

//----------------------------------------------------------------------------------------------------
// This file includes:
#include <msp430.h>
#include "I2C_Handler.h"
#include "Constants.h"

//----------------------------------------------------------------------------------------------------
//Variables
unsigned int TXByteCnt = 0;
unsigned int TXByteSize = 0;
unsigned char TXReg = 0;
unsigned char I2CTXBuf[16];

unsigned int RXByteCnt = 0;
unsigned int RXByteSize = 0;
unsigned char I2CRXBuf[32];

//----------------------------------------------------------------------------------------------------
//Function prototypes
//void Init_GPIO();
void Init_I2C();
void I2C_Write(unsigned char Addr, unsigned int Reg, unsigned int NumBytes);
void I2C_Read(unsigned char Addr, unsigned int Reg, unsigned int NumBytes);

//----------------------------------------------------------------------------------------------------
// I2C Read Function
// Function takes the address to write to, the control register to write, and the number of bytes
// following the control register to transmit from the TX Buffer Array
void I2C_Write(unsigned char Addr, unsigned int CtrlReg, unsigned int NumDataBytes)
{
    TXByteCnt = NumDataBytes+1;             // Increment by 1 to keep proper count in ISR
    TXByteSize = NumDataBytes+1;            // Increment by 1 to keep proper count in ISR
    UCB0CTLW0 |= UCTR;                  // I2C Transmit Mode
    UCB0CTL1 |= UCTXSTT;                // I2C start condition
    TXReg = CtrlReg;

    __bis_SR_register(LPM0_bits|GIE);   // Enter LPM0 w/ interrupt
}

//----------------------------------------------------------------------------------------------------
// I2C Write Function
// Function takes the address to read from, the control register to read from, and the number of
// bytes to read back into the RX Buffer
void I2C_Read(unsigned char Addr, unsigned int CtrlReg, unsigned int NumDataBytes)
{
    I2C_Write(Addr, CtrlReg, 0);

    RXByteCnt = NumDataBytes-1;             // Decrement by 1 to keep proper RX Buffer index
    RXByteSize = NumDataBytes-1;            // Decrement by 1 to keep proper RX Buffer index
    UCB0CTLW0 &= ~UCTR;                     // I2C Recieve Mode
    UCB0CTL1 |= UCTXSTT;                    // I2C start condition
    if(NumDataBytes==1)
    {
        UCB0CTLW0 |= UCTXSTP;
    }

    __bis_SR_register(LPM0_bits|GIE);       // Enter LPM0 w/ interrupt

}

//----------------------------------------------------------------------------------------------------
void Init_I2C()
{
    // Configure USCI_B0 for I2C Master mode
    UCB0CTLW0 |= UCSWRST;                       // Software reset enabled
    UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC;     // I2C mode, Master mode, sync
    //UCB0CTLW1 |= UCASTP_2;                    // Automatic stop generated after UCB0TBCNT is reached
    UCB0BRW = 0x0008;                           // baudrate = SMCLK / 8
    //UCB0TBCNT = 0x0002;                       // number of bytes to be received
    UCB0I2CSA = I2C_BQ769xxADDR;                // Slave address
    UCB0CTLW0 |= UCTR;                          //I2C Transmit Mode
    UCB0CTL1 &= ~UCSWRST;
    UCB0IE |= UCTXIE | UCRXIE | UCNACKIE | UCBCNTIE;
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
          __bic_SR_register_on_exit(LPM0_bits); // Exit LPM0
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

    case USCI_I2C_UCRXIFG0:                 // Vector 24: RXIFG0
        if(RXByteCnt>1)
        {
            I2CRXBuf[RXByteSize-RXByteCnt] = UCB0RXBUF;
            RXByteCnt-=1;
        }
        else if(RXByteCnt==1)
        {
            I2CRXBuf[RXByteSize-RXByteCnt] = UCB0RXBUF;
            RXByteCnt-=1;
            UCB0CTLW0 |= UCTXSTP;                  // I2C stop condition
        }
        else if(RXByteCnt==0)
        {
            // Get RX data
            I2CRXBuf[RXByteSize-RXByteCnt] = UCB0RXBUF;
            __bic_SR_register_on_exit(LPM0_bits); // Exit LPM0
        }
        break;

    case USCI_I2C_UCTXIFG0:                 // Vector 26: TXIFG0
        if(TXByteCnt==TXByteSize)
        {
            UCB0TXBUF = TXReg;
            TXByteCnt-=1;
        }
        if (TXByteCnt>1)
        {
            UCB0TXBUF = I2CTXBuf[(TXByteSize-TXByteCnt)-1];
            TXByteCnt-=1;
        }
        if (TXByteCnt==1)
        {
            UCB0TXBUF = I2CTXBuf[(TXByteSize-TXByteCnt)-1];
            TXByteCnt=0;
        }
        else if(TXByteCnt==0)
        {
            UCB0CTLW0 |= UCTXSTP;           // I2C stop condition
            __bic_SR_register_on_exit(LPM0_bits);
        }
        break;

    case USCI_I2C_UCBCNTIFG:                // Vector 28: BCNTIFG
        P1OUT ^= BIT0;                        // Toggle LED on P1.0
        break;

    case USCI_I2C_UCCLTOIFG: break;         // Vector 30: clock low timeout
    {
          UCB0CTLW0 |= UCTXSTP;                     // I2C stop condition
          __bic_SR_register_on_exit(LPM0_bits); // Exit LPM0
          break;
    }
    case USCI_I2C_UCBIT9IFG: break;         // Vector 32: 9th bit
    default: break;
  }
}
