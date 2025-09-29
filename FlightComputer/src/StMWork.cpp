#include <Arduino.h>

#include <time.h>

#include "HardwareCfg.h"
#include "StMWork.h"
#include "StMComms.h"
#include "GlobalVars.h"
#include "Comms.h"
#include "FlashLog.h"
#include "kalman.h"
#include "quaternion.h"

#include <I2Cdev.h>
#include <MPU6050.h>
#include <HX711.h>
#include <MAX6675.h>
#include <ADS1115_WE.h>
#include <Crc.h>

//#define KALMAN_DEBBUG

int16_t tank_pressure = 0;
float tank_liquid = 0;

uint16_t arm_reset_timer;
uint16_t burn_timer;
uint16_t depressur_timer;
uint16_t depressur_global_timer;

float ttp_values[press_values_size], tbp_values[press_values_size], chp_values[press_values_size];
int ttp_index = 0, tbp_index = 0, chp_index = 0;

uint16_t ematch_main_reading = 0, ematch_drag_reading = 0;

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

void main_ematch_high(void) { 
    digitalWrite(MAIN_CHUTE_DEPLOY_PIN, HIGH); 
}

void main_ematch_low(void) { 
    digitalWrite(MAIN_CHUTE_DEPLOY_PIN, LOW); 
}

void drag_ematch_high(void) { 
    digitalWrite(DRAG_CHUTE_DEPLOY_PIN, HIGH); 
}

void drag_ematch_low(void) { 
    digitalWrite(DRAG_CHUTE_DEPLOY_PIN, LOW); 
}

void barometer_calibrate(void)
{
    // read barometer every 10ms for 5 seconds to avg the ground pha
    float pressure = 0;
    for (int i = 0; i < 500; i++)
    {
        pressure += bmp.readPressure();
        delay(10);
    }

    pressure /= 500;
    pressure /= 100;
    ground_hPa = pressure;
}

void imu_calibrate(void)
{
    IMU.calibrateAccelGyro();

    double ax = 0;
    double ay = 0;
    double az = 0;

    for(int i = 0; i < 500; i++) //5 seconds of reading imu
    {
        read_imu();
        ax += imu_ax;
        ay += imu_ay;
        az += imu_az;
        delay(10);
    }

    ax /= 1000;
    ay /= 1000;
    az /= 1000;

    Serial.printf("Avg imu:\nax %f\nay %f\naz %f\n", ax, ay, az);
}

void kalman_calibrate(void)
{
    //reset kalman position
    alt_kal.update_offset(-kalman_altitude, -kalman_velocity, -kalman_accel);
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
    
    //int16_t ipressure = (int16_t)(Tank_Top_Module.temperature * 10);
    command_rep.data[0] = (ipressure >> 8) & 0xff;
    command_rep.data[1] = (ipressure) & 0xff;

    command_rep.crc = crc((unsigned char *)&command_rep, command_rep.size + 3);

    write_command(&command_rep, DEFAULT_LOG_INFERFACE);
}

