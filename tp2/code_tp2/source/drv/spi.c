/***************************************************************************//**
  @file     SPI.c

  @brief    SPI driver
  @author   Grupo 5
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "hardware.h"
#include "spi.h"
#include "gpio.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define IS_RFDF(tmp)	((tmp) & SPI_SR_RFDF_MASK)
#define IS_TFFF(tmp)	((tmp) & SPI_SR_TFFF_MASK)
#define IS_EOQF(tmp)	((tmp) & SPI_SR_EOQF_MASK)
#define	IS_TCF(tmp)		((tmp) & SPI_SR_TCF_MASK)

#define SPI0_PCS_PIN	0 //PTD0
#define SPI0_SCK_PIN	1 //PTD1
#define SPI0_TX_PIN 	2 //PTD2
#define SPI0_RX_PIN 	3 //PTD3

#define	BUFFER_SIZE	 50
#define OVERFLOW     -1

typedef struct buffer_element{
	uint8_t data;
	uint8_t end_of_data; //last byte if value is 1
}buffer_element_t;

typedef struct spi_buffer{
	buffer_element_t *pin;
	buffer_element_t *pout;
	buffer_element_t buffer[BUFFER_SIZE];
	uint8_t num_Of_Words;
}spi_buffer_t;

spi_buffer_t spi_buffers[2]; //doubles the value for input and output buffers. 1 is input, 0 is output.

static uint32_t delay = 0;
static uint8_t delay_flag = 0;

static uint8_t transfer_complete = 0;

static void (*externalCB)(void);


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
static int8_t push_Queue_Element(uint8_t id, buffer_element_t event);


/**
 * @brief Pulls the earliest event from the queue
 * @return Event_Type variable with the current event if no OVERFLOW is detected.
 */
static buffer_element_t pull_Queue_Element(uint8_t id);


/**
 * @brief Gets the status of the queue
 * @return Returns the number of pending events in the queue
 */
static uint8_t get_Queue_Status(uint8_t id);

/**
 * @brief
 * @return
 */
static void SPI_timer(void);

static void flush_Queue(uint8_t id);
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

void SPI_Init (void)
{

// Note: 5.6 Clock Gating page 192
// Any bus access to a peripheral that has its clock disabled generates an error termination.
		SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;

	    SIM->SCGC6 |= SIM_SCGC6_SPI0_MASK;
	    SIM->SCGC6 |= SIM_SCGC6_SPI1_MASK;
	    SIM->SCGC3 |= SIM_SCGC3_SPI2_MASK;



		NVIC_EnableIRQ(SPI0_IRQn);
		NVIC_EnableIRQ(SPI1_IRQn);
		NVIC_EnableIRQ(SPI2_IRQn);

		//Configure SPI0 PINS

		PORTD->PCR[SPI0_TX_PIN]=0x0; //Clear all bits
		PORTD->PCR[SPI0_TX_PIN]|=PORT_PCR_MUX(PORT_mAlt2); 	 //Set MUX to SPI0
		PORTD->PCR[SPI0_TX_PIN]|=PORT_PCR_IRQC(PORT_eDisabled); //Disable interrupts
//----------------------------------------------------------------------------------
		PORTD->PCR[SPI0_RX_PIN]=0x0; //Clear all bits
		PORTD->PCR[SPI0_RX_PIN]|=PORT_PCR_MUX(PORT_mAlt2); 	 //Set MUX to SPI0
		PORTD->PCR[SPI0_RX_PIN]|=PORT_PCR_IRQC(PORT_eDisabled); //Disable interrupts

//----------------------------------------------------------------------------------
		PORTD->PCR[SPI0_PCS_PIN]=0x0; //Clear all bits
		PORTD->PCR[SPI0_PCS_PIN]|=PORT_PCR_MUX(PORT_mAlt2); 	 //Set MUX to SPI0
		PORTD->PCR[SPI0_PCS_PIN]|=PORT_PCR_IRQC(PORT_eDisabled); //Disable interrupts
//----------------------------------------------------------------------------------
		PORTD->PCR[SPI0_SCK_PIN]=0x0; //Clear all bits
		PORTD->PCR[SPI0_SCK_PIN]|=PORT_PCR_MUX(PORT_mAlt2); 	 //Set MUX to SPI0
		PORTD->PCR[SPI0_SCK_PIN]|=PORT_PCR_IRQC(PORT_eDisabled); //Disable interrupts

	//Enable SPI0 Xmiter and Rcvr

		SPI0->MCR = 0x0;
		// Chip select is active Low for Can Controller
		SPI0->MCR = SPI_MCR_MSTR_MASK | SPI_MCR_PCSIS_MASK;

		SPI0->CTAR[0] = 0x0;
		SPI0->CTAR[0] = SPI_CTAR_FMSZ(7) | SPI_CTAR_PBR_MASK | SPI_CTAR_BR(6) | SPI_CTAR_DT(0b00001000) | SPI_CTAR_CSSCK(0b00000111) | SPI_CTAR_ASC(0b00000111); //insert 10 usec in between transfers

		SPI0->SR = 0x0;
		SPI0->SR = SPI_SR_TXRXS_MASK;

		// Enable transmissions
		//SPI0->MCR &= ~SPI_MCR_HALT_MASK;

		queue_Init(0);
		queue_Init(1);
}


