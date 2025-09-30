#include <Arduino.h>

#include <inttypes.h>
#include <time.h>
#include <string.h>

#include "Comms.h"

#include "FlashLog.h"
#include "GlobalVars.h"

#include <Crc.h>
#include <LoRa.h>

bool check_crc(packet_t* packet);

void write_packet(packet_t* packet, interface_t interface)
{
    int size = 0;
    uint8_t buff[MAX_PAYLOAD_SIZE + 5] = {0};
    
    buff[size++] = 0x55;
    buff[size++] = packet->cmd;
    buff[size++] = packet->sender_id;
    buff[size++] = packet->target_id;
    buff[size++] = packet->payload_size;
    for(int i = 0; i < packet->payload_size; i++)
        buff[size++] = packet->payload[i];
    buff[size++] = ((packet->crc >> 8) & 0xff);
    buff[size++] = ((packet->crc) & 0xff);

    //log_command(cmd);
    
    switch(interface)
    {
        case LORA_INTERFACE:

            break;
        case RS485_INTERFACE:
            Serial2.write(buff, size);
            break;
        case UART_INTERFACE:
            Serial.write(buff, size);
            break;
        default:
            break;
    }
}

static cmd_parse_state_t parse_input(uint8_t read_byte, packet_t* command, cmd_parse_state_t cmd_state)
{
    uint8_t state = cmd_state;

    //printf("data %x\n", read_byte);
    switch(state)
    {
        case SYNC:
        {
            Serial.printf("Sync byte %x\n\r", read_byte);
            if(read_byte == SYNC_BYTE)
            {
                //start timeout timer
                state = CMD;
                command->data_recv = 0;
                memset(command, 0, sizeof(packet_t));
                command->begin = clock();
            }
        }        
        break;

        case CMD:
        {
            Serial.printf("CMD byte %x\n\r", read_byte);
            command->cmd = read_byte;
            state = SENDER_ID;
        }
        break;

        case SENDER_ID:
        {
            Serial.printf("ID byte %x\n\r", read_byte);
            command->sender_id = read_byte;
            state = TARGET_ID;
        }
        break;

        case TARGET_ID:
        {
            Serial.printf("ID byte %x\n\r", read_byte);
            command->target_id = read_byte;
            state = PAYLOAD_SIZE;
        }
        break;

        case PAYLOAD_SIZE:
        {
            Serial.printf("SIZE byte %x %d\n\r", read_byte, read_byte);
            command->payload_size = read_byte;
            if(command->payload_size == 0)
                state = CRC1;
            else state = PAYLOAD;
        }
        break;

        case PAYLOAD:
        {
            Serial.printf("DATA byte %x\n\r", read_byte);
            command->payload[command->data_recv++] = read_byte;
            if(command->data_recv == command->payload_size)
                state = CRC1;
        }
        break;

        case CRC1:
        {
            Serial.printf("CRC1 byte %x\n\r", read_byte);
            command->crc = read_byte << 8;
            state = CRC2;
        }
        break;

        case CRC2:
        {
            Serial.printf("CRC2 byte %x\n\r", read_byte);
            command->crc += read_byte;
            state = END;
        }
        break;

        default:
            state = SYNC;
    };

    return (cmd_parse_state_t)state;
}

packet_t* read_packet(int* error, interface_t interface)
{
    static packet_t command_arr[interface_t_size];
    static cmd_parse_state_t state_arr[interface_t_size] = {SYNC};
    static clock_t end_arr[interface_t_size] = {0};
    
    uint8_t index = (uint8_t)interface;
    packet_t *command = &command_arr[index];
    cmd_parse_state_t *state = &state_arr[index];
    clock_t *end = &end_arr[index];

    size_t size;
    uint8_t read_byte;

    switch(interface)
    {
        case LORA_INTERFACE:
        {
            //Work arourd for lora to not spam the spi bus
            static unsigned int begin = 0, end = 0;
            end = millis();
            if(end - begin < 5) break;
            begin = end;
            
            int packetSize = LoRa.parsePacket();
            if(packetSize > 0) Serial.printf("packet recived %d\n", packetSize);
            while(packetSize != 0 && LoRa.available() && *state != END)
            {
                read_byte = LoRa.read();
                //printf("got byte %x\n", read_byte);
                *state = parse_input(read_byte, command, *state);
            }
        }
        break;

        case RS485_INTERFACE:
        {
            while(Serial2.available() && *state != END)
            {
                read_byte = Serial2.read();
                *state = parse_input(read_byte, command, *state);
            }
        }
        break;
        
        case UART_INTERFACE:
        {
            while(Serial.available() && *state != END)
            {
                read_byte = Serial.read();
                *state = parse_input(read_byte, command, *state);
            }
        }
        break;

        default:
        break;
    };

    *end = clock();
    int msec = (*end - command->begin) * 1000 / CLOCKS_PER_SEC;

    //if timeout reset state
    if(*state != SYNC && msec > RS485_TIMEOUT_TIME_MS) //timeout
    {
        Serial.printf("TIMEOUT\n\r"); //debug
        *state = SYNC;
        
        *error = CMD_READ_TIMEOUT;

        return NULL;
    }
    //if bad cr reset state
    else if(*state == END && 
            (command->target_id == DEFAULT_ID || 
             command->target_id == BROADCAST_ID )
             && (check_crc(command) || ! CRC_ENABLED) )
    {
        Serial.printf("got message %d %d\n\r", command->cmd, command->target_id);
        *state = SYNC;

        *error = CMD_READ_OK;

        if(interface == DEFAULT_CMD_INTERFACE)
            log(command, 0, MSG_RECEIVED);
        
        return command;
    }
    else if(*state == END)
    {
        Serial.printf("crc error %d %d %d\n\r", command->target_id, command->crc, (uint16_t)crc((unsigned char*)command, command->payload_size + 3));
        //uint8_t* ptr = (uint8_t*)command;
        //printf("%x %x %x\n", command->cmd, command->id, command->size);
        //for(int i = 0; i < command->size + 3; i++)
        //{
            //printf("%x ", (uint8_t)(*(ptr + i)));
        //}
        //printf("\nCrc: %x %x %x \n", command->crc , (command->crc >> 8) & 0xff, (command->crc) & 0xff);
        *state = SYNC;

        *error = CMD_READ_BAD_CRC;
        return NULL;
    }
    else //default
    {
        *error = CMD_READ_NO_CMD;
        return NULL;
    }
}

bool check_crc(packet_t* packet)
{
    uint16_t crc1, crc2;

    crc1 = packet->crc;
    crc2 = crc((unsigned char*)packet, packet->payload_size + 3); //+3 bytes cmd, id, size

    return (crc1 == crc2);
}