#include "HYDRA.h"
#include <string.h> // for memset

void init_hydra(hydra_t *hydra)
{
    if (hydra)
    {
        memset(&hydra->data, 0, sizeof(hydra->data));
        hydra->data.cam_enable = false;
    }
}

int read_hydra(hydra_t *hydra, system_data_t *system_data)
{
    if (hydra && system_data)
    {
        switch(hydra->id) {
            case HYDRA_UF:
                system_data->actuators.v_pressurizing = hydra->data.valve_states.v_controlled_1;
                system_data->actuators.v_vent = hydra->data.valve_states.v_controlled_2;
                system_data->thermocouples.n2o_tank_uf_t1 = hydra->data.thermo1;
                system_data->thermocouples.n2o_tank_uf_t2 = hydra->data.thermo2;
                system_data->thermocouples.n2o_tank_uf_t3 = hydra->data.thermo3;
                break;
            case HYDRA_LF:
                system_data->actuators.v_abort = hydra->data.valve_states.v_controlled_1;
                system_data->actuators.v_main = hydra->data.valve_states.v_controlled_2;
                system_data->thermocouples.n2o_tank_lf_t1 = hydra->data.thermo1;
                system_data->thermocouples.n2o_tank_lf_t2 = hydra->data.thermo2;
                system_data->thermocouples.chamber_thermo = hydra->data.thermo3;
                system_data->pressures.n2o_tank_pressure = hydra->data.pressure1;
                break;
            case HYDRA_FS:
                system_data->actuators.v_n2o_fill = hydra->data.valve_states.v_controlled_1;
                system_data->actuators.v_n2o_purge = hydra->data.valve_states.v_controlled_2;
                system_data->actuators.v_n2_fill = hydra->data.valve_states.v_steel_ball_1;
                system_data->actuators.v_n2_purge = hydra->data.valve_states.v_steel_ball_2;
                system_data->actuators.v_n2o_quick_dc = hydra->data.valve_states.v_quick_dc_1;
                system_data->actuators.v_n2_quick_dc = hydra->data.valve_states.v_quick_dc_2;
                system_data->pressures.n2o_line_pressure = hydra->data.pressure1;
                system_data->pressures.n2_line_pressure = hydra->data.pressure2;
                system_data->pressures.quick_dc_pressure = hydra->data.pressure3;
                system_data->thermocouples.n2o_line_thermo = hydra->data.thermo1;
                break;
            default:
                return -1; // Invalid hydra ID
        }
        return 0;
    }
    return -1;
}

int set_hydra_valve(hydra_t *hydra, hydra_valve_t valve, bool state)
{
    if (hydra && valve < hydra_valve_count)
    {
        // Set the valve state
        uint8_t payload[1];
        payload[0] = (uint8_t)(valve);
        send_hydra_command(hydra, HCMD_VALVE_SET, payload, 1);
        return 0;
    }
    return -1;
}

int set_hydra_valve_ms(hydra_t *hydra, hydra_valve_t valve, uint16_t ms)
{
    if (hydra && valve < hydra_valve_count)
    {
        if (ms <= 0 || ms > 60000) // Limit to 60 seconds
            return -1;
        uint8_t payload[3];
        payload[0] = (uint8_t)valve;
        payload[1] = (ms >> 8) & 0xFF;
        payload[2] = ms & 0xFF;
        send_hydra_command(hydra, HCMD_VALVE_MS, payload, 3);
        return 0;
    }
    return -1;
}

int send_hydra_command(hydra_t *hydra, hydra_cmd_t cmd, uint8_t *payload[], uint8_t payload_size)
{
    if (hydra)
    {
        packet_t packet;
        packet.sender_id = DEFAULT_ID;
        packet.target_id = (hydra->id == HYDRA_UF) ? HYDRA_UF_ID : (hydra->id == HYDRA_LF) ? HYDRA_LF_ID
                                                               : (hydra->id == HYDRA_FS)   ? HYDRA_FS_ID
                                                                                           : BROADCAST_ID;
        packet.cmd = (uint8_t)cmd;
        packet.payload_size = payload_size;
        if (payload && payload_size > 0)
        {
            memcpy(packet.payload, payload, payload_size);
        }
        packet.crc = 0; // TODO: Calculate CRC if needed
        write_packet(&packet, RS485_INTERFACE);
        // Send command to the hardware
        return 0;
    }
    return -1;
}

int process_hydra_response(hydra_t *hydra, packet_t *packet)
{
    if (hydra && packet)
    {
        int index = 0;
        if (packet->cmd == HCMD_ACK)
        {
            switch (packet->payload[index++])
            {
            case HCMD_STATUS:
                if (packet->payload_size < sizeof(hydra_data_t)) // Minimum size check
                    return -1;
                hydra->data.thermo1 = (packet->payload[index++] << 8) | packet->payload[index++];
                hydra->data.thermo2 = (packet->payload[index++] << 8) | packet->payload[index++];
                hydra->data.thermo3 = (packet->payload[index++] << 8) | packet->payload[index++];
                hydra->data.pressure1 = (packet->payload[index++] << 8) | packet->payload[index++];
                hydra->data.pressure2 = (packet->payload[index++] << 8) | packet->payload[index++];
                hydra->data.pressure3 = (packet->payload[index++] << 8) | packet->payload[index++];
                hydra->data.valve_states.raw = packet->payload[index++];
                hydra->data.cam_enable = packet->payload[index++];
                return 0;
            case HCMD_VALVE_SET:
            case HCMD_VALVE_MS:
                return 0; // Acknowledged
                break;
            default:
                return -1; // Unknown command in ACK
            }
        }
    }
    return -1;
}