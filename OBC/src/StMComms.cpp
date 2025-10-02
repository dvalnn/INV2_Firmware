#include <Arduino.h>

#include <Crc.h>

#include "GlobalVars.h"
#include "Comms.h"

#include "StMComms.h"
#include "StMWork.h"

#include "FlashLog.h"

extern system_data_t system_data;
extern filling_params_t filling_params;

int handle_status_cmd(packet_t *packet, interface_t interface, packet_t *packet_rep)
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
    packet_rep->sender_id = DEFAULT_ID;
    packet_rep->target_id = packet->sender_id;
    packet_rep->cmd = CMD_ACK;
    packet_rep->payload[index++] = CMD_STATUS;

    // echo ASK bytes
    packet_rep->payload[index++] = packet->payload[0];

    uint8_t ask = packet->payload[0];
    if(ask_groups_size > 8) {
        return CMD_RUN_OUT_OF_BOUND;
    }

    if ((ask & STATE_BIT))
    {
        packet_rep->payload[index++] = system_data.state;
    }

    if ((ask & ACTUATOR_STATES_BIT))
    {
        packet_rep->payload[index++] = (system_data.actuators.raw >> 8) & 0xff;
        packet_rep->payload[index++] = (system_data.actuators.raw) & 0xff;
    }

    if ((ask & R_PRESSURES_BIT))
    {
        int16_t ipressure;
        ipressure = (int16_t)(system_data.pressures.n2o_tank_pressure * 100);
        packet_rep->payload[index++] = (ipressure >> 8) & 0xff;
        packet_rep->payload[index++] = (ipressure) & 0xff;

        ipressure = (int16_t)(system_data.pressures.chamber_pressure * 100);
        packet_rep->payload[index++] = (ipressure >> 8) & 0xff;
        packet_rep->payload[index++] = (ipressure) & 0xff;
    }

    if((ask & FS_PRESSURES_BIT))
    {
        int16_t ipressure;
        ipressure = (int16_t)(system_data.pressures.n2o_line_pressure * 100);
        packet_rep->payload[index++] = (ipressure >> 8) & 0xff;
        packet_rep->payload[index++] = (ipressure) & 0xff;

        ipressure = (int16_t)(system_data.pressures.n2_line_pressure * 100);
        packet_rep->payload[index++] = (ipressure >> 8) & 0xff;
        packet_rep->payload[index++] = (ipressure) & 0xff;

        ipressure = (int16_t)(system_data.pressures.quick_dc_pressure * 100);
        packet_rep->payload[index++] = (ipressure >> 8) & 0xff;
        packet_rep->payload[index++] = (ipressure) & 0xff;
    }

    if ((ask & R_TEMPERATURES_BIT))
    {
        packet_rep->payload[index++] = (system_data.thermocouples.n2o_tank_uf_t1 >> 8) & 0xff;
        packet_rep->payload[index++] = (system_data.thermocouples.n2o_tank_uf_t1) & 0xff;

        packet_rep->payload[index++] = (system_data.thermocouples.n2o_tank_uf_t2 >> 8) & 0xff;
        packet_rep->payload[index++] = (system_data.thermocouples.n2o_tank_uf_t2) & 0xff;

        packet_rep->payload[index++] = (system_data.thermocouples.n2o_tank_uf_t3 >> 8) & 0xff;
        packet_rep->payload[index++] = (system_data.thermocouples.n2o_tank_uf_t3) & 0xff;

        packet_rep->payload[index++] = (system_data.thermocouples.n2o_tank_lf_t1 >> 8) & 0xff;
        packet_rep->payload[index++] = (system_data.thermocouples.n2o_tank_lf_t1) & 0xff;

        packet_rep->payload[index++] = (system_data.thermocouples.n2o_tank_lf_t2 >> 8) & 0xff;
        packet_rep->payload[index++] = (system_data.thermocouples.n2o_tank_lf_t2) & 0xff;

        packet_rep->payload[index++] = (system_data.thermocouples.chamber_thermo >> 8) & 0xff;
        packet_rep->payload[index++] = (system_data.thermocouples.chamber_thermo) & 0xff;
    }

    if ((ask & FS_TEMPERATURES_BIT))
    {
        packet_rep->payload[index++] = (system_data.thermocouples.n2o_line_thermo >> 8) & 0xff;
        packet_rep->payload[index++] = (system_data.thermocouples.n2o_line_thermo) & 0xff;
    }

    if ((ask & NAV_SENSORS_BIT))
    {
        // EUROC: Add nav sensors
    }

    if ((ask & NAV_KALMAN_BIT))
    {
        // EUROC: Add kalman payload
    }

    packet_rep->payload_size = index;
    packet_rep->crc = crc((unsigned char *)&packet_rep, packet_rep->payload_size + 3);
    write_packet(packet_rep, interface);

    return CMD_RUN_OK;
}

