#ifndef UART_H_
#define UART_H_
#include "MK64F12.h"

void UART_Init (void);
void UART_SetBaudRate (UART_Type *uart, uint32_t baudrate);

void UART_Send_Data(unsigned char txdata);
unsigned char UART_Recieve_Data();

#endif /* UART_H_ */
