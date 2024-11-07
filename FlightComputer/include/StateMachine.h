#ifndef STATE_MACHINE_H_
#define STATE_MACHINE_H_

#include "Comms.h"

#define MAX_WORK_SIZE 15
#define MAX_EVENT_SIZE 10

#define ARM_TIMER_TRIGGER 60

#define MOTOR_BURN_TIMER_TRIGGER 82 //0.1sec 

#define MIN_TANK_PRESSURE 30.0

#define CHAMBER_TEMP_THREASHOLD 6000 // 0.1c units

#define MAIN_OPENING_ALTITUDE 450

#define MAX_DELTA_ALTITUDE 1 

#define MAX_RESTART_ALLOWED 6

#define DEPRESSUR_TIMEOUT 10
#define DEPRESSUR_GLOBAL_TIMEOUT 35

//use the enum below as the values of rocket_state_t
//use -1 for default behavior
typedef int8_t rocket_state_t;
enum 
{
    IDLE,
    
    FUELING,
    MANUAL,

    SAFETY_PRESSURE,
    PURGE_PRESSURE,
    PURGE_LIQUID,
    
    SAFETY_PRESSURE_ACTIVE,

    READY,
    ARMED,
    LAUNCH,

    ABORT,

    FLIGHT,

    RECOVERY,

    rocket_state_size, //this needs to be the last state for size to work
} rocket_state;

typedef struct 
{
    bool (*condition)(void);
    void (*reaction)(void);
    rocket_state_t next_state;
} Event_t;

typedef struct
{
    void (*channel)(void);
    unsigned long sample; //millis of delay between samples
    unsigned long delay; //millis phase for reading sensors
    unsigned long begin; //millis of last sensor reading
} Work_t;

typedef struct
{
    Work_t work[MAX_WORK_SIZE];
    Event_t events[MAX_EVENT_SIZE];
    rocket_state_t *comms;

    //used as the time base when dealing with sensor sampling rate and delays
    unsigned long entry_time; 
} State_t;

extern rocket_state_t state; 
extern rocket_state_t comm_transition[rocket_state_size][cmd_size]; //save transition state for communication
extern State_t state_machine[rocket_state_size]; 


rocket_state_t event_handler();
bool work_handler();
#define WORK_HANDLER() work_handler()
#define EVENT_HANDLER() event_handler()


#endif