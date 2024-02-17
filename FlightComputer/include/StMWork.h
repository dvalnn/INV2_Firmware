#ifndef STM_WORK_H_
#define STM_WORK_H_


/*
 * Define here all functions that can run as work in a state
 */

void idle_state(void);
void toggle_led_high(void);
void toggle_led_low(void);
void toggle_led(void);
void toggle_led_1000ms(void);
void toggle_led_500ms(void);
void toggle_led_200ms(void);

void imu_pid_calibration(void);
void read_IMU(void); 

#endif