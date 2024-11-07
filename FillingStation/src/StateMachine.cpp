#include <Arduino.h>

#include <time.h>

#include "StateMachine.h"
#include "StMWork.h"
#include "StMEvent.h"

#include <stdarg.h>
#include <limits.h>

rocket_state_t state = IDLE; 

rocket_state_t comm_transition[rocket_state_size][cmd_size] = {  
//                  STATUS LOG ABORT EXEC  STOP   FUELING  MANUAL MAN_EXEC READY  ARM  LAUNCH  RESUME  FIRE  
/* Idle       */  {   -1,  -1, ABORT, -1,   -1,   FUELING, MANUAL,   -1,   READY, -1,    -1,      -1,   -1,  },
/* Fueling    */  {   -1,  -1, IDLE,  -1,   IDLE,   -1,    MANUAL,   -1,    -1,   -1,    -1,      -1,   -1,  },
/* Manual     */  {   -1,  -1, IDLE,  -1,   IDLE,   -1,     -1,      -1,   -1,   -1,    -1,      -1,   -1,  },
/* FILL_He    */  {   -1,  -1, ABORT, -1, FUELING,  -1,     -1,      -1,    -1,   -1,    -1,      -1,   -1,  },
/* FILL_N2O   */  {   -1,  -1, ABORT, -1, FUELING,  -1,     -1,      -1,    -1,   -1,    -1,      -1,   -1,  },
/* PURGE_LINE */  {   -1,  -1, ABORT, -1, FUELING,  -1,     -1,      -1,    -1,   -1,    -1,      -1,   -1,  },
/* Stop       */  {   -1,  -1, ABORT, -1, FUELING,  -1,     -1,      -1,    -1,   -1,    -1,  FILL_N2O, -1,  },
/* Abort      */  {   -1,  -1,  -1,   -1,   IDLE,   -1,     -1,      -1,   IDLE,  -1,    -1,      -1,   -1,  },
/* Ready      */  {   -1,  -1, IDLE,  -1,   IDLE,   -1,     -1,      -1,    -1,  ARMED,  -1,      -1,   -1,  },
/* Armed      */  {   -1,  -1, IDLE,  -1,  READY,   -1,     -1,      -1,    -1,   -1,    -1,      -1,  FIRE, },
/* FIRE       */  {   -1,  -1, ABORT, -1,  READY,   -1,     -1,      -1,    -1,   -1,    -1,      -1,   -1,  },
/* Launch     */  {   -1,  -1, ABORT, -1,   IDLE,   -1,     -1,      -1,    -1,   -1,    -1,      -1,   -1,  },
};
/*
 EXEC_CMD state is set inside the command function so it can have 3 diferent exits, [prog1, prog2, prog3]
 this also help with safety, the cmand function verifys the state that executed the command
*/

#define HYDROLIC_TEMPERATURE_SENSORS(val) \
        {.channel = read_temperature_He, .sample = val}, \
        {.channel = read_temperature_N2O, .sample = val}, \
        {.channel = read_temperature_Line, .sample = val}

#define HYDROLIC_PRESSURE_SENSORS(val) \
        {.channel = read_pressure_N2O, .sample = val}, \
        {.channel = read_pressure_Line, .sample = val}
        //{.channel = read_pressure_He, .sample = val}, \

#define CLOSE_VALVES \
        {.channel = V_He_close, .sample = 100}, \
        {.channel = V_N2O_close, .sample = 100}, \
        {.channel = V_Line_close, .sample = 100} 

