/*
 * can.c
 *
 *  Created on: 20 oct. 2025
 *      Author: Usuario
 */
#include <stdio.h>
#include "can.h"
#include "spi.h"

#define	CNF1_ADDRESS 0b00101010
#define	CNF2_ADDRESS 0b00101001
#define	CNF3_ADDRESS 0b00101000

#define RxM0SIDH 0b00100000
#define RxM0SIDL 0b00100001

#define RxF0SIDH 0b00000000
#define RxF0SIDL 0b00000001

#define	RxB0CTRL 0b01100000
#define RxB1CTRL 0b01110000

#define	CANINTE	0b00101011
#define	CANINTF	0b00101100

#define	CANCTRL	0b00001111

#define	TXB0CTRL 0b00110000
#define	TXB1CTRL 0b01000000
#define	TXB2CTRL 0b01010000

#define	TXB0SIDH 0b00110001
#define	TXB1SIDH 0b01000001
#define	TXB2SIDH 0b01010001

#define	TXB0SIDL 0b00110010
#define	TXB1SIDL 0b01000010
#define	TXB2SIDL 0b01010010

#define	TXB0DLC	0b00110101
#define	TXB1DLC	0b01000101
#define	TXB2DLC	0b01010101

#define RXB0DLC 0b01100101
#define RXB1DLC 0b01110101

#define	RXB0D0 0b01100110
#define	RXB1D0 0b01110110

#define RXLENGTH 13

#define	BUFFER_SIZE 50
#define OVERFLOW -1



void CAN_readBuffer(uint8_t buffer, uint8_t* data)
{
	SPI0_send3Bytes(READ_INSTRUCTION, RXB0DLC, 0);
	SPI0_pushTxFIFO();
	while(!SPI0_isTxQueueEmpty());

	SPI0_PopRxFIFO();
	SPI0_PopRxFIFO();
	uint8_t n_bytes = SPI0_PopRxFIFO();


	uint8_t aux[17];
	aux[0] = 0b10010000 | buffer;
	int i;
	for(i = 0; i < n_bytes; i++)
	{
		aux[i+1] = 0;
	}
	SPI0_sendNBytes(aux, i + 1);
	SPI0_pushTxFIFO();
	while(!SPI0_isTxQueueEmpty());

	SPI0_PopRxFIFO();

	for(i= 0; i < n_bytes; i++)
	{
		data[i] = SPI0_PopRxFIFO();
	}

}

void CAN_sendData(uint8_t* data, size_t n_bytes, uint8_t id)
{
	//Load Tx buffer
	uint8_t aux[17];
	aux[0] = 0b01000000 | id;

	int i;
	for(i = 0; i < n_bytes; i++)
	{
		aux[i+1] = data[i];
	}
	SPI0_sendNBytes(aux, i + 1);
	SPI0_pushTxFIFO();
	while(!SPI0_isTxQueueEmpty());

	//flush rx buffer
	int j;
	for(j= 0; j < i+1; j++)
	{
		SPI0_PopRxFIFO();
	}



	//REQUEST TO SEND
	switch(id)
	{
		case 0: SPI0_sendByte(0b10000001); break;//last 3 bits T2 T1 T0 to tx
		default: break;
	}

	SPI0_pushTxFIFO();
	while(!SPI0_isTxQueueEmpty());

	SPI0_PopRxFIFO();

}


void CAN_Init(void)
{
		SPI0Master_Init();

		SPI0_sendByte(RESET_INSTR);
		SPI0_pushTxFIFO();
		while(!SPI0_isTxQueueEmpty());


		SPI0_send3Bytes(WRITE_INSTR, CNF1_ADDRESS, 3);
		SPI0_pushTxFIFO();
		while(!SPI0_isTxQueueEmpty());

		SPI0_send3Bytes(WRITE_INSTR, CNF2_ADDRESS, 0b10110001);
		SPI0_pushTxFIFO();
		while(!SPI0_isTxQueueEmpty());

		SPI0_send3Bytes(WRITE_INSTR, CNF3_ADDRESS, 0b10000101);
		SPI0_pushTxFIFO();
		while(!SPI0_isTxQueueEmpty());

		SPI0_send3Bytes(WRITE_INSTR, RxM0SIDH, 0b11111111);
		SPI0_pushTxFIFO();
		while(!SPI0_isTxQueueEmpty());

		SPI0_send3Bytes(WRITE_INSTR, RxM0SIDL, 0);
		SPI0_pushTxFIFO();
		while(!SPI0_isTxQueueEmpty());

		SPI0_send3Bytes(WRITE_INSTR, RxF0SIDH, 0b00100000);
		SPI0_pushTxFIFO();
		while(!SPI0_isTxQueueEmpty());

		SPI0_send3Bytes(WRITE_INSTR, RxF0SIDL, 0);
		SPI0_pushTxFIFO();
		while(!SPI0_isTxQueueEmpty());

		SPI0_send3Bytes(WRITE_INSTR, RxB0CTRL, 0b01100000);
		SPI0_pushTxFIFO();
		while(!SPI0_isTxQueueEmpty());

		SPI0_send3Bytes(WRITE_INSTR, RxB1CTRL, 0b01100000);
		SPI0_pushTxFIFO();
		while(!SPI0_isTxQueueEmpty());

		SPI0_send3Bytes(WRITE_INSTR, CANINTE, 0b00000011);
		SPI0_pushTxFIFO();
		while(!SPI0_isTxQueueEmpty());

		SPI0_send3Bytes(WRITE_INSTR, CANINTF, 0);
		SPI0_pushTxFIFO();
		while(!SPI0_isTxQueueEmpty());

		SPI0_send4Bytes(BIT_MODIFY_INSTRUCTION, CANCTRL, 0b11100000, 0);
		SPI0_pushTxFIFO();
		while(!SPI0_isTxQueueEmpty());
}

