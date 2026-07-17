/***************************************************************************//**
  @file     main.c
  @brief    FW main
  @author   Nicolás Magliola
 ******************************************************************************/

#include <os.h>

#include "hardware.h"
#include "rtos/app_main_task.h"

int main(void)
{
    OS_ERR err;

    hw_Init();
    hw_DisableInterrupts();

    OSInit(&err);
    if (err != OS_ERR_NONE) {
        while (1) {
        }
    }

    AppMainTask_Create();

    OSStart(&err);

    while (1) {
        /*
         * Should never reach this point.
         */
    }
}
