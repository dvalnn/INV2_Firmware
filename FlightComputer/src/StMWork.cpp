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
#include <Max6675.h>
#include <ADS1115_WE.h>
#include <Crc.h>

//#define KALMAN_DEBBUG
float imu_ax;
float imu_ay;
float imu_az;

float imu_gx;
float imu_gy;
float imu_gz;

float imu_mx;
float imu_my;
float imu_mz;

float altitude = 0;
float maxAltitude = 0;

float ground_hPa = 0;

alt_kalman alt_kal;
QuaternionFilter att;
Eigen::Matrix<float, 9, 1> alt_kalman_state; // altitude | vertical velocity | vertical acceleration
float q[4];

int16_t tank_pressure = 0;
float tank_liquid = 0;

uint16_t arm_reset_timer;
uint16_t burn_timer;

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
    ground_hPa = pressure;

    preferences.putFloat("ground_hpa", ground_hPa);
}

void imu_calibrate(void)
{
    IMU.calibrateAccelGyro();

    double ax = 0;
    double ay = 0;
    double az = 0;

    for(int i = 0; i < 1000; i++) //10 seconds of reading imu
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
}


void read_imu(void)
{
    // if(accelgyro.testConnection())
    // accelgyro.getMotion6(&imu_ax, &imu_ay, &imu_az,
    //&imu_gx, &imu_gy, &imu_gz);

    IMU.update_accel_gyro();

    imu_ax = IMU.getAccX() * 9.8;
    imu_ay = IMU.getAccY() * 9.8;
    imu_az = IMU.getAccZ() * 9.8;

    imu_gx = IMU.getGyroX();
    imu_gy = IMU.getGyroY();
    imu_gz = IMU.getGyroZ();

    //Serial.printf("%f %f %f %f %f %f\n", imu_ax, imu_ay, imu_az, imu_gx, imu_gy, imu_gz);

    return;
}

void read_barometer(void)
{
    static float lpf_alt = 0.0f;
    // LOW PASS altitude
    lpf_alt = bmp.readAltitude(ground_hPa);
    altitude += (lpf_alt - altitude) * betha_alt;

    //Serial.printf("Altitude barometer %f\n", altitude);
}

void read_gps(void)
{
    bool reading = false;
    while (Serial1.available() && reading == false)
        reading = gps.encode(Serial1.read());

    if (reading)
    {
        // Serial.printf("N satalites %d\n", gps.satellites.value());
        // Serial.printf("Lat %f Lon %f\n", gps.location.lat(), gps.location.lng());
        // Serial.printf("GPS altitude %f\n", gps.altitude.meters());
    }
}

