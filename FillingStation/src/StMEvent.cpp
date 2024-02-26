#include <Arduino.h>

#include "HardwareCfg.h"

#include "StMEvent.h"


bool IdleCond(void) { return false; }
bool TrueCond(void) { return true; }

bool prog1_finish_cond(void)
{
    return false;
}

bool prog2_finish_cond(void)
{
    return false;
}

bool prog3_finish_cond(void)
{
    return false;
}

bool prog2_stop_cond(void)
{
    return false;
}

void prog2_stop_reaction(void)
{
    return;
}

void close_valves(void)
{
    return;
}