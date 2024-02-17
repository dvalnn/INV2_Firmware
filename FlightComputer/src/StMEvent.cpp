#include <Arduino.h>

#include "HardwareCfg.h"

#include "StMEvent.h"


bool IdleCond(void) { return false; }
bool TrueCond(void) { return true; }

bool IgniteCond(void)
{
    if(digitalRead(TRIGGER) == LOW)
        return true;
    return false;
}

void IgniteReaction()
{
    return;
}