#ifndef THERMO_H
#define THERMO_H

#include <Adafruit_MAX31856.h>

int thermo_setup(void);

typedef void (*thermo_data_callback)(int thermo_num, float temperature,
                                     void *user_data);
void set_thermo_callback(thermo_data_callback callback, void *user_data);

#endif // THERMO_H
