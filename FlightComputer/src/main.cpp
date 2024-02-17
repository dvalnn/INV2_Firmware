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

#include "Target.h"
#include "HardwareCfg.h"
#include "StateMachine.h"
#include "GlobalVars.h"
#include "StMWork.h"
#include "Comms.h"

// I2Cdev and MPU6050 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include <I2Cdev.h>
#include <MPU6050.h>

#include <LoRa.h>

#include <Crc.h>

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 accelgyro;
//MPU6050 accelgyro(0x69); // <-- use for AD0 high



int led_state = 0;

void echo_reply(command_t* cmd)
{
/*
    Used during testing. See Target.h
*/
#ifdef DIGITAL_TARGET

    Serial.printf("Cmd: %x\n", cmd->cmd);
    Serial.printf("Sz: %x\n", cmd->size);
    if(cmd->size > 0)
    {
        for(int i = 0; i < cmd->size; i++) Serial.printf("%x ", cmd->data[i]);
        Serial.printf("\n");
    }
    Serial.printf("CRC: %x\n", cmd->crc);
#endif
}

int run_command(command_t* cmd, rocket_state_t state)
{
    command_t command_rep;
    rocket_state_t return_state = state;
    switch(cmd->cmd)
    {
        case CMD_LED_ON:
        {
            /* Cmd execution */
            digitalWrite(LED_PIN, HIGH);

            /*
             * Prepare ACK response
             * Send response
             */
            command_rep.cmd = CMD_LED_ON_ACK;
            command_rep.size = 0;
            command_rep.crc = 0x3131;

            write_command(&command_rep);

            return CMD_RUN_OK;
        }
        break;

        case CMD_LED_OFF:
        {
            /* Cmd execution */
            digitalWrite(LED_PIN, LOW);

            /*
             * Prepare ACK response
             * Send response
             */
            command_rep.cmd = CMD_LED_OFF_ACK;
            command_rep.size = 0;
            command_rep.crc = 0x2121;

            write_command(&command_rep);

            return CMD_RUN_OK;
        }
        break;

        case CMD_STATUS:
        {
            /*
             * Prepare ACK response
             * Send response
             */
            command_rep.cmd = CMD_STATUS_ACK;
            command_rep.size = 2*6 + 1;
            command_rep.data[0] = state;
            command_rep.data[1] = (imu_ax >> 8) & 0xff;
            command_rep.data[2] = (imu_ax) & 0xff ;
            command_rep.data[3] = (imu_ay >> 8) & 0xff;
            command_rep.data[4] = (imu_ay) & 0xff;
            command_rep.data[5] = (imu_az >> 8) & 0xff;
            command_rep.data[6] = (imu_az) & 0xff;
            command_rep.data[7] = (imu_gx >> 8) & 0xff;
            command_rep.data[8] = (imu_gx) & 0xff;
            command_rep.data[9] = (imu_gy >> 8) & 0xff;
            command_rep.data[10] = (imu_gy) & 0xff;
            command_rep.data[11] = (imu_gz >> 8) & 0xff;
            command_rep.data[12] = (imu_gz) & 0xff;
            command_rep.crc = 0x5151;

            write_command(&command_rep);

            return CMD_RUN_OK;
        }
        break; 

        case CMD_ARM:
        {
            //stage 1
            if(cmd->data[0] != ARN_TRIGGER_1)
            {
                //error
                return CMD_RUN_ARM_ERROR;
            }

            command_rep.cmd = CMD_ARM_ACK;
            command_rep.size = 1;

            command_rep.data[0] = ARN_TRIGGER_1;
            command_rep.crc = 0x5151;

            write_command(&command_rep);

            //stage 2
            int error = 0;
            command_t* arm_cmd;
            while((arm_cmd = read_command(&error)) == NULL && error == CMD_READ_NO_CMD) {}

            if(error != OK || arm_cmd->cmd != CMD_ARM || arm_cmd->data[0] != ARN_TRIGGER_2)
            {
                //error
                return CMD_RUN_ARM_ERROR;
            }

            command_rep.data[0] = ARN_TRIGGER_2;
            write_command(&command_rep);

            //stage 3
            while((arm_cmd = read_command(&error)) == NULL && error == CMD_READ_NO_CMD) {}

            if(error != OK || arm_cmd->cmd != CMD_ARM || arm_cmd->data[0] != ARN_TRIGGER_3)
            {
                //error
                return CMD_RUN_ARM_ERROR;
            }

            command_rep.data[0] = ARN_TRIGGER_3;
            write_command(&command_rep);

            return CMD_RUN_OK;
        }
        break;

        case CMD_READY:
        {
            command_rep.cmd = CMD_READY_ACK;
            command_rep.size = 0;
            command_rep.crc = 0x2121;

            write_command(&command_rep);

            return CMD_RUN_OK;
        }
        break;

        case CMD_ABORT:
        {
            command_rep.cmd = CMD_ABORT_ACK;
            command_rep.size = 0;
            command_rep.crc = 0x2121;

            write_command(&command_rep);

            return CMD_RUN_OK;
        }
        break;

        default:
            // if the command has no action it still needs to return ok to change state
            if(cmd->cmd < cmd_size)
                return CMD_RUN_OK;
            else //cmd code out of bounds, return error
                return CMD_RUN_OUT_OF_BOUND;
        break;
    };

    
    //Serial.printf("cmd: %x state: %d return state %d table: %d\n", cmd->cmd, state, return_state,
    //(rocket_state_t)comm_transition[state][cmd->cmd]);
}

