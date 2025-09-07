/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stddef.h>
#include "timer.h"
#include "drv/SysTick.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define SYSTICK_COUNT 100000 // --> T_systick = 1 ms

/*******************************************************************************
 * VARIABLE DECLARATIONS WITH FILE SCOPE
 ******************************************************************************/

static Timer_t timer[TIMERS_MAX_QTY];  // gets zero-initialized by default

/*******************************************************************************
 * FUNCTION DEFINITIONS WITH FILE SCOPE
 ******************************************************************************/

static void countTicks(void)
{
    for (int i = 0; i < TIMERS_MAX_QTY; i++)
    {
        if (timer[i].ticks == 0) continue; // timer not initialized

        if (timer[i].counter > 0) // prevent wrap around overflow
        {
            timer[i].counter--;
            if (timer[i].counter == 0)
                timer[i].pending = true;
        }
    }
}


/*******************************************************************************
 * FUNCTION DEFINITIONS WITH GLOBAL SCOPE
 ******************************************************************************/

void timerInit(void)
{
    SysTick_Init(countTicks, SYSTICK_COUNT);
    // to-do: handle systick_init error
}

tim_id_t timerGetId(void)
{
    for (int i = 0; i < TIMERS_MAX_QTY; i++)
    {
        if (timer[i].ticks == 0)
            return i;   // the first free id
    }

    return TIMER_INVALID_ID; // there were no id's left
}

bool timerStart(tim_id_t id, tim_tick_t ticks, uint8_t mode, 
                tim_callback_t callback)
{
    if (id >= TIMERS_MAX_QTY || ticks == 0 || timer[id].ticks != 0) 
        return false;   // timer[id].ticks = 0 only when not initialized

    timer[id].ticks     = ticks;
    timer[id].counter   = ticks;
    timer[id].mode      = mode;
    timer[id].pending   = false;
    timer[id].cb        = callback;

    return true;
}

void timerStop(tim_id_t id)
{
    if (id >= TIMER_INVALID_ID) return;

    timer[id].ticks     = 0;
    timer[id].counter   = 0;
    timer[id].mode      = 0;
    timer[id].pending   = false;
    timer[id].cb        = NULL;
}

bool timerExpired(tim_id_t id)
{
    return timer[id].pending;
}

void timerUpdate(void)
{
    for (int i = 0; i < TIMERS_MAX_QTY; i++)
    {
        if (timer[i].ticks == 0) continue;  // timer not initialized

        if (timer[i].pending)
        {
            timer[i].pending = false;

            if (timer[i].mode == TIM_MODE_SINGLESHOT)
                timerStop(i);
            else if (timer[i].mode == TIM_MODE_PERIODIC)
                timer[i].counter = timer[i].ticks;  // re-arm

            if (timer[i].cb != NULL)
                timer[i].cb();
        }
    }
}
