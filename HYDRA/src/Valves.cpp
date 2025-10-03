#include "Valves.h"
#include "IO_Map.h"
#include <Arduino.h>

int valves_setup(void) {
    pinMode(SOL_VALVE_1, OUTPUT);
    pinMode(SOL_VALVE_2, OUTPUT);
    pinMode(SOL_VALVE_3, OUTPUT);
    pinMode(QDC_N2O, OUTPUT);
    pinMode(QDC_N2, OUTPUT);
    pinMode(ST_VALVE_1, OUTPUT);
    pinMode(ST_VALVE_2, OUTPUT);
    pinMode(CAM_EN, OUTPUT);
    pinMode(PWM_SIG, OUTPUT);

    return 0;
}