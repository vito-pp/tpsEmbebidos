#include "board.h"
#include "mag_strip.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>


#define STRIP_ACTIVE LOW
#define MAX_DIGITS 40
#define START_SENTINEL 0b01011
#define END_SENTINEL 0b11111
#define FIELD_SEPARATOR 0b01101

static unsigned char unprocessed_digits[MAX_DIGITS];
static bool reset_reader;
static bool is_data_ready;

/**
 * @brief Reads data sent my Magnetic Strip Track 2 reader.
 */
static void readTrack2Data(void);
/**
 * @brief Enables data to be processed.
 * 		  Resets Track 2's reader.
 */
static void releaseData(void);

//#define MAX_LONG 200
//#define BUFFER_SIZE 4 // array size / ELEMENT SIZE
//#define ELEMENT_SIZE 64

int magStrip_Init(void)
{
	static int already_initialized = 0;
	if(already_initialized)
	{
		return 0;
	}
    gpioMode(STRIP_ENABLE, INPUT);
    gpioMode(STRIP_DATA, INPUT);
    gpioMode(STRIP_CLK, INPUT);
    gpioIRQ(STRIP_CLK, PORT_PCR_IRQC_INT_FALLING, readTrack2Data);
    gpioIRQ(STRIP_ENABLE, PORT_PCR_IRQC_INT_RISING, releaseData);

    already_initialized = 1;
    is_data_ready = 0;

    return 1;
}

bool isMagDataReady(void)
{
	return is_data_ready;
}


void releaseData(void)
{
	is_data_ready = 1;

	reset_reader = 1;
	readTrack2Data();

	return;
}


void readTrack2Data(void)
{
	static unsigned char bit_shift = 0;
	static unsigned char buffer= 0;
	static char es_found = 0;
	static unsigned char i = 0;

	if(reset_reader)
	{
		i = 0;
		buffer = 0;
		es_found = 0;
		reset_reader = 0;
		return;
	}

	if(es_found)
	{
		return;
	}

	buffer |= ((gpioRead(STRIP_DATA) == STRIP_ACTIVE) << bit_shift++);


	if(bit_shift == 5) //A character has been read.
	{
		unprocessed_digits[i++] = buffer;

		if(buffer == END_SENTINEL)
		{
			es_found = 1;
		}
		//Searches for START SENTINEL to start storing data
		if(unprocessed_digits[0]!= START_SENTINEL)
		{
			//Sets variable to continue looking for START_SENTINEL
			i = 0;
			buffer = buffer >> 1;
			bit_shift = 4;
		}
		else
		{
			//Clear buffer
			buffer = 0;
			bit_shift = 0;
		}
	}


	return;
}

int processStripData(uint64_t *pan, uint32_t *add_data, uint32_t *disc_data)
{
	int i, j;
	uint64_t aux = 0;
	is_data_ready = 0;
	uint8_t length = validateData();
	if(!length)
	{
		return 0;
	}

	if(pan == NULL || add_data == NULL || disc_data == NULL)
	{
		return 0;
	}


	for(i = 1; i < length; i++)
	{
		if(unprocessed_digits[i] == FIELD_SEPARATOR)
		{
			i++;
			*pan = aux;
			break;
		}
		aux = aux*10 + (unprocessed_digits[i] & 0b1111); //b4 contains its parity
	}

	aux = 0;
	for(j = 0; j< 7; j++)
	{
		aux = aux*10 + (unprocessed_digits[i] & 0b1111); //b4 contains its parity
		i++;
	}
	*add_data = aux;

	aux = 0;
	for(; i < length; i++)
	{
		aux = aux*10 + (unprocessed_digits[i] & 0b1111); //b4 contains its parity
	}
	*disc_data = aux; //Equals 0 if discretionary data not contained


	return 1;
}

uint8_t validateData(void)
{
	int i, j;
	unsigned char char_copy, parity_count;
	bool fs_found = 0; //Equals TRUE if FIELD SEPARATOR IS FOUND.

	for(i= 0; i < MAX_DIGITS; i++)
	{
		if(unprocessed_digits[i] == END_SENTINEL && fs_found)
		{
			//String has i elements (doesn't count END_SENTINEL char).
			return i;
		}
		else if(unprocessed_digits[i] == FIELD_SEPARATOR)
		{
			fs_found = 1;
		}

		//Validates char parity
		char_copy = unprocessed_digits[i];
		parity_count = 0;
		for(j = 0; j < 5; j++)
		{
			parity_count += 1 & char_copy;
			char_copy = char_copy >> 1;
		}
		if(!(parity_count % 2))
		{
			//It's an even number --> Invalid format
			return 0;
		}
		//If function reaches here, current number has a valid format
	}
	return 0; //Error, END_SENTINEL or FIELD SEPARATOR NOT FOUND
}

void resetMagData(void)
{
	is_data_ready = false;
	reset_reader = true;
}


