#include <Arduino.h>

#include "HardwareCfg.h"
#include "GlobalVars.h"
#include "StMEvent.h"
#include "StMWork.h"

uint16_t RP1 = 0, RP2 = 0; //target and trigger
uint16_t RL1 = 0; //target liquid

bool DragDeployed = false;
bool MainDeployed = false;

bool IdleCond(void) { return false; }
bool TrueCond(void) { return true; }

bool prog2_finish_cond(void)
{
    if((int16_t)(Tank_Top_Module.pressure * 100) < RP1 || Tank_Bot_Module.temperature < RP2)
    {
        return true;
    }
    return false;
}

bool prog3_finish_cond(void)
{
    return false;
}

bool prog1_safety_cond(void)
{
    if((int16_t)(Tank_Top_Module.pressure * 100) > RP2)
        return true;
    return false;
}

bool safety_stop_cond(void)
{
    if((int16_t)(Tank_Top_Module.pressure * 100) < RP1)
        return true;
    return false;
}

float he_mol = 0;
float tank_mol_lost = 0;
float last_tank_moles = 0;
void enter_safety_purge(void)
{
    V_Vpu_open();
}

void exit_safety_purge(void)
{
    V_Vpu_close();   
}

//---------TIMERS---------------
bool arm_timer_event(void)
{
    return (arm_reset_timer > ARM_TIMER_TRIGGER);
}

bool motor_shutdown_event(void)
{
    return (burn_timer > MOTOR_BURN_TIMER_TRIGGER) || (Tank_Top_Module.pressure < MIN_TANK_PRESSURE) ;
}

//---------Kalman------------
bool apogee_event(void)
{
    bool return_val = false;

    xSemaphoreTake(kalman_mutex, portMAX_DELAY);
    
    //if(Launch && ((maxAltitude-kalman_altitude)>2) && (kalman_accel < -2.0))
    //if(Launch && ((maxAltitude-kalman_altitude)>10) && (kalman_accel < -2.0))
    if(Launch && ((maxAltitude-kalman_altitude)>2))
    {
      DragDeployed = true;
      preferences.putChar("drag_state", 1);
      return_val = true;
    }

    xSemaphoreGive(kalman_mutex);

    return return_val;
}

bool main_deployment_event(void)
{
    bool return_val = false;

    xSemaphoreTake(kalman_mutex, portMAX_DELAY);
    
    if(DragDeployed && !MainDeployed && kalman_altitude < MAIN_OPENING_ALTITUDE)
    {
        MainDeployed = true;
        preferences.putChar("main_state", 1);
        return_val = true;
    }
    
    xSemaphoreGive(kalman_mutex);
    return return_val;
}
bool touchdown_event(void)
{
    static float last_altitude = 0;
    static uint8_t tick = 0;
    static unsigned long last_read = 0;

    if(millis() - last_read < 100) //read every 100 ms
        return false;

    if(abs(last_altitude - kalman_altitude) > MAX_DELTA_ALTITUDE)
    {
        last_altitude = kalman_altitude;
        last_read = millis();
        tick = 0;
    }
    else
    {
        tick++;
    }

    if(tick > 100) //5 seconds of readings 
    {
        return true;
    }
    else
    {
        return false;
    }
    
}

//---------recovery events-----------

bool depressur_started = false;
bool tank_depressure_start_event(void)
{
    static bool first = true;
    if((depressur_timer > DEPRESSUR_TIMEOUT ||
        depressur_global_timer > DEPRESSUR_GLOBAL_TIMEOUT) &&
        first)
    {
        first = false;
        depressur_started = true;
        return true;
    }

    return false;
}

bool tank_depressure_end_event(void)
{
    return false;
}