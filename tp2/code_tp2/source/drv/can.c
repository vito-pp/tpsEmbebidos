/***************************************************************************//**
  @file     CAN_SPI.c

  @brief    CAN_sPI driver
  @author   Grupo 5
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "hardware.h"
#include "CAN_SPI.h"
#include "SPI.h"
#include "gpio.h"



/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define INT_PIN      16
#define INT_PORT     PC

#define	WRITE_INSTRUCTION 		0b00000010
#define	BIT_MODIFY_INSTRUCTION 	0b00000101
#define	READ_INSTRUCTION 		0b00000011

#define	CNF1_ADDRESS	0b00101010
#define	CNF2_ADDRESS	0b00101001
#define	CNF3_ADDRESS	0b00101000

#define RxM0SIDH		0b00100000
#define RxM0SIDL		0b00100001

#define RxF0SIDH		0b00000000
#define RxF0SIDL		0b00000001

#define	RxB0CTRL		0b01100000
#define RxB1CTRL		0b01110000

#define	CANINTE			0b00101011
#define	CANINTF			0b00101100

#define	CANCTRL			0b00001111

#define	TXB0CTRL		0b00110000
#define	TXB1CTRL		0b01000000
#define	TXB2CTRL		0b01010000

#define	TXB0SIDH		0b00110001
#define	TXB1SIDH		0b01000001
#define	TXB2SIDH		0b01010001

#define	TXB0SIDL		0b00110010
#define	TXB1SIDL		0b01000010
#define	TXB2SIDL		0b01010010

#define	TXB0DLC			0b00110101
#define	TXB1DLC			0b01000101
#define	TXB2DLC			0b01010101

#define	RXB0D0			0b01100110
#define	RXB1D0			0b01110110

#define RXLENGTH		13

#define	BUFFER_SIZE	 50
#define OVERFLOW     -1

typedef struct spican_buffer{
	RXB_RAWDATA_t *pin;
	RXB_RAWDATA_t *pout;
	RXB_RAWDATA_t buffer[BUFFER_SIZE];
	uint8_t num_Of_Words;
}spican_buffer_t;

spican_buffer_t can_spi_buffers[1];

static uint8_t	transmitting = 0;
static uint8_t	receiving = 0;


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
*******************************************************************************/
/**
 * @brief Initializes the circular queue
 */
static void queue_Init (uint8_t id);


/**
 * @brief Pushes an event to the queue
 * @param event The element to add to the queue
 * @return Number of pending events. Returns value OVERFLOW if the maximun number of events is reached
 */
static int8_t push_Queue_Element(uint8_t id, RXB_RAWDATA_t event);


/**
 * @brief Pulls the earliest event from the queue
 * @return Event_Type variable with the current event if no OVERFLOW is detected.
 */
static RXB_RAWDATA_t pull_Queue_Element(uint8_t id);


/**
 * @brief Gets the status of the queue
 * @return Returns the number of pending events in the queue
 */
static uint8_t get_Queue_Status(uint8_t id);


/**
 * @brief
 * @return
 */
static void load_TX_buffer(uint8_t abc, uint8_t * bytes, uint8_t num_bytes, void (*cb)(void));

/**
 * @brief
 * @return
 */
static void write_SPICAN(uint8_t address, uint8_t * bytes_data, uint8_t num_bytes_data, void (*cb)(void));

/**
 * @brief
 * @return
 */
static void read_SPICAN(uint8_t address, uint8_t num_bytes_to_read, void (*cb)(void));


/**
 * @brief
 * @return
 */
static void read_RX_buffer(uint8_t nm, uint8_t num_bytes_to_read, void (*cb)(void));

/**
 * @brief
 * @return
 */
static void read_status(void (*cb)(void));


/**
 * @brief
 * @return
 */
static void RTS(uint8_t txn, void (*cb)(void));

static void parseData(RXB_RAWDATA_t * rawdata, uint8_t *data_to_send);


