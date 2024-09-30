#ifndef _STM_VAR_H_
#define _STM_VAR_H_

#include <inttypes.h>

#include "HardwareCfg.h"

#include <I2Cdev.h>
#include <MPU6050.h>
#include <HX711.h>
#include <Max6675.h>
#include <ADS1115_WE.h>
#include <MCP9600.h>

#include <Preferences.h>

//-----------Internal flash------------

extern Preferences preferences;
extern bool fast_reboot;


//----------- IMU vars ------------
extern MPU6050 accelgyro;

extern int16_t imu_ax;
extern int16_t imu_ay;
extern int16_t imu_az;

extern int16_t imu_gx;
extern int16_t imu_gy;
extern int16_t imu_gz;

//-----------LOADCELL--------------
extern LoadCell_Module Scale_Module;

//-----------Pressure AMP--------------
extern ADS1115_WE ADS1;
extern ADS1115_WE ADS2;

//----------- Line Modules ------------
extern Control_Module Tank_Top_Module;
extern Control_Module Tank_Bot_Module;
extern Engine_Module Chamber_Module;

//----------- Ultra sonic liquid sensors ------------
extern uint8_t tank_tactile_bits;
extern int16_t tank_t1;
extern int16_t tank_t2;
extern int16_t tank_t3;
extern int16_t tank_t4;
extern int16_t tank_t5;

//---------------Tank state--------------

extern float tank_liquid;
extern float tank_liquid2;

extern int16_t tank_pressure;

//moles of he initialy in the tank
extern float he_mol;
//total amount of moles lost 
extern float tank_mol_lost;
//last reading of moles in the tank
extern float last_tank_moles;

//----------------Tank constansts --------------
extern float g;
extern float Dt;
extern float Do;
extern float h1;
extern float h_tanque;
extern float V_total;

//----------------Tank intermidiate vars --------------
extern double mVL, mVG; // massa volumica Liquido(bot) e Gas(Top)

extern double hL;     // altura liquido
extern double Vl, Vg; // volume liquido / gas
extern double x;      // answer
extern double ml, mg;

extern double hL2;      // altura liquido
extern double Vl2, Vg2; // volume liquido / gas
extern double x2;       // answer
extern double ml2, mg2;

// pressure avgs
const int press_values_size = 10;
extern float ttp_values[press_values_size], tbp_values[press_values_size], chp_values[press_values_size];
extern int ttp_index, tbp_index, chp_index;

//---------------Tank CMD vars vars----------------
extern uint16_t RP1, RP2;
extern uint16_t RL1;

//-----------------Combustion chamber vars-----------
extern int16_t chamber_pressure;

//---------------Timers----------------
extern uint16_t arm_reset_timer;
#endif