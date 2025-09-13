#include <stdint.h>

#include "validate.h"

// zero-initialized variables
static uint64_t current_id; 
static uint8_t id_digit_index;

static uint64_t current_pin;
static uint8_t pin_digit_index;

// true modulo function
static unsigned int mod(int a, int n) 
{
    return ((a % n) + n) % n;
}

void increaseDigitID(void)
{
    id_digit_index = mod(id_digit_index++, 10);
    current_id = current_id + id_digit_index;
}

void decreaseDigitID(void)
{
    id_digit_index = mod(id_digit_index--, 10);
    current_id = current_id + id_digit_index;
}

void storeDigitID(void)
{
    id_digit_index = 0;
    current_id *= 10;
}

void eraseDigitID(void)
{
    id_digit_index = 0;
    current_id /= 10;
}

uint64_t getCurrentID(void)
{
    return current_id;
}

void increaseDigitPIN(void)
{
    pin_digit_index = mod(pin_digit_index++, 10);
    current_pin = current_pin + pin_digit_index;
}

void decreaseDigitPIN(void)
{
    pin_digit_index = mod(pin_digit_index--, 10);
    current_pin = current_pin + pin_digit_index;
}

void storeDigitPIN(void)
{
    pin_digit_index = 0;
    current_pin *= 10;
}

void eraseDigitPIN(void)
{
    pin_digit_index = 0;
    current_pin /= 10;
}

uint64_t getCurrentPIN(void)
{
    return current_pin;
}



