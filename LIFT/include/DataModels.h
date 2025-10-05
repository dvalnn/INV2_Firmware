#ifndef DATA_MODELS_H
#define DATA_MODELS_H

#include <inttypes.h>
#include <stdbool.h>

typedef enum {
    CMD_STATUS = 0,
    CMD_VALVE_SET,
    CMD_VALVE_MS,
    CMD_ACK,
} cmd_t;

typedef union {
    struct {
        int16_t n2o_bottle_weight;
        int16_t thrust_loadcell1;
        int16_t thrust_loadcell2;
        int16_t thrust_loadcell3;
    };
    int16_t raw[4];
} loadcells_t;

typedef struct {
    loadcells_t loadcells;
} data_t;

#endif // DATA_MODELS_H