/**
 * @brief
 * @param
 * @return
 */

static void checkDoubleBuffers(void);


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/



/**
 * @brief
 * @param
 * @return
 */

void CAN_SPI_Init (void)
{
	// 1- SPI in mode 0,0
	SPI_Init();

	queue_Init(0);
	// Set interrupt pin for RX interrupt detection
	//gpioMode (PORTNUM2PIN(INT_PORT,INT_PIN), INPUT_PULLDOWN);
	//gpioIRQ_Config (PORTNUM2PIN(INT_PORT, INT_PIN), PORT_eInterruptFalling);

	gpioMode (PORTNUM2PIN(INT_PORT,INT_PIN+1), INPUT_PULLDOWN);


	// 2- Reset mode
	uint8_t data[10];
	data[0] = 0b11000000;
	SPI_SendData(data, 1, 0);
	while(SPI_Transmission_In_Process());


	// 3- Bit time configuration
	data[0] = WRITE_INSTRUCTION;
	data[1] = CNF1_ADDRESS;
	data[2] = 0b00000011; // SJW = 1TQ; BRP = 4 (3+1)
	SPI_SendData(data, 3, 0);
	while(SPI_Transmission_In_Process());


	data[0] = WRITE_INSTRUCTION;
	data[1] = CNF2_ADDRESS;
	data[2] = 0b10110001; // btl=1; sam=0; phseg = 7 (6+1); prseg = 2 (1+1)
	SPI_SendData(data, 3, 0);
	while(SPI_Transmission_In_Process());

	data[0] = WRITE_INSTRUCTION;
	data[1] = CNF3_ADDRESS;
	data[2] = 0b10000101; // sof = 1; WAKFL = 0; PHSEG2 = 6 (5+1)
	SPI_SendData(data, 3, 0);
	while(SPI_Transmission_In_Process());


	// 4- Filter configurations
	data[0] = WRITE_INSTRUCTION;
	data[1] = RxM0SIDH;
	data[2] = 0b11111111;
	SPI_SendData(data, 3, 0);
	while(SPI_Transmission_In_Process());

	data[0] = WRITE_INSTRUCTION;
	data[1] = RxM0SIDL;
	data[2] = 0b00000000;
	SPI_SendData(data, 3, 0);
	while(SPI_Transmission_In_Process());

	data[0] = WRITE_INSTRUCTION;
	data[1] = RxF0SIDH;
	data[2] = 0b00100000;
	SPI_SendData(data, 3, 0);
	while(SPI_Transmission_In_Process());

	data[0] = WRITE_INSTRUCTION;
	data[1] = RxF0SIDL;
	data[2] = 0b00000000;
	SPI_SendData(data, 3, 0);
	while(SPI_Transmission_In_Process());

	// 5- Reception configurations
	data[0] = WRITE_INSTRUCTION;
	data[1] = RxB0CTRL;
	data[2] = 0b01100000; //0b00000100;//Roll-over enabled (RX0 TO RX1)
	SPI_SendData(data, 3, 0);
	while(SPI_Transmission_In_Process());

	data[0] = WRITE_INSTRUCTION;
	data[1] = RxB1CTRL;
	data[2] = 0b01100000;
	SPI_SendData(data, 3, 0);
	while(SPI_Transmission_In_Process());


	// 6- Erase flags and enable interrupts
	data[0] = WRITE_INSTRUCTION;
	data[1] = CANINTE;
	data[2] = 0b00000011;
	SPI_SendData(data, 3, 0);
	while(SPI_Transmission_In_Process());

	data[0] = WRITE_INSTRUCTION;
	data[1] = CANINTF;
	data[2] = 0b00000000;	// all flags in 0
	SPI_SendData(data, 3, 0);
	while(SPI_Transmission_In_Process());

	// 7- Normal mode
	data[0] = BIT_MODIFY_INSTRUCTION;
	data[1] = CANCTRL;
	data[2] = 0b11100000; // Mask
	data[3] = 0b00000000; //Loopback 0b01000000, NORMAL = 0b00000000
	SPI_SendData(data, 4, 0);
	while(SPI_Transmission_In_Process());

}


