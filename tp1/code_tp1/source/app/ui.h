#include <stdint.h>

#include "fsm.h"
#include "../drv/mag_strip.h"
#include "../drv/rotary_encoder.h"

// returns id read from the magnetic strip
uint32_t readIdMagStrip(void);

// Idle
void printMenu(void);         
void reset(void);             
void printInsertId(void);     

// Insert ID / PIN editin
void increaseDigit(void);     
void decreaseDigit(void);     
void storeDigit(void);        
void eraseDigit(void);        
void clearInput(void);        
void printInsertPin(void);    

// Validation & result
void validateID(void);        
void validatePIN(void);       
void lightLEDs(void);         
void printWrong(void);        
void validation(void);        

// Unlock timin
void unlockLEDOff(void);      
void finalDelay(void);