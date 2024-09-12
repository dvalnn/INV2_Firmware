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

//----------------Tank intermidiate vars --------------
extern double mVL, mVG; // massa volumica Liquido(bot) e Gas(Top)

extern double hL; //altura liquido
extern double Vl, Vg; //volume liquido / gas
extern double x; //answer
extern double ml, mg;

extern double hL2; //altura liquido
extern double Vl2, Vg2; //volume liquido / gas
extern double x2; //answer
extern double ml2, mg2;


//---------------Tank CMD vars vars----------------
extern uint16_t RP1, RP2;
extern uint16_t RL1;

//-----------------Combustion chamber vars-----------
extern int16_t chamber_pressure;

//---------------Timers----------------
extern uint16_t arm_reset_timer;
#endif