/**
 * @brief
 * @param
 * @return
 */

uint8_t CAN_SPI_Is_Read_Ready(void)
{
	return(get_Queue_Status(0) && !receiving);
}

uint8_t CAN_SPI_Is_Busy(void)
{
	return receiving || transmitting;
}

/**
 * @brief
 * @param
 * @return
 */

RXB_RAWDATA_t CAN_SPI_Get_Data(void)
{
	return(pull_Queue_Element(0));
}

/**
 * @brief
 * @param
 * @return
 */

uint8_t CAN_SPI_Attempt_to_read(void)
{
	uint8_t flag = 1;
	if(!gpioRead(PORTNUM2PIN(INT_PORT,INT_PIN+1)) && !receiving && !transmitting)
	{
		CAN_SPI_ReceiveInfo();
		flag = 0;
	}
	return flag;
}


/**
 * @brief
 * @param
 * @return
 */

void CAN_SPI_ReceiveInfo(void)
{
	gpioWrite(CAN_TP_PIN,HIGH);
	static uint8_t receiveState = 0;
	static uint8_t aux[16];
	static uint8_t intStatus = 0;


	switch(receiveState)
	{
	case 0:
		//while(receiving);
		receiving = 1;
		read_SPICAN(CANINTF, 1, &CAN_SPI_ReceiveInfo);
		receiveState = 2;
		break;

	case 1:
		SPI_Get_DataBytes(aux, 3);
		intStatus = aux[2];
		//aux[0] = intStatus & 0b11111100;

		//write_SPICAN(CANINTF, aux, 1, &CAN_SPI_ReceiveInfo);
		// clear interrupt flags
		receiveState = 2;
		break;

	case 2:
		SPI_Get_DataBytes(aux, 3);
		intStatus = aux[2];
		// Check which RX received data
		if((intStatus & 0b11) == 0b11)
		{
			// Read RX 0 first, then RX1
			read_RX_buffer(0b00, 13, &CAN_SPI_ReceiveInfo);
			receiveState = 4;
		}
		else if((intStatus & 0b11) == 0b01)
		{
			// Read RX 0
			read_RX_buffer(0b00, 13, &CAN_SPI_ReceiveInfo);
			receiveState = 3;
		}
		else if((intStatus & 0b11) == 0b10)
		{
			// Read RX 1
			read_RX_buffer(0b10, 13, &CAN_SPI_ReceiveInfo);
			receiveState = 3;
		}
		break;


	case 3:
		checkDoubleBuffers();
		// Can data started to be sent
		uint8_t temp[1] = {0};
		// clear flags
		write_SPICAN(CANINTF, temp, 1, &CAN_SPI_ReceiveInfo);
		receiveState = 5;
		break;

	case 4:
		// read RX1, after reading RX1, if both flags on
		read_RX_buffer(0b10, 13, &CAN_SPI_ReceiveInfo);
		receiveState = 3;
		break;

	case 5:
		receiveState = 0;
		receiving = 0;
		break;

	default:
		receiveState = 0;
		receiving = 0;
		break;


	}
	gpioWrite(CAN_TP_PIN,LOW);
}


/**
 * @brief
 * @param
 * @return
 */

