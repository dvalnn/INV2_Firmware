#include <Arduino.h>

#include <time.h>

#include "StateMachine.h"
#include "StMWork.h"
#include "StMEvent.h"
#include "ADSHandler.h"

#include <stdarg.h>
#include <limits.h>

rocket_state_t state = IDLE;

rocket_state_t comm_transition[rocket_state_size][cmd_size] = {
    //                       STATUS LOG ABORT EXEC  STOP   FUELING  MANUAL MAN_EXEC READY  ARM  LAUNCH  RESUME  FIRE
    /* Idle            */ {    -1,  -1, ABORT, -1,   -1,   FUELING, MANUAL,   -1,   READY,  -1,  -1,     -1,     -1,},
    /* Fueling         */ {    -1,  -1, IDLE,  -1,  IDLE,     -1,   MANUAL,   -1,    -1,    -1,  -1,     -1,     -1,},
    /* Manual          */ {    -1,  -1, IDLE,  -1,  IDLE,     -1,     -1,     -1,    -1,    -1,  -1,     -1,     -1,},
    /* Safety_Pressure */ {    -1,  -1, ABORT, -1,  FUELING,  -1,     -1,     -1,    -1,    -1,  -1,     -1,     -1,},
    /* Purge_Pressure  */ {    -1,  -1, ABORT, -1,  FUELING,  -1,     -1,     -1,    -1,    -1,  -1,     -1,     -1,},
    /* Purge_Liquid    */ {    -1,  -1, ABORT, -1,  FUELING,  -1,     -1,     -1,    -1,    -1,  -1,     -1,     -1,},
    /* Safety_Active   */ {    -1,  -1, ABORT, -1,  FUELING,  -1,     -1,     -1,    -1,    -1,  -1,     -1,     -1,},
    /* Ready           */ {    -1,  -1, IDLE,  -1,  IDLE,     -1,     -1,     -1,    -1,   ARMED,-1,     -1,     -1,},
    /* Armed           */ {    -1,  -1, IDLE,  -1,  READY,    -1,     -1,     -1,    -1,    -1, LAUNCH,  -1,     -1,},
    /* Launch          */ {    -1,  -1, ABORT, -1,  IDLE,     -1,     -1,     -1,    -1,    -1,  -1,     -1,     -1,},
    /* Abort           */ {    -1,  -1,  -1,   -1,   -1,      -1,     -1,     -1,    IDLE,  -1,  -1,     -1,     -1,},
    /* IMU PID         */ {    -1,  -1, IDLE,  -1,   -1,      -1,     -1,     -1,    -1,    -1,  -1,     -1,     -1,}};

#define TANK_TEMPERATURE_SENSORS(val) \
    {.channel = read_temperature_tank_top, .sample = val}, \
    {.channel = read_temperature_tank_bot, .sample = val, .delay = 50}

#define TANK_PRESSURE_SENSORS(val) \
    {.channel = read_pressure_tank_top, .sample = val}, \
    {.channel = read_pressure_tank_bot, .sample = val, .delay = 50}


#define CLOSE_VALVES                             \
    {.channel = V_Vpu_close, .sample = 100},     \
    {                                            \
        .channel = V_Engine_close, .sample = 100 \
    }