/**
 * @brief
 * @param
 * @return
 */

uint8_t SPI_Get_Status(void)
{
	gpioWrite(CAN_TP_PIN,HIGH);
	return(get_Queue_Status(1));
	gpioWrite(CAN_TP_PIN,LOW);
}

/**
 * @brief
 * @param
 * @return
 */

uint8_t SPI_Get_Data(void)
{
	gpioWrite(CAN_TP_PIN,HIGH);
	return(pull_Queue_Element(1).data);
	gpioWrite(CAN_TP_PIN,LOW);
}

/**
 * @brief
 * @param
 * @return
 */

void SPI_Get_DataBytes(uint8_t *data, uint32_t num_of_bytes)
{
	gpioWrite(CAN_TP_PIN,HIGH);
	uint32_t i;
	for(i = 0; i < num_of_bytes; i++)
	{
		data[i] = pull_Queue_Element(1).data;
	}
	gpioWrite(CAN_TP_PIN,LOW);
}

/**
 * @brief	Starts the Transmission of the only byte (8-bits word).
 * @param	byte	Data byte to transmit.
 */
void SPI_SendByte(uint8_t byte)
{
	gpioWrite(CAN_TP_PIN,HIGH);
	buffer_element_t event = {byte, 1};
	push_Queue_Element(0, event);

	buffer_element_t buffer_data_out = pull_Queue_Element(0);
	uint32_t data_out = SPI_PUSHR_CONT_MASK;
	if (buffer_data_out.end_of_data)
	{
		// If last word, indicate that the chip select should not be held
		data_out &= ~SPI_PUSHR_CONT_MASK;
	}
	// Select chip_select 0
	data_out |= SPI_PUSHR_PCS(1);
	data_out |= SPI_PUSHR_TXDATA(buffer_data_out.data);

	transfer_complete = 0;
	// Enable TCF flag to start pushing data to the queue
	SPI0->RSER |= SPI_RSER_TCF_RE_MASK;

	SPI0 -> PUSHR = data_out;
	gpioWrite(CAN_TP_PIN,LOW);

}

/**
 * @brief	Starts the Transmission of the string (8-bits words). The '\0' is not transmitted
 * @param	msg	Array of data (string)
 */

void SPI_SendMsg(uint8_t* msg)
{
	gpioWrite(CAN_TP_PIN,HIGH);
	uint32_t i = 0;
	while (msg[i]  != '\0')
	{
		uint8_t end_of_data = 0;
		if(msg[i+1] == '\0')
		{
			end_of_data = 1;
		}
		buffer_element_t event = {msg[i], end_of_data};
		push_Queue_Element(0, event);
		i++;
	}

	buffer_element_t buffer_data_out = pull_Queue_Element(0);
	uint32_t data_out = SPI_PUSHR_CONT_MASK;
	if (buffer_data_out.end_of_data)
	{
		// If last word, indicate that the chip select should not be held
		data_out &= ~SPI_PUSHR_CONT_MASK;
	}
	// Select chip_select 0
	data_out |= SPI_PUSHR_PCS(1);
	data_out |= SPI_PUSHR_TXDATA(buffer_data_out.data);

	transfer_complete = 0;
	// Enable TCF flag to start pushing data to the queue
	SPI0->RSER |= SPI_RSER_TCF_RE_MASK;

	SPI0 -> PUSHR = data_out;
	gpioWrite(CAN_TP_PIN,LOW);
}

/**
 * @brief	Starts the Transmission of the data (8-bits words)
 * @param	bytes	Array of data (uint8_t*)
 * @param	num_of_bytes	Number of bytes of the array
 * @param 	callback		Function (NULL if not wanted) to be called
 * 							everytime a transmission is completed
 */

void SPI_SendData(uint8_t* bytes, uint32_t num_of_bytes, void (*callback)(void))
{
	gpioWrite(CAN_TP_PIN,HIGH);
	externalCB = callback;	// saves Callback to be used if necessary

	flush_Queue(1);
	uint32_t i = 0;
	for(i = 0; i < num_of_bytes; i++)
	{
		uint8_t end_of_data = 0;
		if(i == (num_of_bytes - 1))
		{
			end_of_data = 1;
		}
		buffer_element_t event = {bytes[i], end_of_data};
		push_Queue_Element(0, event);
	}

	buffer_element_t buffer_data_out = pull_Queue_Element(0);
	uint32_t data_out = SPI_PUSHR_CONT_MASK;
	if (buffer_data_out.end_of_data)
	{
		// If last word, indicate that the chip select should not be held
		data_out &= ~SPI_PUSHR_CONT_MASK;
	}
	// Select chip_select 0
	data_out |= SPI_PUSHR_PCS(1);
	data_out |= SPI_PUSHR_TXDATA(buffer_data_out.data);

	transfer_complete = 0;
	// Enable TCF flag to start pushing data to the queue
	SPI0->RSER |= SPI_RSER_TCF_RE_MASK;

	SPI0 -> PUSHR = data_out;
	gpioWrite(CAN_TP_PIN,LOW);

}

