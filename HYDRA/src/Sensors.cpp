#include "Sensors.h"

elapsedMillis timer;

void read_sensors(data_t *data) {
    // For each sensor, check if ready and read value
    if(timer < 500)
        return;

    read_adc_channels(data);
    timer = 0;
}
