/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stddef.h>
#include <stdio.h>

#include "drv/SysTick.h"
#include "drv/board.h"
#include "drv/gpio.h"
#include "drv/mag_strip.h"
#include "drv/matStream.h"
#include "drv/rotary_encoder.h"
#include "drv/shift_registers.h"
#include "misc/timer.h"
#include "ui/map.h"
#include "ui/auth_ui.h"
#include "ui/display.h"
#include "ui/fsm.h"

/*******************************************************************************
 * EXTERN VARIABLES
 ******************************************************************************/

extern uint8_t sendingDMA;

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

// int pan2Id(uint64_t pan);

void matrixSyncFromCredentials(void);

static FSM_State_t *current;
static FSM_event_t event;

enc_input_t last_button_state = ENC_NONE;

tim_id_t inactivity_id;

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/* Función que se llama 1 vez, al comienzo del programa */
void App_Init(void) {
  encoderInit();
  magStrip_Init();
  display_init();
  dispBus_init();

  current = getInitState();

  timerInit();
  tim_id_t id = timerGetId();
  if (id != TIMER_INVALID_ID) {
    timerStart(id, 1, TIM_MODE_PERIODIC, encoder_callback);
  }
  inactivity_id = timerGetId();
  if (inactivity_id != TIMER_INVALID_ID) {
    timerStart(id, 5000, TIM_MODE_PERIODIC, triggerTimeout);
  }
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run(void) {
  event = getEvent();
  current = fsmStep(current, event);

  if (event != EV_NONE) {
    timerStop(inactivity_id);
    timerStart(inactivity_id, 20000, TIM_MODE_PERIODIC, triggerTimeout);
  }

  timerUpdate();
  if (!sendingDMA) {
    gpioWrite(PORTNUM2PIN(PB, 3), 1);
    matrixSyncFromCredentials();
    // displayMatrix(display,7,sizeof(display));
  }
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void matrixSyncFromCredentials(void)
{
	setOcupation(1, getFloorOccupancy(1));
	setOcupation(2, getFloorOccupancy(2));
	setOcupation(3, getFloorOccupancy(3));

	clearErrorX(1);
	clearErrorX(2);
	clearErrorX(3);
	clearErrorX(4);

	// if last event was valid:
	//     setErrorX(1);      // access OK indicator
	//
	// if last event was invalid:
	//     setErrorX(2);      // invalid credential indicator

	loadMap();
}
