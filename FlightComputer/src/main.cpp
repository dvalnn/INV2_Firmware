/**
 * @file main.cpp
 * @author Andre M. (portospaceteam.pt)
 * @brief Entry point for esp32 after boot
 *      Main message parsing and command executing
 *
 * @version 0.1
 * @date 2024-01-31
 *
 * @copyright Copyright (c) 2024
 *
 */
#include <Arduino.h>

#include <stdbool.h>
#include <inttypes.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "HardwareCfg.h"
#include "GlobalVars.h"

#include "Comms.h"

#include "StateMachine.h"
#include "StMComms.h"
#include "StMWork.h"
#include "FlashLog.h"

#include "Control_Work.h"
#include <I2Cdev.h>
#include <LoRa.h>
#include <Crc.h>
#include <SerialFlash.h>
#include <LittleFS.h>

#define FORMAT_LITTLEFS_IF_FAILED true

bool fast_reboot = 0;

bool Launch = false;

system_data_t system_data;
filling_params_t filling_params;

void valves_setup(void)
{
    // TODO: Send command to hydras to close all valves
}

void sd_setup(void)
{
    // TODO: Setup SD card
}

void lora_setup(void)
{
    // TODO: Setup LoRa module
}

void setup()
{
    Serial.begin(USB_BAUD_RATE);    // USBC serial
    Serial1.begin(RS485_BAUD_RATE); // RS485

    printf("Setup done\n");
}

void loop()
{
    while (true)
    {
        static bool init_flag = false;
        if (!init_flag)
        {
            state_machine[system_data.state].entry_time = millis();
            for (int i = 0; i < MAX_WORK_SIZE; i++)
                state_machine[system_data.state].work[i].begin =
                    state_machine[system_data.state].work[i].delay;
            init_flag = true;
        }

        state_t command_state = system_data.state,
               event_state = system_data.state;

        /*
            Execute the state function
        */
        bool work_performed = WORK_HANDLER();

        /*
        Event handling
        */
        // if (work_performed) event_state = EVENT_HANDLER();
        event_state = EVENT_HANDLER();
        if (event_state == -1)
            event_state = system_data.state;

        /*
        Comms
        */
        int error;

        // check if we have new data
        // if we get a valid message, execute the command associated to it
        packet_t *packet = read_packet(&error, DEFAULT_CMD_INTERFACE);
        if (packet != NULL && error == CMD_READ_OK)
        {
            int error = run_command(packet, system_data.state, DEFAULT_CMD_INTERFACE);

            // make transition to new state on the state machine
            if (error == CMD_RUN_OK &&
                state_machine[system_data.state].next_states[packet->cmd] != -1)
            {
                // we have new state, use lookup table
                command_state = expected_state[system_data.state][packet->cmd];
                // Serial2.printf("change state to %d\n", state_machine[state].next_states[cmd->cmd]);
            }
            else if (error != CMD_RUN_OK)
            {
                // log cmd execution error
                // Serial2.printf("EXECUTING MESSAGE ERROR %d\n", error);
            }
        }
        else if (error != CMD_READ_OK &&
                 error != CMD_READ_NO_CMD)
        {
            // log cmd read error
            // Serial.printf("READING MESSAGE ERROR %d\n", error);
        }

        /*
         * Do state transition
         */
        if (command_state != system_data.state)
        {
            // command change of state as priority over
            // internal events changes of state
            system_data.state = command_state;
            state_machine[system_data.state].entry_time = millis();

            // reset sensor timer
            for (int i = 0; i < MAX_WORK_SIZE; i++)
                state_machine[system_data.state].work[i].begin =
                    state_machine[system_data.state].work[i].delay;

            log(&system_data.state, 0, STATE_CHANGE);
        }

        // command state transitions must take precedence to event state transitions
        else if (event_state != system_data.state)
        {
            // only if next_states haven't changed the state we can
            // acept a new state from internal events
            system_data.state = event_state;
            state_machine[system_data.state].entry_time = millis();

            // reset sensor timer
            for (int i = 0; i < MAX_WORK_SIZE; i++)
                state_machine[system_data.state].work[i].begin =
                    state_machine[system_data.state].work[i].delay;

            log(&system_data.state, 0, STATE_CHANGE);
        }
    }
}

void software_work(void *paramms)
{

    Serial.printf("got to stm work\n\r");
}