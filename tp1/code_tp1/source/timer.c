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
static tim_id_t busy_ids[TIMERS_MAX_QTY];   // gets zero-initialized by default

/*******************************************************************************
 * FUNCTION DEFINITIONS WITH FILE SCOPE
 ******************************************************************************/

static void countTicks(void)
{
    for (int i = 0; i < TIMERS_MAX_QTY; i++)
    {
        if (busy_ids[i] == TIMER_INVALID_ID)
            break;  // optimization for less iterations

        timer[busy_ids[i]].counter--;
        if (timer[busy_ids[i]].counter == 0)
            timer[busy_ids[i]].pending = true;
    }
}

/*******************************************************************************
 * FUNCTION DEFINITIONS WITH GLOBAL SCOPE
 ******************************************************************************/

void timerInit(void)
{
    SysTick_Init(countTicks, SYSTICK_COUNT);
    for (int i = 0; i < TIMERS_MAX_QTY; i++)
        busy_ids[i] = TIMER_INVALID_ID;
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

bool timerStart(tim_id_t id, tim_tick_t ticks, uint8_t mode, tim_callback_t callback)
{
    if (id >= TIMERS_MAX_QTY || ticks <= 0 || timer[id].ticks != 0) 
        return false;   // timer[id].ticks = 0 only when not initialized

    timer[id].ticks = ticks;
    timer[id].counter = ticks;
    timer[id].mode = mode;
    timer[id].pending = false;
    timer[id].cb = callback;

    // put the id in the first free spot in the busy_id array
    for(int i = 0; i < TIMERS_MAX_QTY; i++)
    {
        if (busy_ids[i] == TIMER_INVALID_ID)
        {
            busy_ids[i] = id;
            break;
        }
    }

    return true;
}

void timerStop(tim_id_t id)
{
    timer[id].ticks = 0;
    timer[id].counter = 0;
    timer[id].mode = 0;
    timer[id].pending = false;
    timer[id].cb = NULL;

    // reorganize the busy_ids array
    for(int i = 0; i < TIMERS_MAX_QTY; i++)
    {
        int j = 0;
        if (timer[i].ticks != 0) // the timer is initialized
            busy_ids[j++] = timer[i].id;
    }
}

bool timerExpired(tim_id_t id)
{
    return timer[id].pending;
}

void timerUpdate(void);
