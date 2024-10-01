#include <Arduino.h>

#include "HardwareCfg.h"
#include "GlobalVars.h"
#include "StMEvent.h"
#include "StMWork.h"
#include "Utils.h"

uint16_t RP1 = 0, RP2 = 0; //target and trigger
uint16_t RL1 = 0; //target liquid

bool IdleCond(void) { return false; }
bool TrueCond(void) { return true; }

bool prog2_finish_cond(void)
{
    if(tank_pressure < RP1)
    {
        return true;
    }
    return false;
}

bool prog3_finish_cond(void)
{
    if(tank_liquid < RL1)
        return true;
    return false;
}

bool prog1_safety_cond(void)
{
    if(tank_pressure > RP2)
        return true;
    return false;
}

bool safety_stop_cond(void)
{
    if(tank_pressure < RP1)
        return true;
    return false;
}

float he_mol = 0;
float tank_mol_lost = 0;
float last_tank_moles = 0;
void enter_safety_purge(void)
{
    //calc moles insite tank
    float m = calc_moles(Tank_Top_Module.pressure, Tank_Top_Module.temperature);
    last_tank_moles = m;

    V_Vpu_open();
}

void exit_safety_purge(void)
{
    V_Vpu_close();   

    //calc moles insite tank
    float m = calc_moles(Tank_Top_Module.pressure, Tank_Top_Module.temperature);
    tank_mol_lost += last_tank_moles - m;
}

//---------TIMERS---------------
bool arm_timer_event(void)
{
    return (arm_reset_timer > ARM_TIMER_TRIGGER);
}