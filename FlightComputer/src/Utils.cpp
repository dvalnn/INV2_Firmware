#include "Utils.h"
#include "GlobalVars.h"

const float r = 8.314;
float calc_moles(float pressure, int16_t temperature)
{
    return (pressure * 1e5 * V_total) / (r * ((temperature / 10.0) + 274.15));
}