int handle_arm_cmd(packet_t *packet, interface_t interface, packet_t *packet_rep)
{
    // TODO: check arming logic
    if (system_data.state != READY)
        return CMD_RUN_STATE_ERROR;

    packet_rep->cmd = CMD_ACK;
    packet_rep->payload_size = 1;

    packet_rep->payload[0] = CMD_ARM;

    packet_rep->crc = crc((unsigned char *)packet_rep, packet_rep->payload_size + 3);
    write_packet(packet_rep, interface);

    start_log();

    return CMD_RUN_OK;
}

int handle_launch_override_cmd(packet_t *packet, interface_t interface, packet_t *packet_rep)
{
    // TODO: send command to open main valve

    packet_rep->cmd = CMD_ACK;
    packet_rep->payload_size = 0;
    packet_rep->crc = crc((unsigned char *)packet_rep, packet_rep->payload_size + 3);
    write_packet(packet_rep, interface);

    return CMD_RUN_OK;
}

int handle_fill_exec_cmd(packet_t *packet, interface_t interface, packet_t *packet_rep)
{
    if (packet->payload_size > filling_program_count * 2 + 1) // 1 byte for prog 2 bytes per parameter
    {
        return CMD_RUN_OUT_OF_BOUND;
    }

    // ensure that the rocket is in filling mode before running a filling program
    if (system_data.state == FILLING)
    {
        int index = 1;
        switch (packet->payload[0])
        {
        case SAFE_IDLE:
            index = 1;
            filling_params.target_pressure = (packet->payload[index++] << 8) + packet->payload[index++];
            filling_params.trigger_pressure = (packet->payload[index++] << 8) + packet->payload[index++];
            break;
        case FILLING_N2:
            index = 1;
            filling_params.target_pressure = (packet->payload[index++] << 8) + packet->payload[index++];
            break;
        case PRE_PRESSURE:
            index = 1;
            filling_params.target_pressure = (packet->payload[index++] << 8) + packet->payload[index++];
            filling_params.trigger_pressure = (packet->payload[index++] << 8) + packet->payload[index++];
            break;
        case FILLING_N2O:
            index = 1;
            filling_params.target_pressure = (packet->payload[index++] << 8) + packet->payload[index++];
            filling_params.trigger_pressure = (packet->payload[index++] << 8) + packet->payload[index++];
            filling_params.target_temperature = (packet->payload[index++] << 8) + packet->payload[index++];
            filling_params.trigger_temperature = (packet->payload[index++] << 8) + packet->payload[index++];
            filling_params.target_weight = (packet->payload[index++] << 8) + packet->payload[index++];
            break;
        case POST_PRESSURE:
            index = 1;
            filling_params.target_pressure = (packet->payload[index++] << 8) + packet->payload[index++];
            filling_params.trigger_pressure = (packet->payload[index++] << 8) + packet->payload[index++];
            break;
        default:
            return CMD_RUN_OUT_OF_BOUND;
        }

        packet_rep->cmd = CMD_ACK;
        packet_rep->payload_size = 0;
        packet_rep->crc = crc((unsigned char *)&packet_rep, packet_rep->payload_size + 3);
        write_packet(packet_rep, interface);

        return CMD_RUN_OK;
    }
    else
    {
        return CMD_RUN_STATE_ERROR;
    }
    return CMD_RUN_OK;
}

int handle_manual_valve_cmd(packet_t *packet)
{
    int valve = packet->payload[1];
    int valve_state = packet->payload[2];

    switch (valve)
    {
    case VALVE_ABORT:
        system_data.actuators.v_abort = valve_state;
        break;
    case VALVE_N2_PURGE:
        system_data.actuators.v_n2_purge = valve_state;
        break;
    case VALVE_N2O_PURGE:
        system_data.actuators.v_n2o_purge = valve_state;
        break;
    case VALVE_MAIN:
        system_data.actuators.v_main = valve_state;
        break;
    case VALVE_N2_FILL:
        system_data.actuators.v_n2_fill = valve_state;
        break;
    case VALVE_N2O_FILL:
        system_data.actuators.v_n2o_fill = valve_state;
        break;
    case VALVE_N2_QUICK_DC:
        system_data.actuators.v_n2_quick_dc = valve_state;
        break;
    case VALVE_N2O_QUICK_DC:
        system_data.actuators.v_n2o_quick_dc = valve_state;
        break;
    case VALVE_PRESSURIZING:
        system_data.actuators.v_pressurizing = valve_state;
        break;
    case VALVE_VENT:
        system_data.actuators.v_vent = valve_state;
        break;
    default:
        // valve not defined
        return CMD_RUN_OUT_OF_BOUND;
    }
    return CMD_RUN_OK;
}

