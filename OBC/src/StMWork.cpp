#include <Arduino.h>

#include <time.h>

#include "HardwareCfg.h"
#include "StMWork.h"
#include "StMComms.h"
#include "GlobalVars.h"
#include "Comms.h"
#include "FlashLog.h"

#include <Crc.h>

void flash_log_sensors(void)
{
    log(NULL, 0, SENSOR_READING);
}

void timer_tick(uint16_t *timer) { (*timer)++; }
