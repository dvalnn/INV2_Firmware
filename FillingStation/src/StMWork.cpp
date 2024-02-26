#include <Arduino.h>

#include <time.h>

#include "HardwareCfg.h"
#include "StMWork.h"
#include "GlobalVars.h"

#include <I2Cdev.h>
#include <MPU6050.h>

int16_t imu_ax;
int16_t imu_ay;
int16_t imu_az;

int16_t imu_gx;
int16_t imu_gy;
int16_t imu_gz;

void idle_state(void) { return; }

void toggle_led_high(void)
{
    digitalWrite(LED_PIN, HIGH);
}

void toggle_led(void)
{
    static int led_status = 0;
    led_status ^= 1;
    digitalWrite(LED_PIN, led_status);
}

void read_pressures(void)
{
    //Dummy function
    return;
}

void read_temperatures(void)
{
    //Dummy function
    return;
}

void V1_close(void)
{

}

void V2_close(void)
{

}

void V3_close(void)
{

}

void V1_open(void)
{

}

void V2_open(void)
{

}

void V3_open(void)
{

}

void logger(void)
{

}

void pressure_safety(void)
{

}