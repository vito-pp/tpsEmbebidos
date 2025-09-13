/***************************************************************************//**
  @file     timer.h
  @brief    Timer driver. Advance implementation, Non-Blocking services
  @author   Nicolás Magliola
 ******************************************************************************/

#ifndef _TIMER_H_
#define _TIMER_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define TIMER_TICK_MS       1
#define TIMER_MS2TICKS(ms)  ((ms)/TIMER_TICK_MS)

#define TIMERS_MAX_CANT     16
#define TIMER_INVALID_ID    255

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

// Timer Modes
enum { TIM_MODE_SINGLESHOT, TIM_MODE_PERIODIC, CANT_TIM_MODES };

// Timer alias
typedef uint32_t tim_tick_t;
typedef uint8_t tim_id_t;
typedef void (*tim_callback_t)(void);

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initialize timer and corresponding peripheral
 */
void timerInit(void);

/**
 * @brief Request an timer
 * @return ID of the timer to use
 */
tim_id_t timerGetId(void);


/**
 * @brief Begin to run a new timer
 * @param id ID of the timer to start
 * @param ticks time until timer expires, in ticks
 * @param mode SINGLESHOT or PERIODIC
 * @param callback Function to be call when timer expires (NULL if no necessary)
 * @return true = timer start succeed
 */
bool timerStart(tim_id_t id, tim_tick_t ticks, uint8_t mode, tim_callback_t callback);

/**
 * @brief Finish to run a timer
 * @param id ID of the timer to stop
 */
void timerStop(tim_id_t id);

/**
 * @brief Verify if a timer has run timeout
 * @param id ID of the timer to check for expiration
 * @return true = timer expired
 */
bool timerExpired(tim_id_t id);

/**
 * @brief Call respective callbacks if timeout ocurrs. Must be call from main loop.
 */
void timerUpdate(void);

/*******************************************************************************
 ******************************************************************************/

#endif // _TIMER_H_
