#include "ui.h"
#include "../display.h"

// Idle
void printMenu(void)
{
    display(8, 0, 0);
}
void reset(void)             
{
    display(0, 0, 0);
    display(0, 1, 0);
    display(0, 2, 0);
    display(0, 3, 0); 
}
void printInsertId(void)     
{ 
    display(0, 0, 0);
    display(0, 1, 0);
    display(0, 2, 0);
    display(0, 3, 0);
}

// Insert ID / PIN editing
void increaseDigit(void)     
{
	static int i = 0;
    display(i++, 0, 0);
}
void decreaseDigit(void)     
{ 
    display(2, 0, 0);
    display(0, 1, 0);
    display(0, 2, 0);
    display(0, 3, 0);
}
void storeDigit(void)        
{ 
    display(3, 0, 0);
    display(0, 1, 0);
    display(0, 2, 0);
    display(0, 3, 0);
}
void eraseDigit(void)        
{ 
    display(0, 0, 0);
    display(0, 1, 0);
    display(0, 2, 0);
    display(0, 3, 0);
}
void clearInput(void)        
{ 
    display(0, 0, 0);
    display(0, 1, 0);
    display(0, 2, 0);
    display(0, 3, 0);
}
void printInsertPin(void)    
{ 
    display(0, 0, 0);
    display(0, 1, 0);
    display(0, 2, 0);
    display(0, 3, 0);
}

// Validation & results
void validateID(void)        
{ 
    display(0, 0, 0);
    display(0, 1, 0);
    display(0, 2, 0);
    display(0, 3, 0);
}
void validatePIN(void)       
{ 
    display(0, 0, 0);
    display(0, 1, 0);
    display(0, 2, 0);
    display(0, 3, 0);
}
void lightLEDs(void)         
{ 
    display(0, 0, 0);
    display(0, 1, 0);
    display(0, 2, 0);
    display(0, 3, 0);
}
void printWrong(void)        
{ 
    display(0, 0, 0);
    display(0, 1, 0);
    display(0, 2, 0);
    display(0, 3, 0);
}
void validation(void)        
{ 
    display(0, 0, 0);
    display(0, 1, 0);
    display(0, 2, 0);
    display(0, 3, 0);
}

// Unlock timing
void unlockLEDOff(void)      
{ 
    display(0, 0, 0);
    display(0, 1, 0);
    display(0, 2, 0);
    display(0, 3, 0);
}
void finalDelay(void)
{ 
    display(0, 0, 0);
    display(0, 1, 0);
    display(0, 2, 0);
    display(0, 3, 0);
}
