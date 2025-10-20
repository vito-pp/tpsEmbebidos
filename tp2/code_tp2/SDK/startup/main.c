/***************************************************************************//**
  @file     main.c
  @brief    FW main
  @author   Nicolás Magliola
 ******************************************************************************/

#include "hardware.h"
#include <stdio.h>



void App_Init (void);
void App_Run (void);
//#include "fsl_debug_console.h"



int main (void)
{
    hw_Init();
    hw_DisableInterrupts();
    App_Init(); /* Program-specific setup */
    hw_EnableInterrupts();
    __FOREVER__
      App_Run(); /* Program-specific loop  */
}