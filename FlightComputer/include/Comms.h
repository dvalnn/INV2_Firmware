/**
 * @file Comms.h
 * @author AM
 * @brief
 *      Shared file bettewn all the systems,
 *      This way we only have one definition for each command working everywhere
 * @version 0.1
 * @date 2024-02-26
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef _COMMS_H_
#define _COMMS_H_

#include <inttypes.h>
#include <time.h>

#include "HardwareCfg.h"

#define MAX_COMMAND_BUFFER 150
#define RS485_TIMEOUT_TIME_MS 50 // try to get limit bounds
// #define RS485_TIMEOUT_TIME_MS 5000

/*
    Read command errors
 */
#define CMD_READ_OK 0
#define CMD_READ_TIMEOUT 1
#define CMD_READ_BAD_CRC 2
#define CMD_READ_DEFAULT_ERROR 3
#define CMD_READ_NO_CMD 4

/*
    Run command errors
 */
#define CMD_RUN_OK 0
#define CMD_RUN_ARM_ERROR 1
#define CMD_RUN_DEFAULT_ERROR 2
#define CMD_RUN_NO_ACTION 3
#define CMD_RUN_OUT_OF_BOUND 4
#define CMD_RUN_STATE_ERROR 5

/*
    Arm Stages
*/
#define ARM_TRIGGER_1 1
#define ARM_TRIGGER_2 2
#define ARM_TRIGGER_3 3


/*
    Sensors bit mask
*/
#define BIT(x) (1 << (x))





//----------------------------------------

typedef struct __attribute__((__packed__))
// typedef struct
{
    uint8_t cmd;
    uint8_t id;
    uint8_t size;
    uint8_t data[MAX_COMMAND_BUFFER];
    uint16_t crc;

    uint8_t data_recv; // helper pointer to fill data[]
    unsigned long begin;
} command_t;

typedef enum
{
    SYNC = 0,
    CMD,
    ID,
    SIZE,
    DATA,
    CRC1, // first byte of crc
    CRC2, // second byte of crc
    END,
} COMMAND_STATE;

typedef enum
{
    LORA_INTERFACE,
    RS485_INTERFACE,
    UART_INTERFACE,
    interface_t_size
} interface_t;

//#define DEFAULT_CMD_INTERFACE UART_INTERFACE
#define DEFAULT_CMD_INTERFACE LORA_INTERFACE
// #define DEFAULT_CMD_INTERFACE RS485_INTERFACE

#define DEFAULT_LOG_INTERFACE RS485_INTERFACE
// #define DEFAULT_LOG_INTERFACE LORA_INTERFACE

#define DEFAULT_SYNC_INTERFACE UART_INTERFACE

#define CRC_ENABLED false

#define GROUND_ID 0
#define OBC_ID 1
#define HYDRA_UF_ID 2
#define HYDRA_LF_ID 3
#define HYDRA_FS_ID 4
#define NAVIGATOR_ID 5
#define BROADCAST_ID 0xFF

#define DEFAULT_ID OBC_ID


void write_command(command_t *cmd, interface_t interface);
command_t *read_command(int *error, interface_t interface);

#endif