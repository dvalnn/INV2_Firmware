#ifndef STM_WORK_H_
#define STM_WORK_H_


/*
 * Define here all functions that can run as work in a state
 */

void idle_state(void);
void toggle_led_high(void);
void toggle_led_low(void);
void toggle_led(void);

void read_pressures(void);
void read_temperatures(void);

void V1_close(void);
void V2_close(void);
void V3_close(void);

void V1_open(void);
void V2_open(void);
void V3_open(void);

void logger(void);
void pressure_safety(void);
#endif