#ifndef STM_WORK_H_
#define STM_WORK_H_

#include "HardwareCfg.h"
#include "StateMachine.h"

/*
 * Define here all functions that can run as work in a state
 */

void V_Vpu_close(void);
void V_Purge_close(void);
void V_Engine_close(void);

void V_Vpu_open(void);
void V_Purge_open(void);
void V_Engine_open(void);

void main_ematch_high(void);
void main_ematch_low(void);
void drag_ematch_high(void);
void drag_ematch_low(void);

void read_temperature_tank_top(void);
void read_temperature_tank_bot(void);

void read_pressure_tank_top(void);
void read_pressure_tank_bot(void);

void barometer_calibrate(void);
void imu_calibrate(void);
void kalman_calibrate(void);

void read_imu(void); 
void read_barometer(void);
void read_gps(void);
void kalman(void);

void read_chamber_pressure(void);

void flash_log_sensors(void);
void logger(void);
void telemetry(void);

void reset_timers(void);
void arm_timer_tick(void);
void burn_timer_tick(void);


void single_mode_stm();

#endif