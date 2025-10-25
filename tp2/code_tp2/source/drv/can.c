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

#define RX_STATUS      0xB0u

#define RXLENGTH 13

#define	BUFFER_SIZE 50
#define OVERFLOW -1

#define ID_G1 0x101

uint16_t getID(uint8_t nm);

uint8_t CAN_readAdress(uint8_t adress);
uint8_t CAN_writeAdress(uint8_t adress, uint8_t* data, uint8_t n_bytes);

uint8_t readRxBuffer(uint8_t nm, uint8_t* data);

void loadTxBuffer(uint8_t abc, uint8_t* data, uint8_t n_bytes);
void request2Send(uint8_t tx);


void CAN_readBuffer(uint8_t buffer, uint8_t* data);

uint8_t CAN_writeAdress(uint8_t adress, uint8_t* data, uint8_t n_bytes)
{
	uint8_t aux [17];
	aux[0] = WRITE_INSTR;
	aux[1] = adress;

	int i;
	for(i = 0; i < n_bytes; i++)
	{
		aux[i+2] = data[i];
	}
	SPI0_sendNBytes(aux, i+2);
	SPI0_pushTxFIFO();
	while(!SPI0_isTxQueueEmpty());

	//flush rx buffer
	int j;
	for(j= 0; j < i+2; j++)
	{
		SPI0_PopRxFIFO();
	}
}

void loadTxBuffer(uint8_t abc, uint8_t* data, uint8_t n_bytes)
{
	uint8_t aux[17];

	aux[0] = 0b01000000 | abc;

	int i;
	for(i = 0; i < n_bytes; i++)
	{
		aux[i+1] = data[i];
	}
	SPI0_sendNBytes(aux, i+1);
	SPI0_pushTxFIFO();
	while(!SPI0_isTxQueueEmpty());

	//flush rx buffer
	int j;
	for(j= 0; j < i+1; j++)
	{
		SPI0_PopRxFIFO();
	}
}



uint8_t CAN_readAdress(uint8_t adress)
{
	SPI0_send3Bytes(READ_INSTRUCTION, adress, 0);
	SPI0_pushTxFIFO();
	while(!SPI0_isTxQueueEmpty());

	SPI0_PopRxFIFO();
	SPI0_PopRxFIFO();
	return SPI0_PopRxFIFO();
}

uint8_t getDLC(uint8_t buffer)
{
	uint8_t dlc =  0;

	switch(buffer)
	{
	case 0: dlc = CAN_readAdress(RXB0DLC); break;
	case 1: dlc = CAN_readAdress(RXB1DLC); break;
	default: break;
	}

	return dlc;
}

uint16_t getID(uint8_t nm)
{
	uint8_t aux[] = {0b10010000 | (nm << 1), 0, 0};
	SPI0_pushTxFIFO();
	while(!SPI0_isTxQueueEmpty());

	SPI0_PopRxFIFO();

	uint8_t hi = SPI0_PopRxFIFO();
	uint8_t low = SPI0_PopRxFIFO();

	uint16_t id = hi <<3 | (low >> 5 ) ; //gets id

	return id;
}
uint8_t readRxBuffer(uint8_t nm, uint8_t* data)
{
	uint8_t aux[17];
	aux[0] = 0b10010000 | (nm<<1);
	int i;

	uint8_t n_bytes = 0;
	switch(nm)
	{
	case 0b01: n_bytes= getDLC(0); break;
	case 0b11: n_bytes= getDLC(0); break;
	}


	for(i = 0; i < n_bytes; i++)
	{
		aux[i+1] = 0; //Empty array
	}

	SPI0_sendNBytes(aux, i + 1);
	SPI0_pushTxFIFO();
	while(!SPI0_isTxQueueEmpty());

	SPI0_PopRxFIFO();

	for(i= 0; i < n_bytes; i++)
	{
		data[i] = SPI0_PopRxFIFO();
	}
	return n_bytes;
}


uint8_t CAN_readData(uint8_t* data)
{
	//check buffer que se recibio
	uint8_t status = CAN_rxStatus();

	uint8_t buffer = 0;
	bool both_buffers = 0;

	if(!status)
	{
		return 0;
	}
	if(status == 1)
	{
		buffer =1;
	}
	if(status == 2)
	{
		buffer = 2;
	}
	if(status == 4)
	{
		both_buffers = 1;
	}


	uint8_t id = 0;
	//Validates id
	switch(buffer)
	{
	case 0: id =  getID(0); break;
	case 1: id = getID(0b10); break;
	default: break;
	}

	if(id == ID_G1) // ignore our grupo id
	{
		return 0;
	}
	uint8_t size = 0;
	//Reads buffer
	switch(buffer)
	{
	case 0: size = readRxBuffer(0b01 ,data);
	case 1: size = readRxBuffer(0b11 ,data);
	}

	return size;
}

uint8_t CAN_rxStatus(void)
{
    SPI0_send2Bytes(RX_STATUS, 0x00);
    SPI0_PopRxFIFO();                 // eco del cmd

    uint8_t s = (uint8_t) (SPI0_PopRxFIFO() & 0xFFu);          // status (LSB)

    /* bit6 -> RXB0, bit7 -> RXB1 */
    uint8_t r = 0;
    if (s & 0x40u)	//False ==> nothing received in RXB0
   {
    	r |= 0x01u;              // RXB0
   	}
    if (s & 0x80u)	//False ==> nothing received in RXB1
    {
    	r |= 0x02u;              // RXB1
    }
    return r;         // 0 ==>nada,
    				//3 =>> los dosh
}

void CAN_sendData(uint8_t* data, size_t n_bytes)
{
	flushTxFIFO();
	uint8_t aux[] = {0b00100000, 0b00100000 }; //G1 id and standard

	CAN_writeAdress(TXB0DLC, aux, 1); //specifies dlc

	loadTxBuffer(0, aux, 2);
	aux[0] = n_bytes;


	loadTxBuffer(0b001, data, n_bytes);
	request2Send(0);
}

void request2Send(uint8_t tx)
{
	switch(tx)
	{
	case 0: SPI0_sendByte(0b10000001); break;
	case 1: SPI0_sendByte(0b10000010); break;
	case 2: SPI0_sendByte(0b10000100); break;
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

		flushTxFIFO();
}

