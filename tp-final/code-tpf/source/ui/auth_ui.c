#include <stdint.h>

#include "auth_ui.h"
#include "display.h"
#include "credentials.h"
#include "../misc/timer.h"
#include "../drv/mag_strip.h"

// zero-initialized variables
static uint64_t current_id; 
static uint8_t id_digit_index;

static uint64_t current_pin;
static uint8_t pin_digit_index;

static uint8_t brightness_index;

static uint8_t id_len;
static uint8_t pin_len;

static bool credentials_checked;
static bool credentials_ok;
static bool timeout_pending;

// static local functions prototypes
static uint64_t pan2Id(uint64_t pan);

static unsigned int mod(int a, int b); 

// true modulo function
static unsigned int mod(int a, int n) 
{
    return ((a % n) + n) % n;
}

// global functions definitions
void increaseDigitID(void)
{
    id_digit_index = mod(id_digit_index + 1, 10);
}

void decreaseDigitID(void)
{
    id_digit_index = mod(id_digit_index - 1, 10);
}

void storeDigitID(void)
{
    if (id_len != 7) 
    {
        current_id = (current_id + id_digit_index) * 10;
    }
    else
    {
        current_id = current_id + id_digit_index;
    }
    id_digit_index = 0;
    id_len++;
}

void eraseDigitID(void)
{
    current_id /= 10;
    id_digit_index = current_id % 10;
    current_id = current_id - id_digit_index;
    id_len--;
}

void increaseDigitPIN(void)
{
    if (pin_len == 4) // in the last digit '-' (HYPHEN) is also valid
    {
        pin_digit_index = mod(pin_digit_index + 1, 11);
    }
    else
    {
        pin_digit_index = mod(pin_digit_index + 1, 10);
    }
}

void decreaseDigitPIN(void)
{
    if (pin_len == 4)
    {
        pin_digit_index = mod(pin_digit_index - 1, 11);
    }
    else
    {
        pin_digit_index = mod(pin_digit_index - 1, 10);
    }
}

void storeDigitPIN(void)
{
    if (pin_digit_index == HYPHEN)
    {
        current_pin /= 10;
    }
    else if (pin_len == 4 && pin_digit_index != HYPHEN)
    {
        current_pin = current_pin + pin_digit_index;
    }
    else if (pin_len != 4)
    {
        current_pin = (current_pin + pin_digit_index) * 10;
    }
    pin_digit_index = 0;
    pin_len++;
}

void eraseDigitPIN(void)
{
    current_pin /= 10;
    pin_digit_index = current_pin % 10;
    current_pin = current_pin - pin_digit_index;
    pin_len--;
}

void printID(void)
{
    display(current_id + id_digit_index, false, id_len);
    turnOnLED(1);
}

void printPIN(void)
{    
    if (pin_digit_index == HYPHEN)
    {
        displayHyphens();
    }
    else
    {
        display(current_pin + pin_digit_index, true, pin_len);
    }
    turnOnLED(2);
}

void printMenu(void)
{
    display(8888, false, 4);
}

void increaseBrightness(void)
{
    brightness_index = mod(brightness_index + 1, BRIGHTNESS_LEVELS);
    setPWM(brightness_index);
}

void decreaseBrightness(void)
{
    brightness_index = mod(brightness_index - 1, BRIGHTNESS_LEVELS);
    setPWM(brightness_index);
}

static uint64_t pan2Id(uint64_t pan)
{
	int id = pan % 100000000;
	return id;
}

void storeMagStripID(void)
{
    uint64_t pan;
    uint32_t add_data, disc_data;

    int success = processStripData(&pan, &add_data, &disc_data);
    if (!success) return;

    current_id = pan2Id(pan);
}

void checkCredentials(void)
{
    credentials_checked = true;
    for (size_t i = 0; i < number_of_users; i++) 
    {
        if (credentials[i].id  == current_id &&
            credentials[i].pin == current_pin) 
        {
            credentials_ok = true;
            break;
        }
    }
}

bool isDataReady(void)
{
    return credentials_checked;
}

bool isValid(void)
{
    return credentials_ok;
}

void unlockLED(void)
{
    dispClear();
    tim_id_t tim_id = timerGetId();
    if (tim_id != TIMER_INVALID_ID)
    {
        timerStart(tim_id, 10000, TIM_MODE_SINGLESHOT, NULL);
    }
    while(!timerExpired(tim_id))
    {
        turnOnLED(1);
        turnOnLED(2);
        turnOnLED(3);
    }
    turnOffLEDs();
    reset();
}

void invalidCredentials(void)
{
    turnOffLEDs();
    tim_id_t tim_id = timerGetId();
    if (tim_id != TIMER_INVALID_ID)
    {
        timerStart(tim_id, 10000, TIM_MODE_SINGLESHOT, NULL);
    }
    while(!timerExpired(tim_id));
    reset();
}

void triggerTimeout(void)
{
    timeout_pending = true;
}

bool isTimeout(void)
{
    return timeout_pending;
}

void reset(void)
{
    current_id = 0;
    current_pin = 0;
    id_digit_index = 0;
    pin_digit_index = 0;
    id_len = 0;
    pin_len = 0;
    credentials_checked = 0;
    credentials_ok = 0;
    timeout_pending = false;
    resetMagData();
    turnOffLEDs();
}
