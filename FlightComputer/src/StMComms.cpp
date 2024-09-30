#include <Arduino.h>

#include <Crc.h>

#include "GlobalVars.h"
#include "Comms.h"

#include "StMComms.h"

#include "FlashLog.h"

#include "Utils.h"

int run_command(command_t *cmd, rocket_state_t state, interface_t interface)
{
    // Serial.printf("run command %d\n", cmd->cmd);
    command_t command_rep;
    command_rep.id = GROUND_ID;

    switch (cmd->cmd)
    {
    case CMD_STATUS:
    {
        /*
         * Prepare ACK response
         * Send response
         */
        command_rep.cmd = CMD_STATUS_ACK;

        command_rep.size = 36;
        command_rep.data[0] = state;
        command_rep.data[1] = (Tank_Top_Module.temperature >> 8) & 0xff;
        command_rep.data[2] = (Tank_Top_Module.temperature) & 0xff;
        command_rep.data[3] = (Tank_Bot_Module.temperature >> 8) & 0xff;
        command_rep.data[4] = (Tank_Bot_Module.temperature) & 0xff;

        command_rep.data[5] = (Chamber_Module.temperature1 >> 8) & 0xff;
        command_rep.data[6] = (Chamber_Module.temperature1) & 0xff;
        command_rep.data[7] = (Chamber_Module.temperature2 >> 8) & 0xff;
        command_rep.data[8] = (Chamber_Module.temperature2) & 0xff;
        command_rep.data[9] = (Chamber_Module.temperature3 >> 8) & 0xff;
        command_rep.data[10] = (Chamber_Module.temperature3) & 0xff;

        int16_t ipressure = (int16_t)(Tank_Top_Module.pressure * 100);
        command_rep.data[11] = (ipressure >> 8) & 0xff;
        command_rep.data[12] = (ipressure) & 0xff;

        ipressure = (int16_t)(Tank_Bot_Module.pressure * 100);
        command_rep.data[13] = (ipressure >> 8) & 0xff;
        command_rep.data[14] = (ipressure) & 0xff;

        command_rep.data[15] = (tank_pressure >> 8) & 0xff;
        command_rep.data[16] = (tank_pressure) & 0xff;

        int16_t itank_liquid = (int16_t)(tank_liquid * 10000);
        command_rep.data[17] = (itank_liquid >> 8) & 0xff;
        command_rep.data[18] = (itank_liquid) & 0xff;

        command_rep.data[19] = (uint8_t)((log_running << 7) |
                                         (Tank_Top_Module.valve_state << 6) |
                                         (Tank_Bot_Module.valve_state << 5) |
                                         tank_tactile_bits);

        command_rep.data[20] = (Scale_Module.weight1 >> 8) & 0xff;
        command_rep.data[21] = (Scale_Module.weight1) & 0xff;

        command_rep.data[22] = (Scale_Module.weight2 >> 8) & 0xff;
        command_rep.data[23] = (Scale_Module.weight2) & 0xff;

        command_rep.data[24] = (Scale_Module.weight3 >> 8) & 0xff;
        command_rep.data[25] = (Scale_Module.weight3) & 0xff;

        ipressure = (int16_t)(Chamber_Module.pressure * 100);
        command_rep.data[26] = (ipressure >> 8) & 0xff;
        command_rep.data[27] = (ipressure) & 0xff;

        int16_t he_moles_i = (int16_t)(he_mol * 10);
        command_rep.data[28] = (he_moles_i >> 8) & 0xff;
        command_rep.data[29] = (he_moles_i) & 0xff;

        int16_t tank_mol_lost_i = (int16_t)(tank_mol_lost * 10);
        command_rep.data[30] = (tank_mol_lost_i >> 8) & 0xff;
        command_rep.data[31] = (tank_mol_lost_i) & 0xff;

        int16_t hL_i = (int16_t)(he_mol * 100);
        command_rep.data[32] = (hL_i >> 8) & 0xff;
        command_rep.data[33] = (hL_i) & 0xff;

        int16_t ml_i = (int16_t)(tank_mol_lost * 100);
        command_rep.data[34] = (ml_i >> 8) & 0xff;
        command_rep.data[35] = (ml_i) & 0xff;

        command_rep.crc = crc((unsigned char *)&command_rep, command_rep.size + 3);
        write_command(&command_rep, interface);

        //Serial.printf("he moles %f %d\ntank_moles %f %d\n", he_mol, he_moles_i, tank_mol_lost, tank_mol_lost_i);

        return CMD_RUN_OK;
    }
    break;

    case CMD_ARM:
    {
        // stage 1
        if (cmd->data[0] != ARN_TRIGGER_1)
        {
            // error
            return CMD_RUN_ARM_ERROR;
        }

        command_rep.cmd = CMD_ARM_ACK;
        command_rep.size = 1;

        command_rep.data[0] = ARN_TRIGGER_1;

        command_rep.crc = crc((unsigned char *)&command_rep, command_rep.size + 3);
        write_command(&command_rep, interface);

        // stage 2
        int error = 0;
        command_t *arm_cmd;
        while ((arm_cmd = read_command(&error, DEFAULT_CMD_INTERFACE)) == NULL && error == CMD_READ_NO_CMD)
        {
        }

        if (error != CMD_READ_OK || arm_cmd->cmd != CMD_ARM || arm_cmd->data[0] != ARN_TRIGGER_2)
        {
            // error
            return CMD_RUN_ARM_ERROR;
        }

        command_rep.data[0] = ARN_TRIGGER_2;
        command_rep.crc = crc((unsigned char *)&command_rep, command_rep.size + 3);
        write_command(&command_rep, interface);

        // stage 3
        while ((arm_cmd = read_command(&error, DEFAULT_CMD_INTERFACE)) == NULL && error == CMD_READ_NO_CMD)
        {
        }

        if (error != OK || arm_cmd->cmd != CMD_ARM || arm_cmd->data[0] != ARN_TRIGGER_3)
        {
            // error
            return CMD_RUN_ARM_ERROR;
        }

        command_rep.data[0] = ARN_TRIGGER_3;
        command_rep.crc = crc((unsigned char *)&command_rep, command_rep.size + 3);
        write_command(&command_rep, interface);

        start_log();

        return CMD_RUN_OK;
    }
    break;

    case CMD_EXEC_PROG:
    {
        if (cmd->size != 7) // 1 byte for prog 2 bytes per var (p1,p2,p3,l1,l2)
        {
            return CMD_RUN_OUT_OF_BOUND;
        }

        // ensure that the rocket is in fueling mode before running a fueling program
        if (state == FUELING)
        {
            rocket_state_t next_state = -1;
            switch (cmd->data[0])
            {
            case SAFETY_PRESSURE_PROG:
                next_state = SAFETY_PRESSURE;
                RP1 = (cmd->data[1] << 8) + cmd->data[2];
                RP2 = (cmd->data[3] << 8) + cmd->data[4];

                // if(RP2 > 5500) RP2 = 5500;
                // if(RP1 > RP2) RP1 = RP2 - 1000;

                RL1 = (cmd->data[5] << 8) + cmd->data[6];
                break;
            case PURGE_PRESSURE_PROG:
                next_state = PURGE_PRESSURE;
                RP1 = (cmd->data[1] << 8) + cmd->data[2];
                RP2 = (cmd->data[3] << 8) + cmd->data[4];
                RL1 = (cmd->data[5] << 8) + cmd->data[6];
                break;
            case PURGE_LIQUID_PROG:
                next_state = PURGE_LIQUID;
                RP1 = (cmd->data[1] << 8) + cmd->data[2];
                RP2 = (cmd->data[3] << 8) + cmd->data[4];
                RL1 = (cmd->data[5] << 8) + cmd->data[6];
                break;
            default:
            {
                return CMD_RUN_OUT_OF_BOUND;
            }
            }

            comm_transition[FUELING][CMD_EXEC_PROG] = next_state;

            command_rep.cmd = CMD_EXEC_PROG_ACK;
            command_rep.size = 0;
            command_rep.crc = crc((unsigned char *)&command_rep, command_rep.size + 3);
            write_command(&command_rep, interface);

            return CMD_RUN_OK;
        }
        else
        {
            return CMD_RUN_STATE_ERROR;
        }
    }
    break;

    case CMD_MANUAL_EXEC:
    {
        if (state != MANUAL)
            return CMD_RUN_STATE_ERROR;

        command_rep.cmd = CMD_MANUAL_EXEC_ACK;
        command_rep.size = 1;
        command_rep.data[0] = cmd->data[0] + manual_cmd_size + 1;

        switch (cmd->data[0])
        {
        case CMD_MANUAL_FLASH_LOG_START:
        {
            start_log();
        }
        break;

        case CMD_MANUAL_FLASH_LOG_STOP:
        {
            stop_log();
        }
        break;

        case CMD_MANUAL_FLASH_IDS:
        {
            uint16_t files[256] = {0};
            uint16_t index;
            get_log_ids(files, &index);

            // fill data buff with all file indexs
            for (int i = 0; i < index; i++)
            {
                command_rep.data[i*2 + 1] = ((files[i] >> 8) & 0xff);
                command_rep.data[i*2 + 2] = (files[i] & 0xff);
            }
            command_rep.size += index * 2;

            //for(int i = 0; i < index; i++)
                //printf("%x %x\n", command_rep.data[i*2 + 1], command_rep.data[i*2 + 2]);
        }
        break;

        case CMD_MANUAL_FLASH_DUMP:
        {
            uint16_t id = (cmd->data[1] << 8) + (cmd->data[2]);
            dump_log(id);
        }
        break;

        case CMD_MANUAL_VALVE_STATE:
        {
            int valve = cmd->data[1];
            int valve_state = cmd->data[2];

            switch (valve)
            {
            case VPU_valve:
            {
                digitalWrite(Tank_Top_Module.valve_pin, valve_state);
                Tank_Top_Module.valve_state = valve_state;
            }
            break;
            case Engine_valve:
            {
                digitalWrite(Tank_Bot_Module.valve_pin, valve_state);
                Tank_Bot_Module.valve_state = valve_state;
            }
            break;
            default:
            {
                // bad valve
                return CMD_RUN_OUT_OF_BOUND;
            }
            };
        }
        break;

        case CMD_MANUAL_VALVE_MS:
        {
            int valve = cmd->data[1];
            int valve_time = cmd->data[2];

            switch (valve)
            {
            case VPU_valve:
            {
                if (!Tank_Top_Module.valve_state)
                {
                    digitalWrite(Tank_Top_Module.valve_pin, 1);
                    delay(valve_time);
                    digitalWrite(Tank_Top_Module.valve_pin, 0);
                }
            }
            break;
            case Engine_valve:
            {
                if (!Tank_Top_Module.valve_state)
                {
                    digitalWrite(Tank_Bot_Module.valve_pin, 1);
                    delay(valve_time);
                    digitalWrite(Tank_Bot_Module.valve_pin, 0);
                }
            }
            break;
            default:
            {
                // bad valve
                return CMD_RUN_OUT_OF_BOUND;
            }
            };
        }
        break;

        case CMD_MANUAL_LOADCELL_TARE:
        {
            double read = Scale_Module.scale1.read_average(10);
            float read_scaled = read * Scale_Module.scale1_scale + Scale_Module.scale1_offset;

            Scale_Module.scale1_offset -= read_scaled;

            read = Scale_Module.scale2.read_average(10);
            read_scaled = read * Scale_Module.scale2_scale + Scale_Module.scale2_offset;

            Scale_Module.scale2_offset -= read_scaled;

            read = Scale_Module.scale3.read_average(10);
            read_scaled = read * Scale_Module.scale3_scale + Scale_Module.scale3_offset;

            Scale_Module.scale3_offset -= read_scaled;

            // Scale_Module.scale1.tare();
            // Scale_Module.scale2.tare();
            // Scale_Module.scale3.tare();
        }
        break;

        case CMD_MANUAL_TANK_TARE:
        {
            tank_mol_lost = 0;
            he_mol = calc_moles(Tank_Top_Module.pressure, Tank_Top_Module.temperature);
        }
        break;

        default:
        {
            return CMD_RUN_OUT_OF_BOUND;
        }
        };

        // send manual command ack
        command_rep.crc = crc((unsigned char *)&command_rep, command_rep.size + 3);
        write_command(&command_rep, interface);

        return CMD_RUN_OK;
    }
    break;

    default:
        // if the command has no action it still needs to return ok to change state
        if (cmd->cmd < cmd_size)
        {
            if (cmd->id == BROADCAST_ID)
            {
                // if we recieve a broadcast message, we don't send an ack
                return CMD_RUN_OK;
            }

            command_rep.cmd = (cmd_type_t)(cmd->cmd + cmd_size + 1);
            command_rep.size = 0;
            command_rep.crc = crc((unsigned char *)&command_rep, command_rep.size + 3);

            write_command(&command_rep, interface);

            // printf("Command %d state %d resulting transition state %d\n",
            // cmd->cmd, state, state_machine[state].comms[cmd->cmd]);
            return CMD_RUN_OK;
        }
        else // cmd code out of bounds, return error
            return CMD_RUN_OUT_OF_BOUND;
        break;
    };

    return CMD_RUN_OUT_OF_BOUND;

    // Serial.printf("cmd: %x state: %d return state %d table: %d\n", cmd->cmd, state, return_state,
    //(rocket_state_t)comm_transition[state][cmd->cmd]);
}