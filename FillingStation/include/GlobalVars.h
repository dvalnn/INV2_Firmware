#ifndef _STM_VAR_H_
#define _STM_VAR_H_

#include <inttypes.h>

#include <I2Cdev.h>
#include <MPU6050.h>

//----------- IMU vars ------------
extern MPU6050 accelgyro;

//lattest readding s from the imu sensor
extern int16_t imu_ax;
extern int16_t imu_ay;
extern int16_t imu_az;

extern int16_t imu_gx;
extern int16_t imu_gy;
extern int16_t imu_gz;

//transitional variables for states when calibrating
extern uint8_t brackets_out_done;
extern uint8_t brackets_in_done;


#endif