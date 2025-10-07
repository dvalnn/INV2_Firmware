#include "Sensors.h"
#include "HardwareCfg.h"

clock_t last_loadcell_poll = 0;
clock_t last_thermo_poll = 0;

void read_sensors(data_t *data) {
    // For each sensor, check if ready and read value
    if((millis() - last_loadcell_poll) > LOADCELL_POLL_INTERVAL) {
        read_loadcells(data);
        last_loadcell_poll = millis();
    }
}
