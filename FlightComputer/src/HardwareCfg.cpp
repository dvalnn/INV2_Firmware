#include <Arduino.h>

#include "HardwareCfg.h"
#include "GlobalVars.h"
#include "StmWork.h"

#include <Max6675.h>
#include <ADS1X15.h>
#include <HX711.h>
#include <MPU6050.h>

ADS1115_WE ADS(PRESSURE_AMP_ADDR);

//MPU6050 IMU;
MPU9250 IMU;
Adafruit_BMP280 bmp(&Wire1);
TinyGPSPlus gps;

const float cuttoff_ferquency_alt = (1.0/(float(BMP_READ_TIME)*1000.0))/4; 
const float betha_alt = 1-exp(-BMP_READ_TIME*1000 * 2 * M_PI * cuttoff_ferquency_alt); 

const float cuttoff_ferquency_imu = (1.0/(float(IMU_READ_TIME)*1000.0))/4; 
const float betha_imu = 1-exp(-IMU_READ_TIME*1000 * 2 * M_PI * cuttoff_ferquency_imu); 


Control_Module Tank_Top_Module = 
{
    .valve_pin = V1_PIN,

    .ADC = &ADS,
    .ADC_pressure_id = ADS1115_COMP_0_GND,
    .pressure_serial = K1WIJ,

    .thermocouple_addr = TEMP_AMP2_ADDR,
};

Control_Module Tank_Bot_Module = 
{
    .valve_pin = V2_PIN,
    
    .ADC = &ADS,
    .ADC_pressure_id = ADS1115_COMP_1_GND,
    .pressure_serial = K2LI9,

    .thermocouple_addr = TEMP_AMP1_ADDR,
};

Engine_Module Chamber_Module =
{
    .valve_pin = V3_PIN,

    .ADC = &ADS,
    .ADC_pressure_id = ADS1115_COMP_2_GND,
    .pressure_serial = K0P2A,

}; 
