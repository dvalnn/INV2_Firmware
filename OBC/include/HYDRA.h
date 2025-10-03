#ifndef HYDRA_H
#define HYDRA_H

#include <stdint.h>
#include <stdbool.h>
#include "Comms.h"
#include "DataModels.h"

typedef enum {
    HYDRA_UF = 1,
    HYDRA_LF,
    HYDRA_FS,
    hydra_id_count,
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

typedef union {
    uint8_t raw;
    struct {
        uint8_t v_quick_dc_1 : 1;
        uint8_t v_quick_dc_2 : 1;
        uint8_t v_controlled_1 : 1;
        uint8_t v_controlled_2 : 1;
        uint8_t v_controlled_3 : 1;
        uint8_t v_steel_ball_1 : 1;
        uint8_t v_steel_ball_2 : 1;
        uint8_t v_servo : 1;
    };
} hydra_valve_states_t;

typedef struct {
    uint16_t thermo1, thermo2, thermo3;
    uint16_t pressure1, pressure2, pressure3;
    hydra_valve_states_t valve_states;
    bool cam_enable;
} hydra_data_t;

typedef struct {
    hydra_id_t id;
    hydra_data_t data;
} hydra_t;

void init_hydra(hydra_t *hydra);
int read_hydra(hydra_t *hydra);
int set_hydra_valve(hydra_t *hydra, hydra_valve_t valve, bool state);
int set_hydra_valve_ms(hydra_t *hydra, hydra_valve_t valve, uint16_t ms);

int send_hydra_command(hydra_t *hydra, hydra_cmd_t cmd, uint8_t *payload, uint8_t payload_size);
int parse_hydra_response(hydra_t *hydra, packet_t *packet);
    
#endif // HYDRA_H