int handle_manual_valve_ms_cmd(packet_t *packet)
{
    int valve = packet->payload[1];
    int valve_time = packet->payload[2];
    switch (valve)
    {
    case VALVE_ABORT:
        system_data.actuators.v_abort = 1;
        delay(valve_time);
        system_data.actuators.v_abort = 0;
        break;
    case VALVE_N2_PURGE:
        system_data.actuators.v_n2_purge = 1;
        delay(valve_time);
        system_data.actuators.v_n2_purge = 0;
        break;
    case VALVE_N2O_PURGE:
        system_data.actuators.v_n2o_purge = 1;
        delay(valve_time);
        system_data.actuators.v_n2o_purge = 0;
        break;
    case VALVE_MAIN:
        system_data.actuators.v_main = 1;
        delay(valve_time);
        system_data.actuators.v_main = 0;
        break;
    case VALVE_N2_FILL:
        system_data.actuators.v_n2_fill = 1;
        delay(valve_time);
        system_data.actuators.v_n2_fill = 0;
        break;
    case VALVE_N2O_FILL:
        system_data.actuators.v_n2o_fill = 1;
        delay(valve_time);
        system_data.actuators.v_n2o_fill = 0;
        break;
    case VALVE_N2_QUICK_DC:
        system_data.actuators.v_n2_quick_dc = 1;
        delay(valve_time);
        system_data.actuators.v_n2_quick_dc = 0;
        break;
    case VALVE_N2O_QUICK_DC:
        system_data.actuators.v_n2o_quick_dc = 1;
        delay(valve_time);
        system_data.actuators.v_n2o_quick_dc = 0;
        break;
    case VALVE_PRESSURIZING:
        system_data.actuators.v_pressurizing = 1;
        delay(valve_time);
        system_data.actuators.v_pressurizing = 0;
        break;
    case VALVE_VENT:
        system_data.actuators.v_vent = 1;
        delay(valve_time);
        system_data.actuators.v_vent = 0;
        break;
    default:
        // valve not defined
        return CMD_RUN_OUT_OF_BOUND;
    }
    return CMD_RUN_OK;
}

int handle_manual_exec_cmd(packet_t *packet, interface_t interface, packet_t *packet_rep)
{
    /* For now this check will be done by the GUI
        if (state != MANUAL)
            return CMD_RUN_STATE_ERROR;
        */

    packet_rep->cmd = CMD_ACK;
    packet_rep->payload_size = 1;
    packet_rep->payload[0] = packet->payload[0] + manual_cmd_size + 1;

    switch (packet->payload[0])
    {
    case CMD_MANUAL_SD_LOG_START:
        start_log();
        break;
    case CMD_MANUAL_SD_LOG_STOP:
        stop_log();
        break;
    case CMD_MANUAL_VALVE_STATE:
        handle_manual_valve_cmd(packet);
        break;
    case CMD_MANUAL_VALVE_MS:
        handle_manual_valve_ms_cmd(packet);
        break;
    default:
        return CMD_RUN_OUT_OF_BOUND;
        break;
    }
    // send manual command ack
    packet_rep->crc = crc((unsigned char *)&packet_rep, packet_rep->payload_size + 3);
    write_packet(packet_rep, interface);

    return CMD_RUN_OK;
}

int run_command(packet_t *packet, state_t state, interface_t interface)
{
    // Serial.printf("run command %d\n", cmd->cmd);
    packet_t packet_rep;
    packet_rep.sender_id = DEFAULT_ID;
    packet_rep.target_id = packet->sender_id;
    // Serial.printf("State: %d\n", state);
    switch (packet->cmd) {
        case CMD_STATUS:
            //Serial.println("Status cmd");
            return handle_status_cmd(packet, interface, &packet_rep);
            break;
        case CMD_ARM:
            return handle_arm_cmd(packet, interface, &packet_rep);
            break;
        case CMD_LAUNCH_OVERRIDE:
            return handle_launch_override_cmd(packet, interface, &packet_rep);
            break;
        case CMD_FILL_EXEC:
            return handle_fill_exec_cmd(packet, interface, &packet_rep);
            break;
        case CMD_MANUAL_EXEC:
            return handle_manual_exec_cmd(packet, interface, &packet_rep);
            break;
        default:
            // if the command has no action it still needs to return ok to change state
            if (packet->cmd < cmd_size)
            {
                if (packet->target_id == BROADCAST_ID)
                {
                    // if we recieve a broadcast message, we don't send an ack
                    return CMD_RUN_OK;
                }

                packet_rep.cmd = CMD_ACK;
                packet_rep.payload_size = 1;
                packet_rep.payload[0] = packet->cmd;
                packet_rep.crc = crc((unsigned char *)&packet_rep, packet_rep.payload_size + 3);

                write_packet(&packet_rep, interface);

                // printf("Command %d state %d resulting transition state %d\n",
                // cmd->cmd, state, state_machine[state].next_states[cmd->cmd]);
                return CMD_RUN_OK;
            }
            else // cmd code out of bounds, return error
                return CMD_RUN_OUT_OF_BOUND;
            break;
    };

    return CMD_RUN_OUT_OF_BOUND;
    // Serial.printf("cmd: %x state: %d return state %d table: %d\n", cmd->cmd, state, return_state,
    //(rocket_state_t)expected_state[state][cmd->cmd]);
}