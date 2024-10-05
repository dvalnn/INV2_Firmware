#ifndef STM_WORK_H_
#define STM_WORK_H_

#include "HardwareCfg.h"
#include "StateMachine.h"

/*
 * Define here all functions that can run as work in a state
 */

void V_Vpu_close(void);
void V_Engine_close(void);

void V_Vpu_open(void);
void V_Engine_open(void);

void read_temperature_tank_top(void);
void read_temperature_tank_bot(void);

void read_pressure_tank_top(void);
void read_pressure_tank_bot(void);

void read_weight1(void);
void read_weight2(void);
void read_weight3(void);

void read_temperature_chamber_1(void);
void read_temperature_chamber_2(void);
void read_temperature_chamber_3(void);

void read_imu(void); 
void read_barometer(void);
void read_gps(void);

void kalman(void);

void read_chamber_pressure(void);

void read_tank_tactile(void);

void calc_liquid(void);

void imu_pid_calibration(void);

void flash_log_sensors(void);
void logger(void);

void reset_timers(void);
void arm_timer_tick(void);


void single_mode_stm();

#endif