#include "Peripherals/Valves.h"

int valves_setup(void) {
    pinMode(SOL_VALVE_1_PIN, OUTPUT);
    pinMode(SOL_VALVE_2_PIN, OUTPUT);
    pinMode(SOL_VALVE_3_PIN, OUTPUT);
    pinMode(QDC_VALVE_1_PIN, OUTPUT);
    pinMode(QDC_VALVE_2_PIN, OUTPUT);
    pinMode(ST_VALVE_1_PIN, OUTPUT);
    pinMode(ST_VALVE_2_PIN, OUTPUT);
    pinMode(CAM_EN_PIN, OUTPUT);
    pinMode(PWM_SIG_PIN, OUTPUT);
    return 0;
}

void valve_set(data_t *data, uint8_t valve, uint8_t state) {
    switch (valve) {
        case VALVE_QUICK_DC_1:
            digitalWrite(QDC_VALVE_1_PIN, state);
            data->valve_states.v_quick_dc_1 = state;
            break;
        case VALVE_QUICK_DC_2:
            digitalWrite(QDC_VALVE_2_PIN, state);
            data->valve_states.v_quick_dc_2 = state;
            break;
        case VALVE_CONTROLLED_1:
            digitalWrite(SOL_VALVE_1_PIN, state);
            data->valve_states.v_controlled_1 = state;
            break;
        case VALVE_CONTROLLED_2:
            digitalWrite(SOL_VALVE_2_PIN, state);
            data->valve_states.v_controlled_2 = state;
            break;
        case VALVE_CONTROLLED_3:
            digitalWrite(SOL_VALVE_3_PIN, state);
            data->valve_states.v_controlled_3 = state;
            break;
        case VALVE_STEEL_BALL_1:
            digitalWrite(ST_VALVE_1_PIN, state);
            data->valve_states.v_steel_ball_1 = state;
            break;
        case VALVE_STEEL_BALL_2:
            digitalWrite(ST_VALVE_2_PIN, state);
            data->valve_states.v_steel_ball_2 = state;
            break;
        case VALVE_SERVO:
            // TODO: Implement servo control if needed
            break;
        default:
            // Invalid valve
            break;
    }
}