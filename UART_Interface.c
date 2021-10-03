/*
 * UART_Interface.c
 *
 *  Created on: Oct 3, 2021
 *      Author: cqc
 */

#include <msp430.h>
#include <System.h>

void printf(char *format, ...);

void putc(char d) {

    while(!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = d;                   // Load data into buffer
}

void puts(char* str) {
    while (str[0] != 0) {
        putc(str[0]);
        str++;
    }
}


void Init_UART(void)
{
    // Configure UART pins
    P1SEL0 |= BIT6 | BIT7;                    // set 2-UART pin as second function

    // Configure UART
    UCA0CTLW0 |= UCSWRST;                     // Put into reset
    UCA0CTLW0 |= UCSSEL_1;                    // set SMCLK as BRCLK

    // User Guide Table 22-5, Baud = 9600, BRCLK=SMCLK=1MHz,
    UCA0BR0 = 3;
    UCA0BR1 = 0x00;
    UCA0MCTLW = 0x9200;

    UCA0CTLW0 &= ~UCSWRST;                    // Initialize eUSCI
    UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt

    puts("\n\nHELLO BITCHES!!!\n");

    printf("%i... nice\n", 69);

}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A0_VECTOR))) USCI_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(UCA0IV,USCI_UART_UCTXCPTIFG))
  {
    case USCI_NONE: break;
    case USCI_UART_UCRXIFG:
      while(!(UCA0IFG&UCTXIFG));
      UCA0TXBUF = UCA0RXBUF;
      __no_operation();
      break;
    case USCI_UART_UCTXIFG: break;
    case USCI_UART_UCSTTIFG: break;
    case USCI_UART_UCTXCPTIFG: break;
    default: break;
  }
}




/******************************************************************************
 *                          Reusable MSP430 printf()
 *
 * Description: This printf function was written by oPossum and originally
 *              posted on the 43oh.com forums. For more information on this
 *              code, please see the link below.
 *
 *              http://www.43oh.com/forum/viewtopic.php?f=10&t=1732
 *
 *              A big thanks to oPossum for sharing such great code!
 *
 * Author:  oPossum
 * Source:  http://www.43oh.com/forum/viewtopic.php?f=10&t=1732
 * Date:    10-17-11
 *
 * Note: This comment section was written by Nicholas J. Conn on 06-07-2012
 *       for use on NJC's MSP430 LaunchPad Blog.
 ******************************************************************************/

#include <stdarg.h>

static const unsigned long dv[] = {
//  4294967296      // 32 bit unsigned max
        1000000000,// +0
        100000000, // +1
        10000000, // +2
        1000000, // +3
        100000, // +4
//       65535      // 16 bit unsigned max
        10000, // +5
        1000, // +6
        100, // +7
        10, // +8
        1, // +9
        };

static void xtoa(unsigned long x, const unsigned long *dp) {
    char c;
    unsigned long d;
    if (x) {
        while (x < *dp)
            ++dp;
        do {
            d = *dp++;
            c = '0';
            while (x >= d)
                ++c, x -= d;
            putc(c);
        } while (!(d & 1));
    } else
        putc('0');
}

static void puth(unsigned n) {
    static const char hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8',
            '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    putc(hex[n & 15]);
}

void printf(char *format, ...)
{
    char c;
    int i;
    long n;

    va_list a;
    va_start(a, format);
    while(c = *format++) {
        if(c == '%') {
            switch(c = *format++) {
                case 's': // String
                    puts(va_arg(a, char*));
                    break;
                case 'c':// Char
                    putc(va_arg(a, char));
                break;
                case 'i':// 16 bit Integer
                case 'u':// 16 bit Unsigned
                    i = va_arg(a, int);
                    if(c == 'i' && i < 0) i = -i, putc('-');
                    xtoa((unsigned)i, dv + 5);
                break;
                case 'l':// 32 bit Long
                case 'n':// 32 bit uNsigned loNg
                    n = va_arg(a, long);
                    if(c == 'l' && n < 0) n = -n, putc('-');
                    xtoa((unsigned long)n, dv);
                break;
                case 'x':// 16 bit heXadecimal
                    i = va_arg(a, int);
                    puth(i >> 12);
                    puth(i >> 8);
                    puth(i >> 4);
                    puth(i);
                break;
                case 0: return;
                default: goto bad_fmt;
            }
        } else
            bad_fmt: putc(c);
    }
    va_end(a);
}

