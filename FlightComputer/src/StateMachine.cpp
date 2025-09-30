#include <Arduino.h>

#include <time.h>

#include "StateMachine.h"
#include "StMWork.h"
#include "StMEvent.h"

#include <stdarg.h>
#include <limits.h>

extern system_data_t system_data;

state_t expected_state[state_count][cmd_size] = {
    //                       STATUS_REQ ABORT  READY     ARM       FIRE      LAUNCH_OVERRIDE FILL_EXEC FILL_RESUME MANUAL_EXEC STOP   ACK
    /* IDLE */             { IDLE,      ABORT, READY,    IDLE,     IDLE,     IDLE,           FILLING,  FILLING,    IDLE,       IDLE,  IDLE }, 
    /* FILLING */          { FILLING,   ABORT, FILLING,  FILLING,  FILLING,  FILLING,        FILLING,  FILLING,    FILLING,    IDLE,  FILLING }, 
    /* READY */            { READY,     ABORT, READY,    ARMED,    READY,    READY,          READY,    READY,      READY,      IDLE,  READY }, 
    /* ARMED */            { ARMED,     ABORT, ARMED,    ARMED,    IGNITION, ARMED,          ARMED,    ARMED,      ARMED,      IDLE,  ARMED }, 
    /* IGNITION */         { IGNITION,  ABORT, IGNITION, IGNITION, IGNITION, LAUNCH,         IGNITION, IGNITION,   IGNITION,   IDLE,  IGNITION }, 
    /* LAUNCH */           { LAUNCH,    ABORT, LAUNCH,   LAUNCH,   LAUNCH,   LAUNCH,         LAUNCH,   LAUNCH,     LAUNCH,     IDLE,  LAUNCH }, 
    /* FLIGHT */           { FLIGHT,    ABORT, FLIGHT,   FLIGHT,   FLIGHT,   FLIGHT,         FLIGHT,   FLIGHT,     FLIGHT,     IDLE,  FLIGHT },
    /* RECOVERY */         { RECOVERY,  ABORT, RECOVERY, RECOVERY, RECOVERY, RECOVERY,       RECOVERY, RECOVERY,   RECOVERY,   IDLE,  RECOVERY }, 
    /* ABORT */            { ABORT,     ABORT, IDLE,     ABORT,    ABORT,    ABORT,          ABORT,    ABORT,      ABORT,      IDLE,  ABORT }, 
};

sm_state_t state_machine[state_count] =
{
    // IDLE
    {
        .work = {
        },

        .events = {
        },

        .next_states = expected_state[IDLE],
    },
    // FILLING
    {
        .work = {
        },

        .events = {
        },

        .next_states = expected_state[FILLING],
    },
    // READY
    {
        .work = {
        },

        .events = {
        },

        .next_states = expected_state[READY],

    },
    // ARMED
    {
        .work = {

        },

        .events = {

        },

        .next_states = expected_state[ARMED],

    },
    // IGNITION
    {
        .work = {

        },

        .events = {

        },

        .next_states = expected_state[IGNITION],

    },
    // LAUNCH
    {
        .work = {
        },

        .events = {
        },

        .next_states = expected_state[LAUNCH],

    },
    
    // FLIGHT
    {
        .work = {
        },

        .events = {
        }, 

        .next_states = expected_state[FLIGHT],
    },
    // RECOVERY 
    {
        .work = {
        },

        .events = {
        }, 

        .next_states = expected_state[FLIGHT],
    },
    // ABORT
    {
        .work = {
        },

        .events = {
        },

        .next_states = expected_state[ABORT],

    },
};

state_t event_handler()
{
    state_t next_state_event = S_NONE;

    for (int i = 0; i < MAX_EVENT_SIZE; i++)
    {
        bool (*cond)(void) = state_machine[system_data.state].events[i].condition;
        void (*react)(void) = state_machine[system_data.state].events[i].reaction;
        state_t next_state = state_machine[system_data.state].events[i].next_state;

        if (cond == NULL)
            continue;

        if (cond())
        {
            if (react != NULL)
                react();
            if (next_state > -1 && next_state_event == -1)
                next_state_event = next_state;
        }
    }

    return next_state_event;
}

bool work_handler()
{
    bool change = false;
    for (int i = 0; i < MAX_WORK_SIZE; i++)
    {
        unsigned long end = millis() - state_machine[system_data.state].entry_time;

        if (state_machine[system_data.state].work[i].channel == NULL)
            continue;

        unsigned long msec = (end - state_machine[system_data.state].work[i].begin);

        // avoid overflows when it produce negative msec
        if (end < state_machine[system_data.state].work[i].begin)
            continue;

        if (msec > state_machine[system_data.state].work[i].sample)
        {
            state_machine[system_data.state].work[i].begin = end;
            state_machine[system_data.state].work[i].channel(); // execute sample function
            change = true;
        }
    }

    return change;
}