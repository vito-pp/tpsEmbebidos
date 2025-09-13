#include <stdint.h>

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