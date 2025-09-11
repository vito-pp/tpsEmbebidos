#ifndef FSM_TABLE_H_
#define FSM_TABLE_H_

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef enum 
{
    EV_ENTER,
    EV_DOUBLE_ENTER,  
    EV_FORWARD,   
    EV_BACKWARD, 
    EV_RESET,
    EV_MAG_DATA,
    EV_VALID,
    EV_INVALID,
    EV_TIMEOUT,
    EV_NONE = 0xFF
} FSM_event_t;

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

FSM_State_t *getInitState(void);

#endif // FSM_TABLE_H_