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

#include <LoRa.h>

#include <Crc.h>

#include <SerialFlash.h>

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

void loadCell_Setup(void)
{
    Serial.println("Load CELL starting");
    scale1.begin(LOADCELL_OUT_PIN, LOADCELL_SCK_PIN);
}

void gyroSetup(void)
{
    accelgyro.initialize();
    Wire.setBufferSize(256);
 // verify connection
    Serial.println("Testing device connections...");
    Serial.println(accelgyro.testConnection() ? "MPu6050 connection successful" : "MPu6050 connection failed");
    
}

void Flash_Setup()
{
    if (!SD.begin(Flash_SS_PIN)) {
        printf("Unable to access SPI Flash chip\n");
        return;
    }

    current_id = get_last_id() + 1;
    Serial.print("current id");
    Serial.println(current_id);

    Serial.print("Card type ");
    Serial.println(SD.cardType());

    Serial.print("Used bytes ");
    Serial.println(SD.usedBytes());

    Serial.print("Capacity ");
    Serial.println(SD.cardSize());
}

void LoRa_Setup(void)
{
  LoRa.setPins(LORA_SS_PIN, LORA_RESET_PIN, LORA_DIO0_PIN);
  LoRa.setSignalBandwidth(300E3);
  Serial.println("Lora starting");
  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}


void Spi_Thermo_Setup(void)
{
    Serial.println("Temp spi amp starting");
    
    He_Module.thermocouple.begin();
    N2O_Module.thermocouple.begin();
    Line_Module.thermocouple.begin();
    
    He_Module.thermocouple.setSPIspeed(100000);
    N2O_Module.thermocouple.setSPIspeed(100000);
    Line_Module.thermocouple.setSPIspeed(100000);
}

void Pressure_Setup(void)
{
    Serial.println("Pressure amp starting");
    if(!ADS.init()){
        Serial.println("ADS1115 not connected!");
    }
    
    ADS.setVoltageRange_mV(ADS1115_RANGE_6144); //comment line/change parameter to change range
    ADS.setCompareChannels(ADS1115_COMP_0_GND); //comment line/change parameter to change range
    ADS.setAlertPinMode(ADS1115_ASSERT_AFTER_4); // alternative: ...AFTER_2 or 4. If you disable this sketch does not work
    ADS.setConvRate(ADS1115_475_SPS); //uncomment if you want to change the default
    ADS.setMeasureMode(ADS1115_SINGLE); //comment or change you want to change to single shot
    ADS.setAlertPinToConversionReady(); //needed for this sketch
}

void Valves_Setup(void)
{
    pinMode(He_Module.valve_pin, OUTPUT);
    pinMode(N2O_Module.valve_pin, OUTPUT);
    pinMode(Line_Module.valve_pin, OUTPUT);
}

void setup() {

    Serial.begin(SERIAL_BAUD); //USBC serial
    Serial2.begin(SERIAL2_BAUD); //RS485

    // join I2C bus (I2Cdev library doesn't do this automatically)
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
        Wire.setClock(400000);
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif
    

    pinMode(TEMP_AMP1_SS_PIN, OUTPUT);
    pinMode(TEMP_AMP2_SS_PIN, OUTPUT);
    pinMode(TEMP_AMP3_SS_PIN, OUTPUT);
    pinMode(LORA_SS_PIN, OUTPUT);
    pinMode(Flash_SS_PIN, OUTPUT);
    pinMode(EMATCH_IGNITE_PIN, OUTPUT);

    digitalWrite(TEMP_AMP1_SS_PIN, HIGH);
    digitalWrite(TEMP_AMP2_SS_PIN, HIGH);
    digitalWrite(TEMP_AMP3_SS_PIN, HIGH);
    digitalWrite(LORA_SS_PIN, HIGH);
    digitalWrite(Flash_SS_PIN, HIGH);
    digitalWrite(EMATCH_IGNITE_PIN, LOW);

    SPI.begin();
    
    //gyroSetup();
    
    Valves_Setup();

    loadCell_Setup();

    Pressure_Setup();

    Spi_Thermo_Setup();

    LoRa_Setup();
    //Flash_Setup();


    printf("Setup done\n");

    state_machine[state].entry_time = millis();
    for(int i = 0; i < MAX_WORK_SIZE; i++)
        state_machine[state].work[i].begin = 
            state_machine[state].work[i].delay;
    
    //while(1) {}
}

void loop() {
    //static rocket_state_t state = IDLE; 
    rocket_state_t command_state = state, \
                   event_state   = state; 

    /*
        Execute the state function 
     */
    bool work_performed = WORK_HANDLER();

    /*
     Event handling
     */
    if (work_performed) event_state = EVENT_HANDLER();
    if (event_state == -1) event_state = state;

    /*
     Comms
     */
    int error;

    //check if we have new data
    command_t* cmd = read_command(&error, DEFAULT_CMD_INTERFACE);
    if( cmd != NULL && 
        error == CMD_READ_OK && 
        run_command(cmd, state, DEFAULT_CMD_INTERFACE) == CMD_RUN_OK) 
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


    delay(1);
}
