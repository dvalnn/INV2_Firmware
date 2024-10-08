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

#define I2C_SDA_1_PIN 21
#define I2C_SCL_1_PIN 22

#define I2C_SDA_2_PIN 2
#define I2C_SCL_2_PIN 15

#define V1_PIN 25
#define V2_PIN 26
#define V3_PIN 4

#define Flash_SS_PIN 14

#define LORA_SS_PIN 12
#define LORA_RESET_PIN 13
#define LORA_DIO0_PIN -1

#define GPS_RX_PIN 27
#define GPS_TX_PIN 5

//--------------PARACHUTES------------
#define MAIN_CHUTE_DEPLOY_PIN 33
#define DRAG_CHUTE_DEPLOY_PIN 32

#define MAIN_CHUTE_READ 39
#define DRAG_CHUTE_READ 34

//---------------BAUD-----------------
#define SERIAL_BAUD 115200
#define SERIAL1_BAUD 9600 
#define SERIAL2_BAUD 115200

//---------------ADDR-----------------
#define TEMP_AMP1_ADDR 0x60
#define TEMP_AMP2_ADDR 0x67

#define PRESSURE_AMP_ADDR 0x48

#define IMU_ADDR 0x68

#define BMP_ADDR 0x76
//---------------SENSORS-------------------
#define IMU_READ_TIME 10 //ms
#define BMP_READ_TIME 10 //ms

#define FREEFALL_ALTITUDE_THRESHOLD 2 //free-fall considered if altitude decreases by more than this
#define FREEFALL_THRESHOLD 3 //free-fall considered if global acceleration is smaller than it 
#define WAY_TOO_MUCH_SPEED 1

#define OFFSET_X 0.0075
#define OFFSET_Y -0.015
#define OFFSET_Z 0.045

#define MAX_STORAGE 100 //Kalman

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
    uint8_t valve_pin;

    ADS1115_WE* ADC;
    mux ADC_pressure_id;
    pressure_calib pressure_serial;

    float pressure;      
    bool valve_state;
} Engine_Module;


#endif