#include "board.h"
#include "mag_strip.h"
#include <stdint.h>


#define STRIP_ACTIVE LOW
#define MAX_DIGITS 40
#define START_SENTINEL 0b01011
#define END_SENTINEL 0b11111

unsigned char unprocessed_digits[MAX_DIGITS];


void readTrack2Data(void); //interrupts with reading function, if reading doesnt mach N bits, return error message
void releaseData(void); //callback for when enable IRQ occurs.
int validateData(void); //returns array real size


//static uint8_t data[STRIP_BUFFER_SIZE];
static uint8_t is_data_ready;

//#define MAX_LONG 200
//#define BUFFER_SIZE 4 // array size / ELEMENT SIZE
//#define ELEMENT_SIZE 64
int getIsDataReady(void)
{
	return is_data_ready;
}

int magStrip_Init(void)
{
	static int already_initialized = 0;
	if(already_initialized)
	{
		return 1;
	}
    gpioMode(STRIP_ENABLE, INPUT);
    gpioMode(STRIP_DATA, INPUT);
    gpioMode(STRIP_CLK, INPUT);
    gpioIRQ(STRIP_CLK, PORT_PCR_IRQC_INT_FALLING, readTrack2Data);
    gpioIRQ(STRIP_ENABLE, PORT_PCR_IRQC_INT_RISING, releaseData);

    already_initialized = 1;
    return 1;
}

int processStripData(void)
{

	is_data_ready = 0;
	return 1;
}
void releaseData(void)
{
	is_data_ready=1;
	return;
}
int validateData(void)
{
	int i;
	for(i= 0; i < MAX_DIGITS; i++)
	{
		if(unprocessed_digits[i] == END_SENTINEL)
		{
			return (i+1);
		}
	}
	return 0;
}


void readTrack2Data(void)
{
	static unsigned char bit_shift = 0;
	static unsigned char buffer= 0;
	static unsigned char i = 0;
	static char es_found = 0;

	if(es_found)
	{
		return;
	}

	buffer |= ((gpioRead(STRIP_DATA) == STRIP_ACTIVE) << bit_shift++);


	if(bit_shift == 5) //A character has been processed
	{
		unprocessed_digits[i++] = buffer;

		if(buffer == END_SENTINEL)
		{
			es_found = 1;
		}
		//Checks if first digit is START SENTINEL
		if(unprocessed_digits[0]!= START_SENTINEL)
		{
			i = 0;
			buffer = buffer >> 1;
			bit_shift = 4;
		}
		else
		{
			buffer = 0;
			bit_shift = 0;
		}
	}

	return;
}
//void readTrack2Data(void)
//{
//	if(bit_index <= MAX_LONG)
//	{
//		data_buffer[bit_index/ELEMENT_SIZE] |= (gpioRead(STRIP_DATA) == STRIP_ACTIVE)
//				<< (bit_index - ELEMENT_SIZE * (bit_index/ELEMENT_SIZE) );
//		bit_index++;
//	}
//	//Search for Start Sentinel (0xB Big endian)
//	if(bit_indexx == 4 && (data_buffer[0] != 0b11010))
//	{
//		data_buffer[0] = 0;
//		bit_index = 0;
//	}
//	return;
//}
