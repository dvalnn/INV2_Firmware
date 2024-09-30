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

// I2Cdev and MPU6050 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include <I2Cdev.h>
#include <MPU6050.h>
#include <HX711.h>
#include <Max6675.h>
#include <ADS1X15.h>
#include <MCP9600.h>

#include <LoRa.h>

#include <Crc.h>

#include <SerialFlash.h>

#include <Preferences.h>

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for InvenSense evaluation board)
// AD0 high = 0x69
//MPU6050 accelgyro(0x69); // <-- use for AD0 high

Preferences preferences;
bool fast_reboot = 0;

void pressure_Setup(void)
{
    //Serial.println("Pressure amp starting");

    if(!ADS1.init()){
        //Serial.println("ADS1115 not connected!");
    }
    
    ADS1.setVoltageRange_mV(ADS1115_RANGE_6144); //comment line/change parameter to change range
    ADS1.setCompareChannels(ADS1115_COMP_0_GND); //comment line/change parameter to change range
    ADS1.setAlertPinMode(ADS1115_ASSERT_AFTER_2); // alternative: ...AFTER_2 or 4. If you disable this sketch does not work
    ADS1.setConvRate(ADS1115_860_SPS); //uncomment if you want to change the default
    ADS1.setMeasureMode(ADS1115_SINGLE); //comment or change you want to change to single shot
    ADS1.setAlertPinToConversionReady(); //needed for this sketch

    //ADS1.begin();
    //ADS1.setGain(1); //6v
    ////ADS1.setDataRate(4); //16 hz
    //ADS1.setDataRate(5); //16 hz
    //ADS1.setMode(1); //single mode
    
    //ADS2.begin();
    //ADS2.setGain(0); //6v
    //ADS2.setDataRate(7); //fastest
    //ADS2.setMode(1); //single mode
}

void temp_i2c_Setup(void)
{
    
    //Serial.println("Temp 1 i2c amp starting");
    Tank_Top_Module.thermocouple.begin(Tank_Top_Module.thermocouple_addr);
    Tank_Top_Module.thermocouple.setThermocoupleType(TYPE_K);
    //check if the sensor is connected
    if(Tank_Top_Module.thermocouple.isConnected()){
        //Serial.println("Device will acknowledge!");
        Tank_Top_Module.thermocouple.getThermocoupleTemp();
    }
    else {
        //Serial.println("Device did not acknowledge! Freezing.");
        //while(1); //hang forever
    }

    //Serial.println("Temp 2 i2c amp starting");
    Tank_Bot_Module.thermocouple.begin(Tank_Bot_Module.thermocouple_addr);
    Tank_Bot_Module.thermocouple.setThermocoupleType(TYPE_K);
    //check if the sensor is connected
    if(Tank_Bot_Module.thermocouple.isConnected()){
        //Serial.println("Device will acknowledge!");
        Tank_Bot_Module.thermocouple.getThermocoupleTemp();
    }
    else {
        //Serial.println("Device did not acknowledge! Freezing.");
        while(1); //hang forever
    }
}

void loadCell_Setup(void)
{
    //Serial.println("Loadcells starting\n");

    Scale_Module.scale1.begin(LOADCELL1_OUT_PIN, LOADCELL1_SCK_PIN);
    //loadcell.tare(100); // average 20 measurements.
    Scale_Module.scale1.set_offset(Scale_Module.scale1_offset);
    Scale_Module.scale1.set_scale(Scale_Module.scale1_scale);

    Scale_Module.scale2.begin(LOADCELL2_OUT_PIN, LOADCELL2_SCK_PIN);
    Scale_Module.scale2.set_offset(Scale_Module.scale2_offset);
    Scale_Module.scale2.set_scale(Scale_Module.scale2_scale);

    Scale_Module.scale3.begin(LOADCELL3_OUT_PIN, LOADCELL3_SCK_PIN);
    Scale_Module.scale3.set_offset(Scale_Module.scale3_offset);
    Scale_Module.scale3.set_scale(Scale_Module.scale3_scale);
}

void gyroSetup(void)
{
    Serial.println("Gyro starting");
    accelgyro.initialize();
    Wire.setBufferSize(256);
 // verify connection
    Serial.println("Testing device connections...");
    Serial.println(accelgyro.testConnection() ? "MPu6050 connection successful" : "MPu6050 connection failed");
    
}

void Valves_Setup(void)
{
    //Serial.println("Valves starting");
    pinMode(V1_PIN, OUTPUT);
    pinMode(V2_PIN, OUTPUT);

    //pinMode(Pressure_PIN, INPUT);
}

void Flash_Setup()
{
    if (!SD.begin(Flash_SS_PIN)) {
        //printf("Unable to access SPI Flash chip\n");
        //return;
    }

    current_id = get_last_id() + 1;
    //Serial.print("current id");
    //Serial.print(current_id);
    //Serial.printf("\n");

    //Serial.print("Card type ");
    //Serial.println(SD.cardType());

    //Serial.print("Used bytes ");
    //Serial.println(SD.usedBytes());

    //Serial.print("Capacity ");
    //Serial.println(SD.cardSize());

}

void Spi_Thermo_Setup(void)
{
    Chamber_Module.thermocouple1.begin();
    Chamber_Module.thermocouple2.begin();
    Chamber_Module.thermocouple3.begin();
    Chamber_Module.thermocouple1.setSPIspeed(100000);
    Chamber_Module.thermocouple2.setSPIspeed(100000);
    Chamber_Module.thermocouple3.setSPIspeed(100000);
}

