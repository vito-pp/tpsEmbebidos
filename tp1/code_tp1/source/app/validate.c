#include <stdint.h>

#include "validate.h"
#include "display.h"

// zero-initialized variables
static uint64_t current_id; 
static int8_t id_digit_index;

static uint64_t current_pin;
static int8_t pin_digit_index;

// true modulo function
static unsigned int mod(int a, int n) 
{
    return ((a % n) + n) % n;
}

void increaseDigitID(void)
{
    id_digit_index = mod(++id_digit_index, 10);
}

void decreaseDigitID(void)
{
    id_digit_index = mod(--id_digit_index, 10);
}

void storeDigitID(void)
{
    current_id = (current_id + id_digit_index) * 10;
    id_digit_index = 0;
}

void eraseDigitID(void)
{
    current_id /= 10;
    id_digit_index = 0;
}

void increaseDigitPIN(void)
{
    pin_digit_index = mod(++pin_digit_index, 10);
}

void decreaseDigitPIN(void)
{
    pin_digit_index = mod(--pin_digit_index, 10);
}

void storeDigitPIN(void)
{
    current_pin = (current_pin + pin_digit_index) * 10;
    pin_digit_index = 0;
}

void eraseDigitPIN(void)
{
    current_pin /= 10;
    pin_digit_index = 0;
}

void printID(void)
{
    display(current_id + id_digit_index, false);
}

void printPIN(void)
{
    display(current_pin + pin_digit_index, true);
}

void printMenu(void)
{
    display(8888, false);
}

void increaseBrightness(void)
{
    static int i = 0;
    setPWM(++i);
}

// void decreaseBrightness(void);
