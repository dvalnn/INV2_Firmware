#include <Arduino.h>

#include <time.h>

#include "HardwareCfg.h"
#include "StMWork.h"
#include "GlobalVars.h"
#include "Comms.h"
#include "FlashLog.h"

#include <I2Cdev.h>
#include <MPU6050.h>
#include <HX711.h>
#include <Max6675.h>
#include <ADS1115_WE.h>
#include <Crc.h>

float imu_ax;
float imu_ay;
float imu_az;

float imu_gx;
float imu_gy;
float imu_gz;

float imu_mx;
float imu_my;
float imu_mz;

float altitude;
float maxAltitude;

int16_t tank_pressure = 0;
float tank_liquid = 0;

uint16_t arm_reset_timer;

float ttp_values[press_values_size], tbp_values[press_values_size], chp_values[press_values_size];
int ttp_index = 0, tbp_index = 0, chp_index = 0;

void V_Vpu_close(void)
{
    digitalWrite(Tank_Top_Module.valve_pin, 0);
    Tank_Top_Module.valve_state = 0;
}
void V_Purge_close(void)
{
    digitalWrite(Tank_Bot_Module.valve_pin, 0);
    Tank_Bot_Module.valve_state = 0;
}
void V_Engine_close(void)
{
    digitalWrite(Chamber_Module.valve_pin, 0);
    Chamber_Module.valve_state = 0;
}

void V_Vpu_open(void)
{
    digitalWrite(Tank_Top_Module.valve_pin, 1);
    Tank_Top_Module.valve_state = 1;
}
void V_Purge_open(void)
{
    digitalWrite(Tank_Bot_Module.valve_pin, 1);
    Tank_Bot_Module.valve_state = 1;
}
void V_Engine_open(void)
{
    digitalWrite(Chamber_Module.valve_pin, 1);
    Chamber_Module.valve_state = 1;
}

void imu_pid_calibration(void)
{

    /*A tidbit on how PID (PI actually) tuning works.
      When we change the offset in the MPU6050 we can get instant results. This allows us to use Proportional and
      integral of the PID to discover the ideal offsets. Integral is the key to discovering these offsets, Integral
      uses the error from set-point (set-point is zero), it takes a fraction of this error (error * ki) and adds it
      to the integral value. Each reading narrows the error down to the desired offset. The greater the error from
      set-point, the more we adjust the integral value. The proportional does its part by hiding the noise from the
      integral math. The Derivative is not used because of the noise and because the sensor is stationary. With the
      noise removed the integral value lands on a solid offset after just 600 readings. At the end of each set of 100
      readings, the integral value is used for the actual offsets and the last proportional reading is ignored due to
      the fact it reacts to any noise.
    */
    //if (!accelgyro.testConnection())
        //return;

    //accelgyro.CalibrateAccel(10);
    //accelgyro.CalibrateGyro(10);

    //Serial.println("1000 Total Readings");
    //accelgyro.PrintActiveOffsets();
    //// Serial.println("\n\n Any of the above offsets will work nice \n\n");
    //// Serial.println("\n\n Any of the above offsets will work nice \n\n Lets proof the PID tuning using another method:");

    //accelgyro.setFullScaleAccelRange(2);
}

void read_IMU(void)
{
    // if(accelgyro.testConnection())
    // accelgyro.getMotion6(&imu_ax, &imu_ay, &imu_az,
    //&imu_gx, &imu_gy, &imu_gz);

    return;
}

void read_barometer(void)
{

}

void read_gps(void)
{
    while(Serial1.available())
        gps.encode(Serial1.read());
}

void flash_log_sensors(void)
{
    log(NULL, 0, SENSOR_READING);
}

void logger(void)
{
    command_t command_rep;
    command_rep.cmd = CMD_STATUS;
    command_rep.id = FILL_STATION_ID;

    command_rep.size = 2;
    int16_t ipressure = (int16_t)(Tank_Top_Module.pressure * 100);
    command_rep.data[0] = (ipressure >> 8) & 0xff;
    command_rep.data[1] = (ipressure) & 0xff;

    command_rep.crc = crc((unsigned char *)&command_rep, command_rep.size + 3);

    write_command(&command_rep, DEFAULT_LOG_INFERFACE);
}

void read_temperature_tank_top(void)
{
    if(fast_reboot) return;

    float temp = Tank_Top_Module.thermocouple.getThermocoupleTemp();
    Tank_Top_Module.temperature = (int16_t)(temp * 10.0);

}

void read_temperature_tank_bot(void)
{
    if(fast_reboot) return;

    float temp = Tank_Bot_Module.thermocouple.getThermocoupleTemp();
    Tank_Bot_Module.temperature = (int16_t)(temp * 10.0);
}


//---------TIMERS---------------
void reset_timers(void)
{
    arm_reset_timer = 0;
}

void timer_tick(uint16_t *timer) { (*timer)++; }

void arm_timer_tick(void) { timer_tick(&arm_reset_timer); }
