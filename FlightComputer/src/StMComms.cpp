#include <Arduino.h>

#include <Crc.h>

#include "GlobalVars.h"
#include "Comms.h"

#include "StMComms.h"
#include "StMWork.h"

#include "FlashLog.h"

int run_command(command_t *cmd, rocket_state_t state, interface_t interface)
{
    // Serial.printf("run command %d\n", cmd->cmd);
    command_t command_rep;
    command_rep.id = GROUND_ID;
    //Serial.printf("State: %d\n", state);
    switch (cmd->cmd)
    {
    case CMD_STATUS:
    {
        uint16_t index = 0;
        // union used to get bit representation of float
        union ufloat
        {
            float f;
            uint32_t i;
        };
        union ufloat f;

        /*
         * Prepare ACK response
         * Send response
         */
        command_rep.cmd = CMD_STATUS_ACK;

        command_rep.data[index++] = cmd->data[0];
        command_rep.data[index++] = cmd->data[1];

        uint16_t log_bits = (cmd->data[0] << 8) + cmd->data[1];

        if ((log_bits & ROCKET_STATE_BIT))
        {
            command_rep.data[index++] = state;
            command_rep.data[index++] = (uint8_t)((log_running << 7) |
                                                  (Tank_Top_Module.valve_state << 6) |
                                                  (Tank_Bot_Module.valve_state << 5) |
                                                  (Chamber_Module.valve_state << 4) |
                                                  (DragDeployed << 3) |
                                                  (MainDeployed << 2));
        }

        if ((log_bits & ROCKET_PRESSURE_BIT))
        {
            int16_t ipressure;
            ipressure = (int16_t)(Tank_Top_Module.pressure * 100);
            command_rep.data[index++] = (ipressure >> 8) & 0xff;
            command_rep.data[index++] = (ipressure) & 0xff;

            ipressure = (int16_t)(Tank_Bot_Module.pressure * 100);
            command_rep.data[index++] = (ipressure >> 8) & 0xff;
            command_rep.data[index++] = (ipressure) & 0xff;

            ipressure = (int16_t)(Chamber_Module.pressure * 100);
            command_rep.data[index++] = (ipressure >> 8) & 0xff;
            command_rep.data[index++] = (ipressure) & 0xff;
        }

        if ((log_bits & ROCKET_TEMPERATURE_BIT))
        {
            command_rep.data[index++] = (Tank_Top_Module.temperature >> 8) & 0xff;
            command_rep.data[index++] = (Tank_Top_Module.temperature) & 0xff;

            command_rep.data[index++] = (Tank_Bot_Module.temperature >> 8) & 0xff;
            command_rep.data[index++] = (Tank_Bot_Module.temperature) & 0xff;
        }

        if ((log_bits & ROCKET_GPS_BIT))
        {

            uint16_t gps_alt = (uint16_t)(gps_altitude);

            command_rep.data[index++] = (uint8_t)(gps_satalites);

            command_rep.data[index++] = (uint8_t)((gps_alt >> 8) & 0xff);
            command_rep.data[index++] = (uint8_t)((gps_alt) & 0xff);

            f.f = gps_lat;
            command_rep.data[index++] = (uint8_t)((f.i >> 24) & 0xff);
            command_rep.data[index++] = (uint8_t)((f.i >> 16) & 0xff);
            command_rep.data[index++] = (uint8_t)((f.i >> 8) & 0xff);
            command_rep.data[index++] = (uint8_t)((f.i) & 0xff);

            f.f = gps_lon;
            command_rep.data[index++] = (uint8_t)((f.i >> 24) & 0xff);
            command_rep.data[index++] = (uint8_t)((f.i >> 16) & 0xff);
            command_rep.data[index++] = (uint8_t)((f.i >> 8) & 0xff);
            command_rep.data[index++] = (uint8_t)((f.i) & 0xff);

            uint16_t horizontal_vel = (gps_horizontal_vel * 10);
            command_rep.data[index++] = (uint8_t)((horizontal_vel >> 8) & 0xff);
            command_rep.data[index++] = (uint8_t)((horizontal_vel) & 0xff);
        }

        if ((log_bits & ROCKET_BAROMETER_BIT))
        {
            uint16_t ualtitude = altitude;
            command_rep.data[index++] = (uint8_t)((ualtitude >> 8) & 0xff);
            command_rep.data[index++] = (uint8_t)((ualtitude) & 0xff);
        }

        if ((log_bits & ROCKET_IMU_BIT))
        {
            uint16_t u_ax = imu_ax * 10;
            command_rep.data[index++] = (uint8_t)((u_ax >> 8) & 0xff);
            command_rep.data[index++] = (uint8_t)((u_ax) & 0xff);

            uint16_t u_ay = imu_ay * 10;
            command_rep.data[index++] = (uint8_t)((u_ay >> 8) & 0xff);
            command_rep.data[index++] = (uint8_t)((u_ay) & 0xff);

            uint16_t u_az = imu_az * 10;
            command_rep.data[index++] = (uint8_t)((u_az >> 8) & 0xff);
            command_rep.data[index++] = (uint8_t)((u_az) & 0xff);

            uint16_t u_gx = imu_gx * 10;
            command_rep.data[index++] = (uint8_t)((u_gx >> 8) & 0xff);
            command_rep.data[index++] = (uint8_t)((u_gx) & 0xff);

            uint16_t u_gy = imu_gy * 10;
            command_rep.data[index++] = (uint8_t)((u_gy >> 8) & 0xff);
            command_rep.data[index++] = (uint8_t)((u_gy) & 0xff);

            uint16_t u_gz = imu_gz * 10;
            command_rep.data[index++] = (uint8_t)((u_gz >> 8) & 0xff);
            command_rep.data[index++] = (uint8_t)((u_gz) & 0xff);
        }

        if ((log_bits & ROCKET_KALMAN_BIT))
        {
            uint16_t u_z = kalman_altitude * 10;
            uint16_t u_z_max = maxAltitude * 10;
            uint16_t u_vz = kalman_velocity * 10;
            uint16_t u_az = kalman_accel * 10;

            command_rep.data[index++] = (uint8_t)((u_z >> 8) & 0xff);
            command_rep.data[index++] = (uint8_t)((u_z) & 0xff);

            command_rep.data[index++] = (uint8_t)((u_z_max >> 8) & 0xff);
            command_rep.data[index++] = (uint8_t)((u_z_max) & 0xff);

            command_rep.data[index++] = (uint8_t)((u_vz >> 8) & 0xff);
            command_rep.data[index++] = (uint8_t)((u_vz) & 0xff);

            command_rep.data[index++] = (uint8_t)((u_az >> 8) & 0xff);
            command_rep.data[index++] = (uint8_t)((u_az) & 0xff);

            // convert quaternion [0:1] to uint16 [0, 2^16 - 1]
            uint16_t u_quat1 = kalman_q[0] * (0xFFFF);
            uint16_t u_quat2 = kalman_q[1] * (0xFFFF);
            uint16_t u_quat3 = kalman_q[2] * (0xFFFF);
            uint16_t u_quat4 = kalman_q[3] * (0xFFFF);

            command_rep.data[index++] = (uint8_t)((u_quat1 >> 8) & 0xff);
            command_rep.data[index++] = (uint8_t)((u_quat1) & 0xff);

            command_rep.data[index++] = (uint8_t)((u_quat2 >> 8) & 0xff);
            command_rep.data[index++] = (uint8_t)((u_quat2) & 0xff);

            command_rep.data[index++] = (uint8_t)((u_quat3 >> 8) & 0xff);
            command_rep.data[index++] = (uint8_t)((u_quat3) & 0xff);

            command_rep.data[index++] = (uint8_t)((u_quat4 >> 8) & 0xff);
            command_rep.data[index++] = (uint8_t)((u_quat4) & 0xff);
        }

        if ((log_bits & ROCKET_CHUTE_EMATCH_BIT))
        {
            command_rep.data[index++] = ((ematch_main_reading >> 8) & 0xff);
            command_rep.data[index++] = ((ematch_main_reading) & 0xff);

            command_rep.data[index++] = ((ematch_drag_reading >> 8) & 0xff);
            command_rep.data[index++] = ((ematch_drag_reading) & 0xff);

        }

        command_rep.size = index;
        command_rep.crc = crc((unsigned char *)&command_rep, command_rep.size + 3);
        write_command(&command_rep, interface);

        // Serial.printf("he moles %f %d\ntank_moles %f %d\n", he_mol, he_moles_i, tank_mol_lost, tank_mol_lost_i);

        return CMD_RUN_OK;
    }
    break;

    case CMD_ARM:
    {
        if(state != READY)
            return CMD_RUN_STATE_ERROR;

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

        if (error != CMD_READ_OK || arm_cmd->cmd != CMD_ARM || arm_cmd->data[0] != ARN_TRIGGER_3)
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

    case CMD_ALLOW_LAUNCH:
    {
        Launch = true; // used in kalman for events

        command_rep.cmd = CMD_ALLOW_LAUNCH_ACK;
        command_rep.size = 0;
        command_rep.crc = crc((unsigned char *)&command_rep, command_rep.size + 3);
        write_command(&command_rep, interface);

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
                command_rep.data[i * 2 + 1] = ((files[i] >> 8) & 0xff);
                command_rep.data[i * 2 + 2] = (files[i] & 0xff);
            }
            command_rep.size += index * 2;

            // for(int i = 0; i < index; i++)
            // printf("%x %x\n", command_rep.data[i*2 + 1], command_rep.data[i*2 + 2]);
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
            case Purge_valve:
            {
                digitalWrite(Tank_Bot_Module.valve_pin, valve_state);
                Tank_Bot_Module.valve_state = valve_state;
            }
            break;
            case Engine_valve:
            {
                digitalWrite(Chamber_Module.valve_pin, valve_state);
                Chamber_Module.valve_state = valve_state;
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
            case Purge_valve:
            {
                if (!Tank_Bot_Module.valve_state)
                {
                    digitalWrite(Tank_Bot_Module.valve_pin, 1);
                    delay(valve_time);
                    digitalWrite(Tank_Bot_Module.valve_pin, 0);
                }
            }
            break;
            case Engine_valve:
            {
                if (!Chamber_Module.valve_state)
                {
                    digitalWrite(Chamber_Module.valve_pin, 1);
                    delay(valve_time);
                    digitalWrite(Chamber_Module.valve_pin, 0);
                }
            }
            default:
            {
                // bad valve
                return CMD_RUN_OUT_OF_BOUND;
            }
            };
        }
        break;

        case CMD_MANUAL_IMU_CALIBRATE:
        {
            imu_calibrate();
        }
        break;

        case CMD_MANUAL_BAROMETER_CALIBRATE:
        {
            barometer_calibrate();
        }
        break;

        case CMD_MANUAL_KALMAN_CALIBRATE:
        {
            kalman_calibrate();
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