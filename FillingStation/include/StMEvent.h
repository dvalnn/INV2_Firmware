#ifndef STM_EVENT_H_
#define STM_EVENT_H_

#include "StateMachine.h"

/*
 * Define here all conditions for events and reaction functions 
 */


//rocket_state_t event_handler(rocket_state_t state);
bool IdleCond(void);
bool TrueCond(void);

bool prog1_finish_cond(void);
bool prog2_finish_cond(void);
bool prog3_finish_cond(void);
bool prog2_stop_cond(void);
void prog2_stop_reaction(void);
void close_valves(void);

#endif