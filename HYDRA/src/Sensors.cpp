#include "Sensors.h"

clock_t last_sensor_poll = 0;

void read_sensors(data_t *data) {
    // For each sensor, check if ready and read value
    if((millis() - last_sensor_poll) < 500)
        return;

    read_adc_channels(data);
    last_sensor_poll = millis();
}
