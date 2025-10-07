#ifndef VALVES_H_
#define VALVES_H_

#include "HardwareCfg.h"
#include "GlobalVars.h"
#include "DataModels.h"

#define VALVE_OPEN 1
#define VALVE_CLOSE 0

int valve_set(valve_t valve, int state);

#endif