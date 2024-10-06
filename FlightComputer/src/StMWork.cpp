#include <Arduino.h>

#include <time.h>

#include "HardwareCfg.h"
#include "StMWork.h"
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
Eigen::Matrix<float, 9,1> alt_kalman_state; //altitude | vertical velocity | vertical acceleration

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

void barometer_calibrate(void)
{
    //read barometer every 10ms for 5 seconds to avg the ground pha
    float pressure = 0;
    for(int i = 0; i < 500; i++)
    {
        pressure += bmp.readPressure();
        delay(10);
    }

    pressure /= 500;
    ground_hPa = pressure;
}

void imu_calibrate(void)
{

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

void read_imu(void)
{
    // if(accelgyro.testConnection())
    // accelgyro.getMotion6(&imu_ax, &imu_ay, &imu_az,
    //&imu_gx, &imu_gy, &imu_gz);
    
    IMU.update_accel_gyro();

    imu_ax = IMU.getAccX();
    imu_ay = IMU.getAccY();
    imu_az = IMU.getAccZ();

    imu_gx = IMU.getGyroX();
    imu_gy = IMU.getGyroY();
    imu_gz = IMU.getGyroZ();

    Serial.printf("%f %f %f %f %f %f\n", imu_ax, imu_ay, imu_az, imu_gx, imu_gy, imu_gz);

    return;
}

void read_barometer(void)
{
    static float lpf_alt = 0.0f;
    //LOW PASS altitude
    lpf_alt = bmp.readAltitude(ground_hPa);
    altitude += (lpf_alt - altitude) * betha_alt;


    Serial.printf("Altitude barometer %f\n", altitude);
}

void read_gps(void)
{
    bool reading = false;
    while(Serial1.available() && reading == false)
        reading = gps.encode(Serial1.read());

    if(reading)
    {
        //Serial.printf("N satalites %d\n", gps.satellites.value());
        //Serial.printf("Lat %f Lon %f\n", gps.location.lat(), gps.location.lng());
        //Serial.printf("GPS altitude %f\n", gps.altitude.meters());
    }
}

void kalman(void)
{
    const Vector3f norm_g( 0.0, 0.0, 1.0); 
    static float angles[3];

    static MyQuaternion Q;
    static Matrix<float,7,1> Z;
    static Matrix<float,3,1> U,Acc;
    static Matrix<float,9,1> Z_2,U_2;
    static Vector3f norm_acc,axis,acc;
    static float q[4],last_alt = altitude;
    static float gps_alt_offset,alt_offset,last_lat, last_long;
    static bool first = true;

    static unsigned int start;

    if(first){
      acc << imu_ax, imu_ay, imu_az;
      norm_acc = acc/acc.norm();
      axis = norm_acc.cross(norm_g);
      axis = axis/axis.norm();

      float t = iacos(norm_acc.dot(norm_g));

      q[0]= icos(t/2.0);
      q[1]= axis(0)*isin(t/2);
      q[2]= axis(1)*isin(t/2);
      q[3]= axis(2)*isin(t/2);
      first = false;
      Z_2 << 0, 0, 0, 0, 0, 0, 0, 0, 0;
      last_lat = gps.location.lat();
      last_long = gps.location.lng();
      gps_alt_offset = gps.altitude.meters();
      alt_offset = altitude;
      start = millis();
    }
    
    /* acc << ax, ay, az;
    norm_acc = acc/acc.norm();
    axis = norm_acc.cross(norm_g);
    axis = axis/axis.norm();
    float t = iacos(norm_acc.dot(norm_g));
    Z << icos(t/2.0), axis(0)*isin(t/2), axis(1)*isin(t/2), axis(2)*isin(t/2), gx, gy, gz;
    U << gx, gy, gz;
    
    orientation_state = attitude.cicle(Z,U); */
    att.update(imu_ax,imu_ay,imu_az,imu_gx,imu_gy,imu_gz,0,0,0,q);

    Q.qw = q[0];
    Q.qx = q[1];
    Q.qy = q[2];
    Q.qz = q[3];

    /* Q.qw = orientation_state(0);
    Q.qx = orientation_state(1);
    Q.qy = orientation_state(2);
    Q.qz = orientation_state(3); */

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
    quaternion_to_euler(Q,angles);
    Acc << imu_ax, imu_ay, imu_az;
    //Acc = quaternion_to_rotation_matrix(Q)*Acc;

    U_2 << 0, 0, Acc(0), 0, 0, Acc(1), 0, 0, Acc(2);
    Z_2 << (last_lat-gps.location.lat())*111000.0, gps.speed.mps()*icos(angles[2]), Acc(0), (last_long-gps.location.lng())*111000.0, gps.speed.mps()*isin(angles[2]), Acc(1), altitude, 0, Acc(2); 

    last_alt=altitude;
    #ifdef KALMAN_DEBBUG
        Serial.println("measurements done");
        Serial.println("Starting kalman");
    #endif
    alt_kalman_state = alt_kal.cicle(Q,Z_2,U_2);

    if(Launch)
      maxAltitude=max(maxAltitude,alt_kalman_state(6)); 

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

void main_ematch_high(void) { digitalWrite(MAIN_CHUTE_DEPLOY_PIN, HIGH); }

void main_ematch_low(void) { digitalWrite(MAIN_CHUTE_DEPLOY_PIN, LOW); }

void drag_ematch_high(void) { digitalWrite(DRAG_CHUTE_DEPLOY_PIN, HIGH); }

void drag_ematch_low(void) { digitalWrite(DRAG_CHUTE_DEPLOY_PIN, LOW); }

void read_main_ematch(void)
{
    //should already be low 
    digitalWrite(MAIN_CHUTE_READ, LOW);
    
    ematch_main_reading = analogRead(MAIN_CHUTE_READ);
}

void read_drag_ematch(void)
{
    //should already be low 
    digitalWrite(DRAG_CHUTE_READ, LOW);
    
    ematch_drag_reading = analogRead(DRAG_CHUTE_READ);
}

//---------TIMERS---------------
void reset_timers(void)
{
    arm_reset_timer = 0;
}

void timer_tick(uint16_t *timer) { (*timer)++; }

void arm_timer_tick(void) { timer_tick(&arm_reset_timer); }
void burn_timer_tick(void) { timer_tick(&burn_timer); }
