#include <Arduino.h>

#include <inttypes.h>
#include <time.h>
#include <string.h>

#include "Target.h"
#include "Comms.h"

void write_command(command_t* cmd)
{
    int size = 0;
    uint8_t buff[100] = {0};
    
    buff[size++] = 0x55;
    buff[size++] = cmd->cmd;
    buff[size++] = cmd->size;
    for(int i = 0; i < cmd->size; i++)
        buff[size++] = cmd->data[i];
    buff[size++] = ((cmd->crc >> 8) & 0xff);
    buff[size++] = ((cmd->crc) & 0xff);

#ifndef DIGITAL_TARGET
    Serial2.write(buff, size);
#else
    Serial.write(buff, size);
#endif
}

command_t* read_command(int* error)
{
    static COMMAND_STATE state = SYNC;
    static command_t command;
    static uint8_t data_recv;
    static clock_t begin = 0, end = 0;
    
    size_t size;
    uint8_t read_byte;

#ifdef DIGITAL_TARGET
    /*
        Used for testing. See Target.h
    */
    while(Serial.available())
    {
        read_byte = Serial.read();
#else
    while(Serial2.available())
    {
        read_byte = Serial2.read();
#endif
        //printf("data %x\n", read_byte);
        switch(state)
        {
            case SYNC:
                if(read_byte == 0x55)
                {
                    //start timeout timer
                    state = CMD;
                    data_recv = 0;
                    memset(&command, 0, sizeof(command_t));
                    begin = clock();
                }
            break;

            case CMD:
                command.cmd = (cmd_type_t)read_byte;
                state = SIZE;
            break;

            case SIZE:                
                command.size = read_byte;
                if(command.size == 0)
                    state = CRC1;
                else state = DATA;
            break;

            case DATA:
                command.data[data_recv++] = read_byte;
                if(data_recv == command.size)
                    state = CRC1;
            break;

            case CRC1:
                command.crc = read_byte << 8;
                state = CRC2;
            break;

            case CRC2:
                command.crc += read_byte;
                state = END;
            break;

            default:
                state = SYNC;
        };
    }

    end = clock();
    int msec = (end - begin) * 1000 / CLOCKS_PER_SEC;

    //if timeout reset state
    if(state != SYNC && msec > RS485_TIMEOUT_TIME_MS) //timeout
    {
        Serial.printf("reset\n"); //debug
        state = SYNC;
        
        *error = CMD_READ_TIMEOUT;

        return NULL;
    }
    //if bad cr reset state
    else if(state == END /* && check_crc(&command) */)
    {
        state = SYNC;

        *error = CMD_READ_OK;
        return &command;
    }
    else if(state == END)
    {
        state = SYNC;

        *error = CMD_READ_BAD_CRC;
        return NULL;
    }
    else //default
    {
        *error = CMD_READ_NO_CMD;
        return NULL;
    }
}