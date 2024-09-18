#include <Arduino.h>

#include "HardwareCfg.h"
#include "StmWork.h"

#include <Max6675.h>
#include <ADS1X15.h>
#include <HX711.h>
#include <MPU6050.h>

MPU6050 accelgyro;
HX711 scale1;
HX711 scale2;
HX711 scale3;
ADS1115_WE ADS1(PRESSURE_AMP1_ADDR);
ADS1115_WE ADS2(PRESSURE_AMP2_ADDR);

Control_Module Tank_Top_Module = 
{
    .valve_pin = V1_PIN,

    .ADC = &ADS1,
    .ADC_pressure_id = ADS1115_COMP_0_GND,
    .pressure_serial = K1WIJ,

    .thermocouple_addr = TEMP_AMP1_ADDR,
};

Control_Module Tank_Bot_Module = 
{
    .valve_pin = V2_PIN,
    
    .ADC = &ADS1,
    .ADC_pressure_id = ADS1115_COMP_1_GND,
    .pressure_serial = K0P2A,

    .thermocouple_addr = TEMP_AMP2_ADDR,
};

Engine_Module Chamber_Module =
{
    .ADC = &ADS1,
    .ADC_pressure_id = ADS1115_COMP_2_GND,
    .pressure_serial = K2LI9,

    .thermocouple1 = MAX6675(TEMP_AMP3_SS_PIN, &SPI),
    .thermocouple2 = MAX6675(TEMP_AMP4_SS_PIN, &SPI),
    .thermocouple3 = MAX6675(TEMP_AMP5_SS_PIN, &SPI)
}; 

LoadCell_Module Scale_Module = 
{
    .scale1 = HX711(),
    .scale1_offset = 86.43530,
    .scale1_scale = -0.00447,

    .scale2 = HX711(),
    .scale2_offset = 88.48772,
    .scale2_scale = -0.004434,

    .scale3 = HX711(),
    .scale3_offset = 76.38184,
    .scale3_scale = -0.0044029,
};