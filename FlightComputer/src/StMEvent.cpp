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

bool motor_timer_event(void)
{
    return (burn_timer > MOTOR_BURN_TIMER_TRIGGER);
}

//---------Kalman------------
bool apogee_event(void)
{
    if(Launch && ((maxAltitude-alt_kalman_state(6))>0.5) &&(abs(alt_kalman_state(7))<0.1))
    {
      DragDeployed = true;
      preferences.putChar("drag_state", 1);
      return true;
    }
    return false;
}

bool main_deployment_condition(void)
{
    if(DragDeployed && !MainDeployed && alt_kalman_state(6) < 400)
    {
        MainDeployed = true;
        preferences.putChar("main_state", 1);
        return true;
    }
    return false;
}