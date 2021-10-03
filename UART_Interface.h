/*
 * UART_Interface.h
 *
 *  Created on: Oct 3, 2021
 *      Author: cqc
 */

#ifndef UART_INTERFACE_H_
#define UART_INTERFACE_H_


void Init_UART(void);
void putc(char);
void puts(char*);
void printf(char *format, ...);

#endif /* UART_INTERFACE_H_ */