void LoRa_Setup(void)
{
  LoRa.setPins(LORA_SS_PIN, LORA_RESET_PIN, LORA_DIO0_PIN);
  LoRa.setSignalBandwidth(300E3);
  //LoRa.setSignalBandwidth(500E3);
  //LoRa.setCodingRate4(5);
  //LoRa.setSpreadingFactor(7);
  //LoRa.setGain(1);

  //Serial.println("Lora starting");
  if (!LoRa.begin(868E6)) {
    //Serial.println("Starting LoRa failed!");
    //while (1);
  }
}


void setup() {

    Serial.begin(SERIAL_BAUD); //USBC serial
    Serial2.begin(SERIAL2_BAUD);

    preferences.begin("config", false);

    uint8_t restart_count = preferences.getUInt("restart_count", 0);
    rocket_state_t last_state = preferences.getUInt("last_state", 0);

    // join I2C bus (I2Cdev library doesn't do this automatically)
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
        //Wire.setClock(400000);
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif
    
    SPI.begin();

    pinMode(TEMP_AMP3_SS_PIN, OUTPUT);
    pinMode(TEMP_AMP4_SS_PIN, OUTPUT);
    pinMode(TEMP_AMP5_SS_PIN, OUTPUT);
    pinMode(LORA_SS_PIN, OUTPUT);
    pinMode(Flash_SS_PIN, OUTPUT);

    digitalWrite(TEMP_AMP3_SS_PIN, HIGH);
    digitalWrite(TEMP_AMP4_SS_PIN, HIGH);
    digitalWrite(TEMP_AMP5_SS_PIN, HIGH);
    digitalWrite(LORA_SS_PIN, HIGH);
    digitalWrite(Flash_SS_PIN, HIGH);

    LoRa_Setup();
    Flash_Setup();
    //printf("Last state %u\n", last_state);
    //printf("Restart_count %u\n", restart_count);

    if(last_state == LAUNCH && restart_count < MAX_RESTART_ALLOWED)
    {
        state = last_state;
        preferences.putUInt("restart_count", restart_count + 1);
        start_log();
        fast_reboot = 1;
    }
    else if(last_state == ABORT && restart_count < MAX_RESTART_ALLOWED)
    {
        state = last_state;
        preferences.putUInt("restart_count", restart_count + 1);
        start_log();
        fast_reboot = 1;
    }
    else
    {
        preferences.putUInt("restart_count", 0);
        preferences.putUInt("last_state", 0);
    }

    //gyroSetup();

    Valves_Setup();

    loadCell_Setup();

    pressure_Setup();

    if(! fast_reboot)
    {
        temp_i2c_Setup();
        Spi_Thermo_Setup();
    }

    printf("Setup done\n");

    //delay(2000);
    //while(1){}
}

void loop() {
    static bool init_flag = false;
    if(!init_flag)
    {
        state_machine[state].entry_time = millis();
        for(int i = 0; i < MAX_WORK_SIZE; i++)
            state_machine[state].work[i].begin = 
                state_machine[state].work[i].delay;
        init_flag = true;
    }

    rocket_state_t command_state = state, \
                   event_state   = state; 

    /*
        Execute the state function 
     */
    bool work_performed = WORK_HANDLER();

    /*
     Event handling
     */
    //if (work_performed) event_state = EVENT_HANDLER();
    event_state = EVENT_HANDLER();
    if(event_state == -1) event_state = state;

    /*
     Comms
     */
    int error;

    //check if we have new data
    //if we get a valid message, execute the command associated to it
    command_t* cmd = read_command(&error, DEFAULT_CMD_INTERFACE);
    if( cmd != NULL && error == CMD_READ_OK) 
    {
        int error = run_command(cmd, state, DEFAULT_CMD_INTERFACE); 

        //make transition to new state on the state machine
        if(error == CMD_RUN_OK && 
           state_machine[state].comms[cmd->cmd] != -1)
        {
            //we have new state, use lookup table
            command_state = (rocket_state_t)comm_transition[state][cmd->cmd];
            //Serial2.printf("change state to %d\n", state_machine[state].comms[cmd->cmd]);
        }
        else if (error != CMD_RUN_OK)
        {
            //log cmd execution error
            //Serial2.printf("EXECUTING MESSAGE ERROR %d\n", error);
        }
    }
    else if(error != CMD_READ_OK && 
            error != CMD_READ_NO_CMD)
    {
        //log cmd read error
        //Serial.printf("READING MESSAGE ERROR %d\n", error);
    }

    /*
     * Do state transition
     */
    if(command_state != state) 
    {
        //command change of state as priority over
        //internal events changes of state
        state = command_state;
        state_machine[state].entry_time = millis();
        
        //reset sensor timer
        for(int i = 0; i < MAX_WORK_SIZE; i++)
            state_machine[state].work[i].begin = 
                state_machine[state].work[i].delay;
        
        log(&state, 0, STATE_CHANGE);
    }

    //used as the time base when dealing with sensor sampling rate and delays
    else if(event_state != state)
    {
        //only if comms haven't changed the state we can
        //acept a new state from internal events
        state = event_state;
        state_machine[state].entry_time = millis();
        
        //reset sensor timer
        for(int i = 0; i < MAX_WORK_SIZE; i++)
            state_machine[state].work[i].begin = 
                state_machine[state].work[i].delay;
            
        log(&state, 0, STATE_CHANGE);
    }

    //save last state
    preferences.putUInt("last_state", state);

    //delay(1);
}
