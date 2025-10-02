#ifndef HYDRA_H
#define HYDRA_H

#include <stdint.h>

typedef enum {
    HYDRA_UF = 0,
    HYDRA_LF,
    HYDRA_FS,
} hydra_id_t;

typedef enum {
    HCMD_STATUS = 0,
    HCMD_VALVE_SET,
    HCMD_VALVE_MS,
    HCMD_ACK,
} hydra_cmd_t;

typedef enum {
    VALVE_QUICK_DC_1 = 0,
    VALVE_QUICK_DC_2,
    VALVE_CONTROLLED_1,
    VALVE_CONTROLLED_2,
    VALVE_CONTROLLED_3,
    VALVE_STEEL_BALL_1,
    VALVE_STEEL_BALL_2,
    VALVE_SERVO,
    hydra_valve_count,
} hydra_valve_t;

typedef struct {
    uint16_t thermo1, thermo2, thermo3;
    uint16_t pressure1, pressure2, pressure3;
    bool valve_states[hydra_valve_count];
    bool cam_enable;
} hydra_data_t;

typedef struct {
    hydra_id_t id;
    hydra_data_t data;
} hydra_t;

#endif // HYDRA_H