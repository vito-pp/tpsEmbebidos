#ifndef UART_H_
#define UART_H_
#include "MK64F12.h"

#define UART_HAL_DEFAULT_BAUDRATE 1200

#define UART0_TX_PIN 	17   //PTB17
#define UART0_RX_PIN 	16   //PTB16

typedef enum
{
	PORT_mAnalog,
	PORT_mGPIO,
	PORT_mAlt2,
	PORT_mAlt3,
	PORT_mAlt4,
	PORT_mAlt5,
	PORT_mAlt6,
	PORT_mAlt7,

} PORTMux_t;

typedef enum
{
	PORT_eDisabled				= 0x00,
	PORT_eDMARising				= 0x01,
	PORT_eDMAFalling			= 0x02,
	PORT_eDMAEither				= 0x03,
	PORT_eInterruptDisasserted	= 0x08,
	PORT_eInterruptRising		= 0x09,
	PORT_eInterruptFalling		= 0x0A,
	PORT_eInterruptEither		= 0x0B,
	PORT_eInterruptAsserted		= 0x0C,
} PORTEvent_t;

typedef enum {
    UART_PARITY_NONE = 0,
    UART_PARITY_EVEN,
    UART_PARITY_ODD
} uart_parity_t;

void UART_Init (char parity);
void UART_SetParity(UART_Type *uart, uart_parity_t parity);
void UART_SetBaudRate (UART_Type *uart, uint32_t baudrate);

void UART_Send_Data(unsigned char txdata);
unsigned char UART_Recieve_Data();



#endif /* UART_H_ */
