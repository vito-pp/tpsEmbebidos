/***************************************************************************//**
  @file     main.c
  @brief    FW main
  @author   Nicolás Magliola
 ******************************************************************************/

#include "hardware.h"
#include "../../source/drv/FXOS8700CQ.h"
#include "../../source/drv/can.h"
#include <stdio.h>



void App_Init (void);
void App_Run (void);
void __error_handler(void); 
//#include "fsl_debug_console.h"



int main (void)
{
    hw_Init();
    hw_DisableInterrupts();
    App_Init(); /* Program-specific setup */
    hw_EnableInterrupts();
    // these inits need IRQs
    if (!FXOS_Init(0, 9600))
    {
        __error_handler__();
    }
    CAN_SPI_Init();

    __FOREVER__
        App_Run(); /* Program-specific loop  */
}
