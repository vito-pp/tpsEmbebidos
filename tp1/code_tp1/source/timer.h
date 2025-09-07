/***************************************************************************//**
  @file     timer.h
  @brief    Timer driver. Advance implementation, Non-Blocking services. Timer 
            abstraction layer based on SysTick
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

#define TIMER_TICK_PER_MS   1
#define TIMER_MS2TICKS(ms)  ((ms)/TIMER_TICK_MS)

#define TIMERS_MAX_QTY      6
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

typedef struct Timer_t
{
  tim_id_t id;
  tim_tick_t ticks;
  volatile tim_tick_t counter;
  volatile bool pending;
  uint8_t mode;   // singleshot or periodic
  tim_callback_t cb;
} Timer_t;

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
 * @brief Request a timer ID
 * @return ID of the timer to use. If no timers available returns 
 * TIMER_INVLALID_ID
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
 * @brief Kills a given timer
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
 * @brief Call respective callbacks if timeout ocurrs. Must be called from main 
 * loop
 */
void timerUpdate(void);

/*******************************************************************************
 ******************************************************************************/

#endif // _TIMER_H_