State_t state_machine[rocket_state_size] =
{
    // IDLE
    {
        .work = {
            //TANK_TEMPERATURE_SENSORS(1000),
            
            CLOSE_VALVES,
            {.channel = read_barometer, .sample = 1000},
            {.channel = read_gps, .sample = 100},
            {.channel = read_imu, .sample = 1000},
            {.channel = kalman, .sample = 1000},

            //{.channel = ADS_handler_slow, .sample = 1},
            {.channel = logger, .sample = 1000},
            {.channel = flash_log_sensors, .sample = 1000, .delay = 200},
        },

        .events = {
            {.condition = ADS_event, .reaction = ADS_reader, .next_state = -1},
        },

        .comms = comm_transition[IDLE],
    },
    // FUELING
    {
        .work = {
            TANK_TEMPERATURE_SENSORS(100),

            {.channel = ADS_handler_fast, .sample = 1},

            {.channel = V_Vpu_close, .sample = 500}, // safety

            CLOSE_VALVES,

            {.channel = logger, .sample = 50},
            {.channel = flash_log_sensors, .sample = 100},
        },

        .events = {
            {.condition = ADS_event, .reaction = ADS_reader, .next_state = -1},
        },

        .comms = comm_transition[FUELING],
    },
    // MANUAL
    {
        .work = {
            TANK_TEMPERATURE_SENSORS(100),

            {.channel = ADS_handler_fast, .sample = 1},


            {.channel = logger, .sample = 50},
            {.channel = flash_log_sensors, .sample = 100},
        },

        .events = {
            {.condition = ADS_event, .reaction = ADS_reader, .next_state = -1},
        },

        .comms = comm_transition[MANUAL],
    },
    // SAFETY_PRESSURE
    {
        .work = {
            TANK_TEMPERATURE_SENSORS(100),

            {.channel = ADS_handler_fast, .sample = 1},

            {.channel = V_Vpu_close, .sample = 500}, // safety
            {.channel = logger, .sample = 50},
            {.channel = flash_log_sensors, .sample = 100},
        },

        .events = {
            {.condition = prog1_safety_cond, .reaction = enter_safety_purge, .next_state = SAFETY_PRESSURE_ACTIVE}, 
            {.condition = ADS_event, .reaction = ADS_reader, .next_state = -1},
        },

        .comms = comm_transition[SAFETY_PRESSURE],
    },
    // PURGE_PRESSURE
    {
        .work = {
            TANK_TEMPERATURE_SENSORS(100),

            {.channel = ADS_handler_fast, .sample = 1},

            {.channel = V_Vpu_open, .sample = 500},
            {.channel = logger, .sample = 50},
            {.channel = flash_log_sensors, .sample = 100},
        },

        .events = {
            {.condition = prog2_finish_cond, .reaction = V_Vpu_close, .next_state = FUELING}, 
            {.condition = ADS_event, .reaction = ADS_reader, .next_state = -1},
        },

        .comms = comm_transition[PURGE_PRESSURE],
    },
    // PURGE_LIQUID
    {
        .work = {
            TANK_TEMPERATURE_SENSORS(100),

            {.channel = ADS_handler_fast, .sample = 1},

            {.channel = V_Vpu_open, .sample = 500},
            {.channel = logger, .sample = 50},
            {.channel = flash_log_sensors, .sample = 100},
        },

        .events = {
            {.condition = prog3_finish_cond, .reaction = V_Vpu_close, .next_state = FUELING}, 
            {.condition = ADS_event, .reaction = ADS_reader, .next_state = -1},
        },

        .comms = comm_transition[PURGE_LIQUID],
    },
    // SAFETY_PRESSURE_ACTIVE
    {
        .work = {
            TANK_TEMPERATURE_SENSORS(100),

            {.channel = ADS_handler_fast, .sample = 1},

            {.channel = V_Vpu_open, .sample = 500},
            {.channel = logger, .sample = 50},
            {.channel = flash_log_sensors, .sample = 100},
        },

        .events = {
            {.condition = safety_stop_cond, .reaction = exit_safety_purge, .next_state = SAFETY_PRESSURE}, 
            {.condition = ADS_event, .reaction = ADS_reader, .next_state = -1},
        },

        .comms = comm_transition[SAFETY_PRESSURE_ACTIVE],
    },
    // READY
    {
        .work = {
            TANK_TEMPERATURE_SENSORS(1000),

            {.channel = ADS_handler_all_slow, .sample = 1},

            {.channel = reset_timers, .sample = 200}, // used to reset the timers used in armed, fire, launch

            {.channel = logger, .sample = 50},
            {.channel = flash_log_sensors, .sample = 100},
        },

        .events = {
            {.condition = ADS_event, .reaction = ADS_reader, .next_state = -1},
        },

        .comms = comm_transition[READY],

    },
    // ARMED
    {
        .work = {
            TANK_TEMPERATURE_SENSORS(1000),
            
            {.channel = ADS_handler_all_fast, .sample = 1},

            {.channel = arm_timer_tick, .sample = 1000},
            {.channel = logger, .sample = 50},
            {.channel = flash_log_sensors, .sample = 100},
        },

        .events = {
            //{.condition = IgniteCond, .reaction = V_Engine_open, .next_state = LAUNCH},
            {.condition = arm_timer_event, .reaction = NULL, .next_state = READY},
            {.condition = ADS_event, .reaction = ADS_reader, .next_state = -1},
        },

        .comms = comm_transition[ARMED],

    },
    // LAUNCH
    {
        .work = {
            {.channel = V_Engine_open, .sample = 5},

            //TANK_TEMPERATURE_SENSORS(1000),

            {.channel = ADS_handler_all_fast, .sample = 1},

            {.channel = logger, .sample = 100},
            {.channel = flash_log_sensors, .sample = 100},
        },

        .events = {
            {.condition = ADS_event, .reaction = ADS_reader, .next_state = -1},
        },

        .comms = comm_transition[LAUNCH],

    },
    // ABORT
    {
        .work = {
            {.channel = V_Engine_close, .sample = 5},
            {.channel = V_Vpu_open, .sample = 5},

            //TANK_TEMPERATURE_SENSORS(250),

            {.channel = ADS_handler_all_fast, .sample = 1},

            {.channel = logger, .sample = 500},
            {.channel = flash_log_sensors, .sample = 500},
        },

        .events = {
            {.condition = ADS_event, .reaction = ADS_reader, .next_state = -1},
        },

        .comms = comm_transition[ABORT],

    },
    // IMU_TUNE
    {
        .work = {
            {.channel = imu_pid_calibration, .sample = 1000},
        },

        .events = {{TrueCond, NULL, IDLE}}, // this will always evaluate true, imu calib is run one time and is automaticly sent to idle

        .comms = comm_transition[IMU_TUNE],
    }
};

rocket_state_t event_handler()
{
    // digitalWrite(TEMP_AMP3_SS_PIN, HIGH);
    rocket_state_t next_state_event = -1;

    for (int i = 0; i < MAX_EVENT_SIZE; i++)
    {
        bool (*cond)(void) = state_machine[state].events[i].condition;
        void (*react)(void) = state_machine[state].events[i].reaction;
        int next_state = state_machine[state].events[i].next_state;

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
    // digitalWrite(TEMP_AMP3_SS_PIN, LOW);

    return next_state_event;
}

bool work_handler()
{
    bool change = false;
    for (int i = 0; i < MAX_WORK_SIZE; i++)
    {
        unsigned long end = millis() - state_machine[state].entry_time;

        if (state_machine[state].work[i].channel == NULL)
            continue;

        unsigned long msec = (end - state_machine[state].work[i].begin);

        // avoid overflows when it produce negative msec
        if (end < state_machine[state].work[i].begin)
            continue;

        if (msec > state_machine[state].work[i].sample)
        {
            state_machine[state].work[i].begin = end;
            state_machine[state].work[i].channel(); // execute sample function
            change = true;
        }
    }

    return change;
}