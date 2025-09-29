#ifndef _STM_VAR_H_
#define _STM_VAR_H_

#include <inttypes.h>

#include "HardwareCfg.h"
#include "kalman.h"

#include <I2Cdev.h>
#include <ADS1115_WE.h>
#include <MCP9600.h>
#include <MPU9250.h>
#include <MPU6050.h>
#include <TinyGPSPlus.h>
#include <Adafruit_BMP280.h>

#include <ArduinoEigen.h>

//-----------Internal flash------------
extern bool fast_reboot;

//----------- IMU vars ------------
extern MPU9250 IMU;
//extern MPU6050 IMU;

extern float imu_ax,imu_ay,imu_az; //accel
extern float imu_gx,imu_gy,imu_gz; //gyro

extern const float betha_imu;

//------------ Barometer -----------
extern Adafruit_BMP280 bmp;
extern float altitude;

extern const float betha_alt;

extern float ground_hPa;

extern unsigned long transmit_time;

//------------- GPS ----------------
extern TinyGPSPlus gps;
extern float gps_lat, gps_lon;
extern float gps_altitude;
extern float gps_horizontal_vel;
extern uint16_t gps_satalites;

//------------ Kalman -------------


extern bool Launch;
extern bool DragDeployed;
extern bool MainDeployed;
extern float maxAltitude;
extern Eigen::Matrix<float, 9,1> alt_kalman_state; // x, v_x, a_x, y, v_y, a_y, z, v_z, a_z
extern alt_kalman alt_kal;
extern QuaternionFilter att;
extern float q[4];

extern float kalman_altitude;
extern float kalman_velocity;
extern float kalman_accel;
extern float kalman_q[4];

extern bool debug_flag;
//-----------Pressure AMP--------------
extern ADS1115_WE ADS;

//----------- Line Modules ------------
extern Control_Module Tank_Top_Module;
extern Control_Module Tank_Bot_Module;
extern Engine_Module Chamber_Module;

// pressure avgs
const int press_values_size = 10;
extern float ttp_values[press_values_size], tbp_values[press_values_size], chp_values[press_values_size];
extern int ttp_index, tbp_index, chp_index;

//---------------EMATCH state----------------
extern uint16_t ematch_main_reading;
extern uint16_t ematch_drag_reading;

//---------------Tank CMD vars vars----------------
extern uint16_t RP1, RP2;
extern uint16_t RL1;

//---------------Timers----------------
extern uint16_t arm_reset_timer;
extern uint16_t burn_timer;
extern uint16_t depressur_timer;
extern uint16_t depressur_global_timer;
#endif