#include <stdint.h>

#include "validate.h"

// arrays that store the digits read from the encoder. zero-initialized
static uint64_t current_id; 
static uint8_t id_digit_index;

static uint64_t current_pin;
static uint8_t pin_digit_index;

void increaseDigitID(void)
{

}

void decreaseDigitID(void);

void storeDigitID(void);

void eraseDigitID(void);

void increaseDigitPIN(void);

void decreaseDigitPIN(void);

void storeDigitPIN(void);

void eraseDigitPIN(void);





