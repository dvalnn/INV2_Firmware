#include <Arduino.h>
#include <Crc.h>
#include <Wire.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "Comms.h"
#include "DataModels.h"
#include "HardwareCfg.h"
#include "Peripherals/IO_Map.h"
#include "Sensors.h"

#include "Peripherals/IO_Map.h"
#include "Peripherals/RS485.h"

bool setup_error = false;
data_t my_data = {0};

void run_command(packet_t *packet)
{
    if (packet->cmd == CMD_STATUS)
    {
        // send status packet
        packet_t status_packet;
        status_packet.sender_id = DEFAULT_ID;
        status_packet.target_id = packet->sender_id;
        status_packet.cmd = CMD_ACK;
        status_packet.payload_size = sizeof(data_t) + 1; // +1 for cmd ack
        status_packet.payload[0] = CMD_STATUS;
        memcpy(status_packet.payload + 1, &my_data, sizeof(data_t));
        status_packet.crc = 0;
        if (CRC_ENABLED)
            status_packet.crc = crc((uint8_t *)&status_packet,
                                    HEADER_SIZE + status_packet.payload_size);
        write_packet(&status_packet);
    }
}

void setup()
{
    memset(&my_data, 0, sizeof(data_t));

    Serial.begin(USB_BAUD_RATE); // USBC serial
    // rs485_init(); // RS-485 serial
    while(!Serial) {
        ;
    }
    setup_error |= loadcells_setup(); // change to loadcell setup

    if (setup_error != 0)
    {
        Serial.println("Setup error detected!");
    }
    else
    {
        Serial.println("Setup complete!");
    }
}


void loop()
{
    static unsigned long last_test = 0;
    int error;

    // check if we have new data
    // if we get a valid message, execute the command associated to it
    /* packet_t *packet = read_packet(&error);
    if (packet != NULL && error == CMD_READ_OK)
    {
        // tone(BUZZER_PWM_PIN, 1000, 50); // beep on command receive
        run_command(packet);
    }
    */
    calibrate_loadcells();
    read_sensors(&my_data);
    //delay(10); // small delay to avoid overwhelming the CPU
}
