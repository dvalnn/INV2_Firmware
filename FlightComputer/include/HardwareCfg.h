#ifndef HARDWARE_CFG_H_
#define HARDWARE_CFG_H_

#include <inttypes.h>

#include <MCP9600.h>
#include <Max6675.h>
#include <HX711.h>
#include <ADS1115_WE.h> 

#define SPI_SCLK_PIN 18
#define SPI_MISO_PIN 19
#define SPI_MOSI_PIN 23

#define V1_PIN 33
#define V2_PIN 32

#define Flash_SS_PIN 0

#define LORA_SS_PIN 12
#define LORA_RESET_PIN 5
#define LORA_DIO0_PIN -1

#define TEMP_AMP3_SS_PIN 4
#define TEMP_AMP4_SS_PIN 27
#define TEMP_AMP5_SS_PIN 26

#define LOADCELL1_OUT_PIN 36 
#define LOADCELL2_OUT_PIN 39 
#define LOADCELL3_OUT_PIN 34 

#define LOADCELL1_SCK_PIN 14
#define LOADCELL2_SCK_PIN 13
#define LOADCELL3_SCK_PIN 25

//---------------BAUD-----------------
#define SERIAL_BAUD 115200
#define SERIAL2_BAUD 115200

//---------------ADDR-----------------
#define TEMP_AMP1_ADDR 0x60
#define TEMP_AMP2_ADDR 0x67

#define PRESSURE_AMP1_ADDR 0x48
#define PRESSURE_AMP2_ADDR 0x49

//---------------TACTILE----------------
#define TACTILE_THREADHOLD 23000

//---------------CALIB Values-----------------
typedef struct { float m; float b; } pressure_calib;

#define S5A2D {20.06008, -0.22376}
#define JYJ31 {20.04804, -0.21367}
#define K2LI9 {20.06813, -0.22385}
#define JX8IQ {20.04801, -0.20689}
#define K0P2A {20.04805, -0.19363}
#define K1WIJ {20.07217, -0.26072}

#define P1_Serial_No S5A2D
#define P2_Serial_No JYJ31
#define P3_Serial_No JX8IQ

//--------------sensors and actuators ---------------

typedef struct 
{
    uint8_t valve_pin;

    ADS1115_WE* ADC;
    mux ADC_pressure_id;
    pressure_calib pressure_serial;
    
    MCP9600 thermocouple;
    uint8_t thermocouple_addr;

    float pressure;    
    int16_t temperature;
    bool valve_state;

} Control_Module;

typedef struct
{
    ADS1115_WE* ADC;
    mux ADC_pressure_id;
    pressure_calib pressure_serial;

    MAX6675 thermocouple1;
    MAX6675 thermocouple2;
    MAX6675 thermocouple3;

    float pressure;    
    
    int16_t temperature1;
    int16_t temperature2;
    int16_t temperature3;

} Engine_Module;

typedef struct
{
    HX711 scale1;
    //int32_t scale1_offset;
    float scale1_offset;
    float scale1_scale;

    HX711 scale2;
    //int32_t scale2_offset;
    float scale2_offset;
    float scale2_scale;

    HX711 scale3;
    //int32_t scale3_offset;
    float scale3_offset;
    float scale3_scale;

    int16_t weight1;
    int16_t weight2;
    int16_t weight3;
} LoadCell_Module;


#endif