/**
 * @return	Transmission in process (0 No tranmission; 1 in process)
 */

uint8_t SPI_Transmission_In_Process()
{
	gpioWrite(CAN_TP_PIN,HIGH);
	return !transfer_complete;	// if EOQF==1; transmission is completed
	gpioWrite(CAN_TP_PIN,LOW);
}

/**
 * @return	Data_ready_to_read (0 Not ready; 1 ready)
 */

uint8_t SPI_Read_Status()
{
	gpioWrite(CAN_TP_PIN,HIGH);
	return !(SPI0 -> SR & SPI_SR_RXCTR_MASK) && get_Queue_Status(1) && !SPI_Transmission_In_Process();	// if EOQF==1; transmission is completed
	gpioWrite(CAN_TP_PIN,LOW);
}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
/**
 * @brief Handles Spi interrupts
 */
__ISR__ SPI0_IRQHandler(void)
{
	gpioWrite(CAN_TP_PIN,HIGH);
	uint32_t tmp;
	tmp = SPI0 -> SR;// Dummy read status register

	// Transfer FIFO Fill Flag (1 if not empty)
	if(IS_TCF(tmp))
	{
		SPI0->SR = SPI_SR_TCF_MASK;

		uint32_t rxdata = SPI0 -> POPR;
		buffer_element_t data_in = {rxdata, 0};
		push_Queue_Element(1, data_in); //positions itself in the receiver queue

		if (get_Queue_Status(0))
		{
			buffer_element_t buffer_data_out = pull_Queue_Element(0);
			uint32_t data_out = SPI_PUSHR_CONT_MASK;
			if (buffer_data_out.end_of_data)
			{
				// If last word, indicate that the chip select should not be held
				data_out &= ~SPI_PUSHR_CONT_MASK;
			}
			// Select chip_select 0
			data_out |= SPI_PUSHR_PCS(1);
			data_out |= SPI_PUSHR_TXDATA(buffer_data_out.data);
			SPI0 -> PUSHR = data_out;
		}
		// If no element in the queue, disable TFFF interrupts
		else
		{
			// write 0 to disable TFFF interrupt
			SPI0->RSER &= ~SPI_RSER_TCF_RE_MASK;
			transfer_complete = 1;
			if (externalCB)
			{
				externalCB();	// callback function
			}
		}
	}
	gpioWrite(CAN_TP_PIN,HIGH);

}

/**
 * @brief Initializes the circular queue
 */
static void queue_Init (uint8_t id)
{
	spi_buffers[id].pin = spi_buffers[id].buffer;
	spi_buffers[id].pout = spi_buffers[id].pin;
	spi_buffers[id].num_Of_Words = 0;
}

/**
 * @brief Pushes an event to the queue
 * @param event The element to add to the queue
 * @return Number of pending events. Returns value OVERFLOW if the maximun number of events is reached
 */
static int8_t push_Queue_Element(uint8_t id, buffer_element_t event)
{
	// Check for EventQueue Overflow
	if (spi_buffers[id].num_Of_Words > BUFFER_SIZE-1)
	{
		return OVERFLOW;
	}

	*(spi_buffers[id].pin)++ = event;
	spi_buffers[id].num_Of_Words++;

	// Return pointer to the beginning if necessary
	if (spi_buffers[id].pin == BUFFER_SIZE + spi_buffers[id].buffer)
	{
		spi_buffers[id].pin = spi_buffers[id].buffer;
	}

	return spi_buffers[id].num_Of_Words;

}

/**
 * @brief Pulls the earliest event from the queue
 * @return Event_Type variable with the current event if no OVERFLOW is detected.
 */


static buffer_element_t pull_Queue_Element(uint8_t id)
{
	buffer_element_t event = *(spi_buffers[id].pout);

	if (spi_buffers[id].num_Of_Words == 0)
	{
		buffer_element_t event1 = {0,2};
		return event1;
	}

	spi_buffers[id].num_Of_Words--;
	spi_buffers[id].pout++;

	if (spi_buffers[id].pout == BUFFER_SIZE + spi_buffers[id].buffer)
	{
		spi_buffers[id].pout = spi_buffers[id].buffer;
	}

	return event;

}


/**
 * @brief Gets the status of the queue
 * @return Returns the number of pending events in the queue
 */
static uint8_t get_Queue_Status(uint8_t id)
{
	return spi_buffers[id].num_Of_Words;
}


/**
 * @brief Pulls the earliest event from the queue
 * @return Event_Type variable with the current event if no OVERFLOW is detected.
 */


static void flush_Queue(uint8_t id)
{
	spi_buffers[id].pout = spi_buffers[id].pin;
	spi_buffers[id].num_Of_Words = 0;
}




/*******************************************************************************
 ******************************************************************************/





