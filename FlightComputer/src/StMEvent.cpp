#include <Arduino.h>

#include "HardwareCfg.h"
#include "GlobalVars.h"
#include "StMEvent.h"

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

bool IgniteCond(void)
{
    if(Chamber_Module.temperature3 > CHAMBER_TEMP_THREASHOLD)
        return true;
    return false;
}

//---------TIMERS---------------
bool arm_timer_event(void)
{
    return (arm_reset_timer > ARM_TIMER_TRIGGER);
}