State_t state_machine[rocket_state_size] = 
{
    //IDLE
    {
        .work = 
        { 
            //hydrolic system temperature and pressures
            HYDROLIC_TEMPERATURE_SENSORS(1000),
            {.channel = module_pressure_reader_slow, .sample = 1},

            //oxidizer bottle weight
            {.channel = read_N20_weight, .sample = 1000},

            //continuity check
            {.channel = read_ematch, .sample = 1000},

            //logging and bus comunication
            {.channel = echo_reply, .sample = 5}, 
            {.channel = flash_log_sensors, .sample = 1000, .delay = 1000}, 
        },

        .events = {},

        .comms = comm_transition[IDLE],
    },
    //FUELING
    {
        .work = 
        { 
            HYDROLIC_TEMPERATURE_SENSORS(250),
            {.channel = module_pressure_reader_fast, .sample = 1},

            CLOSE_VALVES,
            
            {.channel = read_N20_weight, .sample = 100},
            
            {.channel = echo_reply, .sample = 5}, 
            {.channel = flash_log_sensors, .sample = 100}, 
        },

        .events = {},

        .comms = comm_transition[FUELING],
    },
    //MANUAL
    {
        .work = 
        { 
            HYDROLIC_TEMPERATURE_SENSORS(250),
            {.channel = module_pressure_reader_fast, .sample = 1},

            {.channel = read_N20_weight, .sample = 100},
            
            {.channel = echo_reply, .sample = 5}, 
            {.channel = flash_log_sensors, .sample = 100}, 
        },

        .events = {
            {.condition = heart_beat_signal, .reaction = NULL, .next_state = FUELING},
        },

        .comms = comm_transition[MANUAL],
    },
    //FILL_He
    {
        .work = 
        { 
            HYDROLIC_TEMPERATURE_SENSORS(250),
            {.channel = module_pressure_reader_fast, .sample = 1},

            {.channel = read_N20_weight, .sample = 100},
            
            {.channel = V_He_open, .sample = 500},
            
            {.channel = echo_reply, .sample = 5}, 
            {.channel = flash_log_sensors, .sample = 100}, 
        },

        .events = 
        { 
            {.condition = prog1_finish_cond, .reaction = V_He_close, .next_state = FUELING},
            {.condition = heart_beat_signal, .reaction = NULL, .next_state = FUELING},
        },

        .comms = comm_transition[FILL_He],
    },
    //FILL_N2O
    {
        .work = 
        { 
            HYDROLIC_TEMPERATURE_SENSORS(250),
            {.channel = module_pressure_reader_fast, .sample = 1},

            {.channel = read_N20_weight, .sample = 100},
            
            {.channel = V_N2O_open, .sample = 500}, 
            
            {.channel = echo_reply, .sample = 5}, 
            {.channel = flash_log_sensors, .sample = 100}, 
        },

        .events = 
        { 
            {.condition = prog2_stop_cond, .reaction = V_N2O_close, .next_state = STOP},
            {.condition = prog2_finish_cond, .reaction = V_N2O_close, .next_state = FUELING},
            {.condition = heart_beat_signal, .reaction = NULL, .next_state = FUELING},
        },

        .comms = comm_transition[FILL_N2O],
    },
    //PURGE_LINE
    {
        .work = 
        { 
            HYDROLIC_TEMPERATURE_SENSORS(250),
            {.channel = module_pressure_reader_fast, .sample = 1},

            {.channel = read_N20_weight, .sample = 100},
            
            {.channel = V_Line_open, .sample = 500},
            
            {.channel = echo_reply, .sample = 5}, 
            {.channel = flash_log_sensors, .sample = 100}, 
        },

        .events = 
        { 
            {.condition = prog3_finish_cond, .reaction = V_Line_close, .next_state = FUELING}, 
            {.condition = heart_beat_signal, .reaction = NULL, .next_state = FUELING},
        },

        .comms = comm_transition[PURGE_LINE],
    },
    //STOP
    {
        .work = 
        { 
            HYDROLIC_TEMPERATURE_SENSORS(250),
            {.channel = module_pressure_reader_fast, .sample = 1},

            {.channel = read_N20_weight, .sample = 100},
            
            {.channel = V_N2O_close, .sample = 500}, 
            
            {.channel = echo_reply, .sample = 5}, 
            {.channel = flash_log_sensors, .sample = 100}, 
        },

        .events = {
            {.condition = heart_beat_signal, .reaction = NULL, .next_state = FUELING},
        },

        .comms = comm_transition[STOP],
    },
    //ABORT
    {
        .work = 
        { 
            HYDROLIC_TEMPERATURE_SENSORS(250),
            {.channel = module_pressure_reader_slow, .sample = 1},

            CLOSE_VALVES,     
            {.channel = ematch_low, .sample = 1000, .delay = 0},
            
            {.channel = echo_reply, .sample = 5}, 
            {.channel = flash_log_sensors, .sample = 100}, 
        },

        .events = {},

        .comms = comm_transition[ABORT],
    },
    //READY
    {
        .work = 
        { 
        },
                

        .events = {},

        .comms = comm_transition[READY],

    },
    //ARMED
    {
        .work = 
        { 
        },

        .events = 
        { 
        },

        .comms = comm_transition[ARMED],

    },
    //FIRE
    {
        .work = 
        { 
        },

        .events = 
        {
        },

        .comms = comm_transition[FIRE],

    },
    //LAUNCH
    {
        .work = 
        {
        },

        .events = 
        {
        },

        .comms = comm_transition[LAUNCH],

    },

};

rocket_state_t event_handler()
{
    for(int i = 0; i < MAX_EVENT_SIZE; i++)
    {
        bool (*cond)(void) = state_machine[state].events[i].condition;
        void (*react)(void) = state_machine[state].events[i].reaction;
        int next_state = state_machine[state].events[i].next_state;

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

bool work_handler()
{
    bool change = false;
    for(int i = 0; i < MAX_WORK_SIZE; i++)
    {
        unsigned long end = millis() - state_machine[state].entry_time;

        if(state_machine[state].work[i].channel == NULL)
            continue;

        //avoid overflows when it produce negative msec
        if(end < state_machine[state].work[i].begin)
            continue;

        unsigned long msec = (end - state_machine[state].work[i].begin);
        if(msec > state_machine[state].work[i].sample)
        {
            state_machine[state].work[i].begin = end;
            state_machine[state].work[i].channel(); //execute sample function
            change = true;
        }
    }

    return change;
}