void telemetry(void)
{
    command_t command;
    uint16_t ask_bits = (ROCKET_STATE_BIT | ROCKET_PRESSURE_BIT | ROCKET_GPS_BIT | ROCKET_KALMAN_BIT | ROCKET_CHUTE_EMATCH_BIT);
    uint16_t index = 0;

    // union used to get bit representation of float
    union ufloat
    {
        float f;
        uint32_t i;
    };
    union ufloat f;

    command.cmd = CMD_LOG;
    command.id = GROUND_ID;
    command.data[index++] = ((ask_bits >> 8) & 0xff);
    command.data[index++] = ((ask_bits) & 0xff);

//-----------flags
    command.data[index++] = state;
    command.data[index++] = (uint8_t)((log_running << 7) |
                                      (Tank_Top_Module.valve_state << 6) |
                                      (Tank_Bot_Module.valve_state << 5) |
                                      (Chamber_Module.valve_state << 4) |
                                      (DragDeployed << 3) |
                                      (MainDeployed << 2));

//------------pressure
    int16_t ipressure;
    ipressure = (int16_t)(Tank_Top_Module.pressure * 100);
    command.data[index++] = (ipressure >> 8) & 0xff;
    command.data[index++] = (ipressure) & 0xff;

    ipressure = (int16_t)(Tank_Bot_Module.pressure * 100);
    command.data[index++] = (ipressure >> 8) & 0xff;
    command.data[index++] = (ipressure) & 0xff;

    ipressure = (int16_t)(Chamber_Module.pressure * 100);
    command.data[index++] = (ipressure >> 8) & 0xff;
    command.data[index++] = (ipressure) & 0xff;
//------------gps

    
    uint16_t gps_alt = (uint16_t)(gps_altitude);

    command.data[index++] = (uint8_t)(gps_satalites);

    command.data[index++] = (uint8_t)((gps_alt >> 8) & 0xff);
    command.data[index++] = (uint8_t)((gps_alt) & 0xff);

    f.f = gps_lat;
    command.data[index++] = (uint8_t)((f.i >> 24) & 0xff);
    command.data[index++] = (uint8_t)((f.i >> 16) & 0xff);
    command.data[index++] = (uint8_t)((f.i >> 8) & 0xff);
    command.data[index++] = (uint8_t)((f.i) & 0xff);

    f.f = gps_lon;
    command.data[index++] = (uint8_t)((f.i >> 24) & 0xff);
    command.data[index++] = (uint8_t)((f.i >> 16) & 0xff);
    command.data[index++] = (uint8_t)((f.i >> 8) & 0xff);
    command.data[index++] = (uint8_t)((f.i) & 0xff);

    uint16_t horizontal_vel = (gps_horizontal_vel * 10);
    command.data[index++] = (uint8_t)((horizontal_vel >> 8) & 0xff);
    command.data[index++] = (uint8_t)((horizontal_vel) & 0xff);
//---------------kalman

    uint16_t u_z = kalman_altitude * 10;
    uint16_t u_z_max = maxAltitude;
    uint16_t u_vz = kalman_velocity * 10;
    uint16_t u_az = kalman_accel * 10;

    command.data[index++] = (uint8_t)((u_z >> 8) & 0xff);
    command.data[index++] = (uint8_t)((u_z) & 0xff);

    command.data[index++] = (uint8_t)((u_z_max >> 8) & 0xff);
    command.data[index++] = (uint8_t)((u_z_max) & 0xff);

    command.data[index++] = (uint8_t)((u_vz >> 8) & 0xff);
    command.data[index++] = (uint8_t)((u_vz) & 0xff);

    command.data[index++] = (uint8_t)((u_az >> 8) & 0xff);
    command.data[index++] = (uint8_t)((u_az) & 0xff);

    // convert quaternion [0:1] to uint16 [0, 2^16 - 1]
    uint16_t u_quat1 = kalman_q[0] * (0xFFFF);
    uint16_t u_quat2 = kalman_q[1] * (0xFFFF);
    uint16_t u_quat3 = kalman_q[2] * (0xFFFF);
    uint16_t u_quat4 = kalman_q[3] * (0xFFFF);

    command.data[index++] = (uint8_t)((u_quat1 >> 8) & 0xff);
    command.data[index++] = (uint8_t)((u_quat1) & 0xff);

    command.data[index++] = (uint8_t)((u_quat2 >> 8) & 0xff);
    command.data[index++] = (uint8_t)((u_quat2) & 0xff);

    command.data[index++] = (uint8_t)((u_quat3 >> 8) & 0xff);
    command.data[index++] = (uint8_t)((u_quat3) & 0xff);

    command.data[index++] = (uint8_t)((u_quat4 >> 8) & 0xff);
    command.data[index++] = (uint8_t)((u_quat4) & 0xff);
//-------------- ematchs

    command.data[index++] = ((ematch_main_reading >> 8) & 0xff);
    command.data[index++] = ((ematch_main_reading) & 0xff);

    command.data[index++] = ((ematch_drag_reading >> 8) & 0xff);
    command.data[index++] = ((ematch_drag_reading) & 0xff);

    command.size = index; 
    command.crc = crc((unsigned char *)&command, command.size + 3);
    write_command(&command, DEFAULT_CMD_INTERFACE);

    Serial.printf("telemetry\n");
}

void read_temperature_tank_top(void)
{
    if (fast_reboot)
        return;

    float temp = Tank_Top_Module.thermocouple.getThermocoupleTemp();
    Tank_Top_Module.temperature = (int16_t)(temp * 10.0);

    //TODO remove, just for debugging
    //Tank_Top_Module.pressure = (Tank_Top_Module.temperature * 1.0) / 10.0;
}

void read_temperature_tank_bot(void)
{
    if (fast_reboot)
        return;

    float temp = Tank_Bot_Module.thermocouple.getThermocoupleTemp();
    Tank_Bot_Module.temperature = (int16_t)(temp * 10.0);
}


void read_main_ematch(void)
{
    // should already be low
    digitalWrite(MAIN_CHUTE_DEPLOY_PIN, LOW);

    ematch_main_reading = analogRead(MAIN_CHUTE_READ);
}

void read_drag_ematch(void)
{
    // should already be low
    digitalWrite(DRAG_CHUTE_DEPLOY_PIN, LOW);

    ematch_drag_reading = analogRead(DRAG_CHUTE_READ);
}

void read_gps(void)
{
    bool reading = false;
    bool first = true;
    while (Serial1.available() && reading == false)
        reading = gps.encode(Serial1.read());

    if (reading)
    {
        gps_lat = gps.location.lat();
        gps_lon = gps.location.lng();
        gps_altitude = gps.altitude.meters();
        gps_horizontal_vel = gps.speed.kmph();
        gps_satalites = gps.satellites.value();
    }
}


void recover_now(void)
{
    V_Purge_open();
    drag_ematch_high();
}

//---------TIMERS---------------
void reset_timers(void)
{
    arm_reset_timer = 0;
    burn_timer = 0;
    depressur_timer = 0;
    depressur_global_timer = 0;
}

void timer_tick(uint16_t *timer) { (*timer)++; }

void arm_timer_tick(void) { timer_tick(&arm_reset_timer); }
void burn_timer_tick(void) { timer_tick(&burn_timer); }
void depressur_timer_tick(void) { timer_tick(&depressur_timer); }
void depressur_global_timer_tick(void) { timer_tick(&depressur_global_timer); }
