#include <Arduino.h>

#include <time.h>

#include "StateMachine.h"
#include "StMWork.h"
#include "StMEvent.h"

#include <stdarg.h>
#include <limits.h>


//need to add a stop and see how the executin function changes
rocket_state_t comm_transition[rocket_state_size][cmd_size] = {  
//                STATUS ABORT EXEC   STOP   FUELING    [Flight cmds]  RESUME, ADD, REMOVE    
/* Idle    */   {   -1 , ABORT, -1,   -1,    FUELING,  -1,-1,-1,-1,-1,    -1,  -1,   -1   },      
/* FUELING */   {   -1 , ABORT, -1,   -1,    FUELING,  -1,-1,-1,-1,-1,    -1,  -1,   -1   },      
/* Prog1   */   {   -1 , ABORT, -1, FUELING, FUELING,  -1,-1,-1,-1,-1,    -1,  -1,   -1   },      
/* Prog2   */   {   -1 , ABORT, -1, FUELING, FUELING,  -1,-1,-1,-1,-1,    -1,  -1,   -1   },      
/* Prog3   */   {   -1 , ABORT, -1, FUELING, FUELING,  -1,-1,-1,-1,-1,    -1,  -1,   -1   },      
/* Stop    */   {   -1 , ABORT, -1, FUELING, FUELING,  -1,-1,-1,-1,-1,  PROG2, -1,   -1   },      
/* Abort   */   {   -1 ,  -1  , -1,   -1,      IDLE,   -1,-1,-1,-1,-1,    -1   -1,   -1   },      
};

/*
 EXEC_CMD state is set inside the command function so it can have 3 diferent exits, [prog1, prog2, prog3]
 this also help with safety, the cmand function verifys the state that executed the command
*/

State_t state_machine[rocket_state_size] = 
{
    //IDLE
    {
        .work = { {.chanel = read_pressures, .delay = 10},
                  {.chanel = read_temperatures, .delay = 10}},

        .events = {},

        .comms = comm_transition[IDLE],
    },
    //FUELING
    {
        .work = { {.chanel = read_pressures, .delay = 10},
                  {.chanel = read_temperatures, .delay = 10},
                  {.chanel = V1_close, .delay = 500},
                  {.chanel = V2_close, .delay = 500},
                  {.chanel = V3_close, .delay = 500} },

        .events = {},

        .comms = comm_transition[FUELING],
    },
    //PROG1
    {
        .work = { {.chanel = read_pressures, .delay = 10},
                  {.chanel = read_temperatures, .delay = 10},
                  {.chanel = V1_open, .delay = 500} },

        .events = { {.condition = prog1_finish_cond, .reaction = V1_close, .next_state = FUELING} },

        .comms = comm_transition[PROG1],
    },
    //PROG2
    {
        .work = { {.chanel = read_pressures, .delay = 10},
                  {.chanel = read_temperatures, .delay = 10},
                  {.chanel = V2_open, .delay = 500} },

        .events = { {.condition = prog2_stop_cond, .reaction = V2_close, .next_state = STOP},
                    {.condition = prog2_finish_cond, .reaction = V2_close, .next_state = FUELING} },

        .comms = comm_transition[PROG2],
    },
    //PROG3
    {
        .work = { {.chanel = read_pressures, .delay = 10},
                  {.chanel = read_temperatures, .delay = 10},
                  {.chanel = V3_open, .delay = 500} },

        .events = { {.condition = prog3_finish_cond, .reaction = V3_close, .next_state = FUELING} },

        .comms = comm_transition[PROG3],
    },
    //STOP
    {
        .work = { {.chanel = read_pressures, .delay = 10},
                  {.chanel = read_temperatures, .delay = 10},
                  {.chanel = V2_close, .delay = 500} },

        .events = {},

        .comms = comm_transition[STOP],
    },
    //ABORT
    {
        .work = { {.chanel = read_pressures, .delay = 10},
                  {.chanel = read_temperatures, .delay = 10}},

        .events = {},

        .comms = comm_transition[ABORT],
    },
};

rocket_state_t event_handler(State_t * states, rocket_state_t state)
{
    for(int i = 0; i < MAX_EVENT_SIZE; i++)
    {
        bool (*cond)(void) = states[state].events[i].condition;
        void (*react)(void) = states[state].events[i].reaction;
        int next_state = states[state].events[i].next_state;

        if(cond == NULL)
            continue;

        if(cond())
        {
            if(react != NULL) 
                react();
            if(next_state > -1)
                return next_state;
        }
    }

    return -1;
}

bool exec(State_t * states, rocket_state_t state)
{
    //printf("exec size %d work %x\n", size, work);
    unsigned long end = millis();
    bool change = false;
    for(int i = 0; i < MAX_WORK_SIZE; i++)
    {

        if(states[state].work[i].chanel == NULL)
            continue;

        int msec = (end - states[state].work[i].begin);
        if(msec > states[state].work[i].delay)
        {
            states[state].work[i].begin = end;
            states[state].work[i].chanel(); //execute sample function
            change = true;
        }
    }

    return change;
}