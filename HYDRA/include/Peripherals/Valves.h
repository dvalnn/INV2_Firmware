#ifndef VALVES_H
#define VALVES_H

#include <inttypes.h>
#include "DataModels.h"
#include "Peripherals/IO_Map.h"
#include <Arduino.h>

int valves_setup(void);

void valve_set(data_t *data, uint8_t valve, uint8_t state);

#endif // VALVES_H