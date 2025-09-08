/**
 * @file fms.h
 * @brief Prototype of the FSM structure
 */

#ifndef FSM_H_
#define FSM_H_

#include <stdint.h>

typedef struct State_t
{
    uint8_t event;
    const struct State_t *next_state_table;
    void (*action)(void);
} State_t;

#endif // FSM_H_