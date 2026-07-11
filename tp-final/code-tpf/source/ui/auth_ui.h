#ifndef _AUTH_UI_H_
#define _AUTH_UI_H_

#include <stdint.h>
#include <stdbool.h>

void increaseDigitID(void);

void decreaseDigitID(void);

void storeDigitID(void);

void eraseDigitID(void);

void increaseDigitPIN(void);

void decreaseDigitPIN(void);

void storeDigitPIN(void);

void eraseDigitPIN(void);

// returns id read from the magnetic strip
uint32_t readIdMagStrip(void);

void printID(void);

void printPIN(void);

void printMenu(void);

void increaseBrightness(void);

void decreaseBrightness(void);

void storeMagStripID(void);

void checkCredentials(void);

bool isDataReady(void);

bool isValid(void);

void unlockLED(void);

void invalidCredentials(void);

void triggerTimeout(void);

bool isTimeout(void);

void reset(void);

#endif // _AUTH_UI_H_