void CAN_SPI_SendInfo(RXB_RAWDATA_t * rawdata)
{
	gpioWrite(CAN_TP_PIN,HIGH);
	// Initial Send Info state
	static uint8_t sendState = 0;
	static uint8_t aux[16];
	static uint8_t data_to_send[16];
	static uint8_t number_buffer = 0;
	static uint8_t rxStatus;

	switch(sendState)
	{
	case 0:
		transmitting = 1;
		sendState = 1;
		parseData(rawdata, data_to_send);
		read_status(&CAN_SPI_SendInfo);
		break;

	case 1:
		SPI_Get_DataBytes(aux, 3);
		rxStatus = aux[1];

		if(!(rxStatus & 0b00000100))	// if TX
		{
			sendState = 2;
			load_TX_buffer(0b000000000, data_to_send, 13, &CAN_SPI_SendInfo);
			number_buffer = 0;
		}
		else if(!(rxStatus & 0b00010000))
		{
			sendState = 2;
			load_TX_buffer(0b000000010, data_to_send, 13, &CAN_SPI_SendInfo);
			number_buffer = 1;
		}
		else if(!(rxStatus & 0b01000000))
		{
			sendState = 2;
			load_TX_buffer(0b000000100, data_to_send, 13, &CAN_SPI_SendInfo);
			number_buffer = 2;
		}
		else
		{
			sendState = 0;
			transmitting = 0;
		}

		break;

	case 2:
		sendState = 3;
		RTS(number_buffer, &CAN_SPI_SendInfo);
		break;

	case 3:
		// Can data started to be sent
		sendState = 0;
		transmitting = 0;
		break;

	default:
		sendState = 0;
		transmitting = 0;
		break;


	}
	gpioWrite(CAN_TP_PIN,LOW);
}




/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
/**
 * @brief
 * @return
 */
static void load_TX_buffer(uint8_t abc, uint8_t * bytes_data, uint8_t num_bytes_data, void (*cb) (void))
{
	uint8_t aux[16];
	aux[0] = 0b01000000 | abc;
	uint32_t i;
	for(i = 0; i < num_bytes_data; i++)
	{
		aux[i+1] = bytes_data[i];
	}
	SPI_SendData(aux, num_bytes_data + 1, cb);
}

/**
 * @brief
 * @return
 */
static void write_SPICAN(uint8_t address, uint8_t * bytes_data, uint8_t num_bytes_data, void (*cb) (void))
{
	uint8_t aux[16];
	aux[0] = WRITE_INSTRUCTION;
	aux[1] = address;
	uint32_t i;
	for(i = 0; i < num_bytes_data; i++)
	{
		aux[i+2] = bytes_data[i];
	}
	SPI_SendData(aux, num_bytes_data + 2, cb);
}

/**
 * @brief
 * @return
 */
static void read_SPICAN(uint8_t address, uint8_t num_bytes_to_read, void (*cb) (void))
{
	uint8_t aux[16];
	aux[0] = READ_INSTRUCTION;
	aux[1] = address;
	uint32_t i;
	for(i = 0; i < num_bytes_to_read; i++)
	{
		aux[i+2] = 0;
	}
	SPI_SendData(aux, num_bytes_to_read + 2, cb);
}

/**
 * @brief
 * @return
 */
static void read_RX_buffer(uint8_t nm, uint8_t num_bytes_to_read, void (*cb) (void))
{
	uint8_t aux[16];
	aux[0] = 0b10010000 | nm;
	uint32_t i;
	for(i = 0; i < num_bytes_to_read; i++)
	{
		aux[i+1] = 0;
	}
	SPI_SendData(aux, num_bytes_to_read + 1, cb);
}

/**
 * @brief
 * @return
 */
static void read_status(void (*cb) (void))
{
	uint8_t aux[16];
	aux[0] = 0b10100000;
	uint32_t i;
	for(i = 0; i < 2; i++)
	{
		aux[i+1] = 0;
	}
	SPI_SendData(aux, 3, cb);
}

/**
 * @brief
 * @return
 */
static void RTS(uint8_t txn, void (*cb) (void))
{
	uint8_t aux[16];
	aux[0] = 0b10000000 | (1 << txn);
	SPI_SendData(aux, 1, cb);
}



/**
 * @brief
 * @return
 */