void gyroSetup(void)
{
    accelgyro.initialize();
    Wire.setBufferSize(256);
 // verify connection
    Serial.println("Testing device connections...");
    Serial.println(accelgyro.testConnection() ? "MPu6050 connection successful" : "MPu6050 connection failed");
    
}

void LoRa_Setup(void)
{
  LoRa.setPins(5,4,36);
  Serial.println("Lora starting");
  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

void setup() {

    Serial.begin(115200); //USBC serial

    // join I2C bus (I2Cdev library doesn't do this automatically)
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif
    
    gyroSetup();
    LoRa_Setup();

    //setup trigger switch
    pinMode(TRIGGER, INPUT_PULLUP);

    //Set board LED 
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    char arr[] = {'a','b','c','d'};
    //unsigned long crc_1 = crc((unsigned char*)arr, 4); 

/*
 Removed during testing. See Target.h
 */
#ifndef DIGITAL_TARGET

    Serial2.begin(115200);
#endif

    printf("Setup done\n");
}

void loop() {
    static rocket_state_t state = IDLE; 
    rocket_state_t work_state    = state, \
                   command_state = state, \
                   event_state   = state; 

    /*
        Execute the state function 
        and perform the default state transition 
     */
    bool work_performed = WORK_HANDLER();

    /*
     Event handling
     */
    if (work_performed) event_state = EVENT_HANDLER();
    if(event_state == -1) event_state = state;

    /*
     Comms
     */
    int error;

    //check if we have new data
    command_t* cmd = read_command(&error);
    if( cmd != NULL && 
        error == CMD_READ_OK && 
        run_command(cmd, state) == CMD_RUN_OK) 
    {
        //make transition to new state on the state machine
        if(state_machine[state].comms[cmd->cmd] != -1)
            //we have new state, use lookup table
            command_state = (rocket_state_t)comm_transition[state][cmd->cmd];
    }

    /*
     * Do state transition
     */
    if(command_state != state) 
        //command change of state as priority over
        //internal events changes of state
        state = command_state;
    else if(event_state != state)
        //only if comms haven't changed the state we can
        //acept a new state from internal events
        state = event_state;
    else if(work_state != state)
        //this is the lowest priority for state change
        //this can be used as a mechanisn to execute a work function only one time
        state = work_state;


    delay(1);
}