void kalman(void)
{
    const Vector3f norm_g(0.0, 0.0, 1.0);
    static float angles[3];

    static MyQuaternion Q;
    static Matrix<float, 7, 1> Z;
    static Matrix<float, 3, 1> U, Acc;
    static Matrix<float, 9, 1> Z_2, U_2;
    static Vector3f norm_acc, axis, acc;
    static float last_alt = altitude;
    static float gps_alt_offset, alt_offset, last_lat, last_long;
    static bool first = true;

    static unsigned int start;

    if (first)
    {
        acc << imu_ax, imu_ay, imu_az;
        norm_acc = acc / acc.norm();
        axis = norm_acc.cross(norm_g);
        axis = axis / axis.norm();

        float t = iacos(norm_acc.dot(norm_g));

        q[0] = icos(t / 2.0);
        q[1] = axis(0) * isin(t / 2);
        q[2] = axis(1) * isin(t / 2);
        q[3] = axis(2) * isin(t / 2);
        first = false;
        Z_2 << 0, 0, 0, 0, 0, 0, 0, 0, 0;
        
        start = millis();
    }

    att.update(imu_ax, imu_ay, imu_az, imu_gx, imu_gy, imu_gz, 0, 0, 0, q);

    Q.qw = q[0];
    Q.qx = q[1];
    Q.qy = q[2];
    Q.qz = q[3];

#ifdef KALMAN_DEBBUG
    Serial.println("Orientation done");
    Serial.print(" |qw:");
    Serial.print(Q.qw);
    Serial.print(" |qx:");
    Serial.print(Q.qx);
    Serial.print(" |qy:");
    Serial.print(Q.qy);
    Serial.print(" |qz:");
    Serial.print(Q.qz);
    Serial.println("Starting altitude");
#endif
    Acc << imu_ax, imu_ay, imu_az;

    U_2 << 0, 0, Acc(0), 0, 0, Acc(1), 0, 0, Acc(2);
    Z_2 << 0, 0, Acc(0), 0, 0, Acc(1), altitude, 0, Acc(2); 

    last_alt = altitude;
#ifdef KALMAN_DEBBUG
    Serial.println("measurements done");
    Serial.println("Starting kalman");
#endif
    alt_kalman_state = alt_kal.cicle(Q, Z_2, U_2);

    if (Launch)
        maxAltitude = max(maxAltitude, alt_kalman_state(6));

    //Serial.print(" |pos_X:");
    //Serial.print(alt_kalman_state(0));
    //Serial.print(" |vel_X:");
    //Serial.print(alt_kalman_state(1));
    //Serial.print(" |Acc_X:");
    //Serial.print(alt_kalman_state(2));
    //Serial.print(" |pos_Y:");
    //Serial.print(alt_kalman_state(3));
    //Serial.print(" |vel_Y:");
    //Serial.print(alt_kalman_state(4));
    //Serial.print(" |Acc_Y:");
    //Serial.print(alt_kalman_state(5));

    //Serial.print(" |altitude:");
    //Serial.print(alt_kalman_state(6));
    //Serial.print(" |vel_Z:");
    //Serial.print(alt_kalman_state(7));
    //Serial.print(" |Acc_Z:");
    //Serial.print(alt_kalman_state(8));
    //Serial.println();
    //Serial.flush();

#ifdef KALMAN_DEBBUG
    Serial.println("altitude done");
#endif
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
    float gps_lat = gps.location.lat();
    float gps_lon = gps.location.lng();
    uint16_t gps_altitude = (uint16_t)(gps.altitude.meters());

    command.data[index++] = (uint8_t)(gps.satellites.value());

    command.data[index++] = (uint8_t)((gps_altitude >> 8) & 0xff);
    command.data[index++] = (uint8_t)((gps_altitude) & 0xff);

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

    uint16_t horizontal_vel = (gps.speed.kmph() * 10);
    command.data[index++] = (uint8_t)((horizontal_vel >> 8) & 0xff);
    command.data[index++] = (uint8_t)((horizontal_vel) & 0xff);

//---------------kalman

    uint16_t u_z = alt_kalman_state(6) * 10;
    uint16_t u_vz = alt_kalman_state(7) * 10;
    uint16_t u_az = alt_kalman_state(8) * 10;

    command.data[index++] = (uint8_t)((u_z >> 8) & 0xff);
    command.data[index++] = (uint8_t)((u_z) & 0xff);

    command.data[index++] = (uint8_t)((u_vz >> 8) & 0xff);
    command.data[index++] = (uint8_t)((u_vz) & 0xff);

    command.data[index++] = (uint8_t)((u_az >> 8) & 0xff);
    command.data[index++] = (uint8_t)((u_az) & 0xff);

    // convert quaternion [0:1] to uint16 [0, 2^16 - 1]
    uint16_t u_quat1 = q[0] * (0xFFFF);
    uint16_t u_quat2 = q[1] * (0xFFFF);
    uint16_t u_quat3 = q[2] * (0xFFFF);
    uint16_t u_quat4 = q[3] * (0xFFFF);

    command.data[index++] = (uint8_t)((u_quat1 >> 8) & 0xff);
    command.data[index++] = (uint8_t)((u_quat1) & 0xff);

    command.data[index++] = (uint8_t)((u_quat2 >> 8) & 0xff);
    command.data[index++] = (uint8_t)((u_quat2) & 0xff);

    command.data[index++] = (uint8_t)((u_quat3 >> 8) & 0xff);
    command.data[index++] = (uint8_t)((u_quat3) & 0xff);

    command.data[index++] = (uint8_t)((u_quat4 >> 8) & 0xff);
    command.data[index++] = (uint8_t)((u_quat4) & 0xff);

//-------------- ematchs

    command.data[index++] = ((ematch_drag_reading >> 8) & 0xff);
    command.data[index++] = ((ematch_drag_reading) & 0xff);

    command.data[index++] = ((ematch_main_reading >> 8) & 0xff);
    command.data[index++] = ((ematch_main_reading) & 0xff);

    command.size = index; 
    command.crc = crc((unsigned char *)&command, command.size + 3);
    write_command(&command, DEFAULT_CMD_INTERFACE);
}

void read_temperature_tank_top(void)
{
    if (fast_reboot)
        return;

    float temp = Tank_Top_Module.thermocouple.getThermocoupleTemp();
    Tank_Top_Module.temperature = (int16_t)(temp * 10.0);
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
    digitalWrite(MAIN_CHUTE_READ, LOW);

    ematch_main_reading = analogRead(MAIN_CHUTE_READ);
}

void read_drag_ematch(void)
{
    // should already be low
    digitalWrite(DRAG_CHUTE_READ, LOW);

    ematch_drag_reading = analogRead(DRAG_CHUTE_READ);
}

//---------TIMERS---------------
void reset_timers(void)
{
    arm_reset_timer = 0;
    burn_timer = 0;
}

void timer_tick(uint16_t *timer) { (*timer)++; }

void arm_timer_tick(void) { timer_tick(&arm_reset_timer); }
void burn_timer_tick(void) { timer_tick(&burn_timer); }