static void parseData(RXB_RAWDATA_t * rawdata, uint8_t *data_to_send)
{
	data_to_send[0] = (rawdata->SID >> 3);
	data_to_send[1] = (rawdata->SID << 5) & 0b11100000;
	data_to_send[2] = 0; // EID not used
	data_to_send[3] = 0;
	data_to_send[4] = rawdata->DLC;
	uint32_t i = 0;
	for(i = 0; i < 8; i++)
	{
		data_to_send[i+5] = rawdata->Dn[i];
	}
}

/**
 * @brief
 * @param
 * @return
 */

static void checkDoubleBuffers(void)
{
	uint8_t aux[16];
	RXB_RAWDATA_t received_data;

	uint32_t i = 0;

	for(i = 0; i < SPI_Get_Status() / RXLENGTH; i++)
	{
		// read
		SPI_Get_DataBytes(aux, 14);
		//
		received_data.SID = (((uint16_t)aux[1]) << 3) | ((aux[2] & 0b11100000)>> 5);
		received_data.DLC = aux[5] & 0b00001111;
		uint32_t j = 0;
		for(j = 0; j < received_data.DLC; j++)
		{
			received_data.Dn[j] = aux[j + 6];
		}

		push_Queue_Element(0, received_data);
	}

}


__ISR__ PORTC_IRQHandler(void)
{
	PORTC->PCR[INT_PIN] |= PORT_PCR_ISF_MASK;
	CAN_SPI_ReceiveInfo();
}




///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Initializes the circular queue
 */
static void queue_Init (uint8_t id)
{
	can_spi_buffers[id].pin = can_spi_buffers[id].buffer;
	can_spi_buffers[id].pout = can_spi_buffers[id].pin;
	can_spi_buffers[id].num_Of_Words = 0;
}

/**
 * @brief Pushes an event to the queue
 * @param event The element to add to the queue
 * @return Number of pending events. Returns value OVERFLOW if the maximun number of events is reached
 */
static int8_t push_Queue_Element(uint8_t id, RXB_RAWDATA_t event)
{
	// Check for EventQueue Overflow
	if (can_spi_buffers[id].num_Of_Words > BUFFER_SIZE-1)
	{
		return OVERFLOW;
	}

	*(can_spi_buffers[id].pin)++ = event;
	can_spi_buffers[id].num_Of_Words++;

	// Return pointer to the beginning if necessary
	if (can_spi_buffers[id].pin == BUFFER_SIZE + can_spi_buffers[id].buffer)
	{
		can_spi_buffers[id].pin = can_spi_buffers[id].buffer;
	}

	return can_spi_buffers[id].num_Of_Words;

}

/**
 * @brief Pulls the earliest event from the queue
 * @return Event_Type variable with the current event if no OVERFLOW is detected.
 */


static RXB_RAWDATA_t pull_Queue_Element(uint8_t id)
{
	RXB_RAWDATA_t event = *(can_spi_buffers[id].pout);

	if (can_spi_buffers[id].num_Of_Words == 0)
	{
		RXB_RAWDATA_t event1 = {0,2};
		return event1;
	}

	can_spi_buffers[id].num_Of_Words--;
	can_spi_buffers[id].pout++;

	if (can_spi_buffers[id].pout == BUFFER_SIZE + can_spi_buffers[id].buffer)
	{
		can_spi_buffers[id].pout = can_spi_buffers[id].buffer;
	}

	return event;

}


/**
 * @brief Gets the status of the queue
 * @return Returns the number of pending events in the queue
 */
static uint8_t get_Queue_Status(uint8_t id)
{
	return can_spi_buffers[id].num_Of_Words;
}


/**
 * @brief Pulls the earliest event from the queue
 * @return Event_Type variable with the current event if no OVERFLOW is detected.
 */


static void flush_Queue(uint8_t id)
{
	can_spi_buffers[id].pout = can_spi_buffers[id].pin;
	can_spi_buffers[id].num_Of_Words = 0;
}



/*******************************************************************************
 ******************************************************************************/
