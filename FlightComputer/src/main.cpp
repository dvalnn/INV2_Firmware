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

// I2Cdev and MPU6050 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include <I2Cdev.h>
#include <MPU6050.h>
#include <MPU9250.h>
#include <HX711.h>
#include <MAX6675.h>
#include <ADS1X15.h>
#include <MCP9600.h>
#include <Adafruit_BMP280.h>
#include <LoRa.h>
#include <Crc.h>
#include <SerialFlash.h>
#include <LittleFS.h>

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

#define FORMAT_LITTLEFS_IF_FAILED true

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for InvenSense evaluation board)
// AD0 high = 0x69
//MPU6050 accelgyro(0x69); // <-- use for AD0 high

bool fast_reboot = 0;

bool Launch = false;

void software_work(void* paramms);

void pressure_Setup(void)
{
    Serial.println("Pressure amp starting");

    if(!ADS.init()){
        Serial.println("ADS1115 not connected!");
    }
    
    ADS.setVoltageRange_mV(ADS1115_RANGE_6144); //comment line/change parameter to change range
    ADS.setCompareChannels(ADS1115_COMP_0_GND); //comment line/change parameter to change range
    ADS.setAlertPinMode(ADS1115_ASSERT_AFTER_2); // alternative: ...AFTER_2 or 4. If you disable this sketch does not work
    ADS.setConvRate(ADS1115_860_SPS); //uncomment if you want to change the default
    ADS.setMeasureMode(ADS1115_SINGLE); //comment or change you want to change to single shot
    ADS.setAlertPinToConversionReady(); //needed for this sketch
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

void sendPacket(byte *packet, byte len) {
    for (byte i = 0; i < len; i++)
    {
        Serial1.write(packet[i]);
    }
}

void changeBaudrate() {
    byte packet115200[] = {
      0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 
      0x00, 0xD0, 0x08, 0x00, 0x00, 0x00, 0xC2, 0x01, 0x00, 
      0x07, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x7E,
    };
    sendPacket(packet115200, sizeof(packet115200));
}

void changeFrequency() {
    byte packet[] = {
      0xB5,0x62,0x06,0x08,0x06,0x00,0xC8,0x00,0x01,0x00,0x01,0x00,0xDE,0x6A,
    };
    sendPacket(packet, sizeof(packet));
}

void GPS_Setup(void)
{
    Serial1.begin(9600); //GPS

    changeFrequency();
    delay(200);

    Serial1.flush();

    changeBaudrate();
    
    delay(200);

    Serial1.flush();

    Serial1.end();
    
    Serial1.begin(115200); //GPS
}

void IMU_Setup(void)
{
    IMU.verbose(true);
    //Serial.println("Gyro starting");
    IMU.setup(0x68, MPU9250Setting() ,Wire1);
    
    //IMU.initialize();
    Serial.println("Testing device connections...");
    Serial.println(IMU.isConnectedMPU9250() ? "MPU9250 connection successful" : "MPU9250 connection failed");
    Serial.println(IMU.isConnectedAK8963() ? "AK8963 connection successful" : "AK8963 connection failed");
    //Wire.setBufferSize(256);
 //// verify connection
    //Serial.println("Testing device connections...");
    //Serial.println(accelgyro.testConnection() ? "MPu6050 connection successful" : "MPu6050 connection failed");

    //IMU.calibrateAccelGyro();

}

void BAROMETER_Setup(void)
{
  bool success = bmp.begin(BMP_ADDR);
  Serial.printf("Succsess %d\n", success);

  bmp.setSampling(
                  //Adafruit_BMP280::MODE_FORCED,     /* Operating Mode. */
                  Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X16,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_OFF,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_1   /* Standby time. */
                  //Adafruit_BMP280::STANDBY_MS_250   /* Standby time. */
                  );
    ground_hPa = bmp.readPressure(); // in Si units for Pascal
    ground_hPa /= 100;
    //barometer_calibrate();
}

void Valves_Setup(void)
{
    pinMode(V1_PIN, OUTPUT);
    pinMode(V2_PIN, OUTPUT);
    pinMode(V3_PIN, OUTPUT);

    digitalWrite(V1_PIN, LOW);
    digitalWrite(V2_PIN, LOW);
    digitalWrite(V3_PIN, LOW);
}

void kalman_Setup(void)
{
    att.select_filter(QuatFilterSel::MAHONY);
    alt_kal.begin();
}

void Pyro_Setup(void)
{
    pinMode(MAIN_CHUTE_DEPLOY_PIN, OUTPUT);
    pinMode(DRAG_CHUTE_DEPLOY_PIN, OUTPUT);

    digitalWrite(MAIN_CHUTE_DEPLOY_PIN, LOW);
    digitalWrite(DRAG_CHUTE_DEPLOY_PIN, LOW);

    pinMode(MAIN_CHUTE_READ, INPUT);
    pinMode(DRAG_CHUTE_READ, INPUT);
}

void Flash_Setup()
{
    //if (!SD.begin(Flash_SS_PIN)) {
        //printf("Unable to access SPI Flash chip\n");
        ////return;
    //}

    //current_id = get_last_id() + 1;
    
    //if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
        //Serial.println("LittleFS Mount Failed");
        //return;
    //}


}


void LoRa_Setup(void)
{
  LoRa.setPins(LORA_SS_PIN, LORA_RESET_PIN, LORA_DIO0_PIN);
  LoRa.setSignalBandwidth(300E3);
  //LoRa.setSignalBandwidth(500E3);
  LoRa.setCodingRate4(8);
  LoRa.setSpreadingFactor(12);
  LoRa.setGain(6);

  Serial.println("Lora starting");
  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    //while (1);
  }
}

void dummy_func(void* parameters) {}

void setup() {
    Serial.begin(SERIAL_BAUD); //USBC serial
    Serial2.begin(SERIAL2_BAUD); //RS485

    Wire.begin();
    Wire1.begin();

    SPI.begin();
    //SPI.setFrequency(800000000);
    
    Pyro_Setup();
    Valves_Setup();

    pinMode(LORA_SS_PIN, OUTPUT);
    pinMode(Flash_SS_PIN, OUTPUT);

    digitalWrite(LORA_SS_PIN, HIGH);
    digitalWrite(Flash_SS_PIN, HIGH);

    LoRa_Setup();
    //Flash_Setup();

    GPS_Setup();

    //IMU_Setup();
    //BAROMETER_Setup();
    //kalman_Setup();

    pressure_Setup();
    
    if(! fast_reboot) temp_i2c_Setup();

    printf("Setup done\n");
}

void loop() { }

void software_work(void* paramms) {

    Serial.printf("got to stm work\n\r");

    while(true)
    {
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

        //command state transitions must take precedence to event state transitions
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
    }
}