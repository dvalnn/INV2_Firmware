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

void toggle_led_1000ms(void)
{
    static clock_t begin = 0, end = 0;
    static int led_status = 0;
    end = millis();
    
    int msec = (end - begin);
    if(msec > 1000)
    {
        begin = clock();
        //togle led
        led_status ^= 1;
        digitalWrite(LED_PIN, led_status);
    }
}

void toggle_led_500ms(void)
{
    static clock_t begin = 0, end = 0;
    static int led_status = 0;
    end = millis();
    
    int msec = (end - begin);
    if(msec > 500)
    {
        begin = clock();
        //togle led
        led_status ^= 1;
        digitalWrite(LED_PIN, led_status);
    }
}

void toggle_led_200ms(void)
{
    static clock_t begin = 0, end = 0;
    static int led_status = 0;
    end = millis();
    
    int msec = (end - begin);
    if(msec > 200)
    {
        begin = clock();
        //togle led
        led_status ^= 1;
        digitalWrite(LED_PIN, led_status);
    }
}


void imu_pid_calibration(void)
{
    Serial.println(accelgyro.testConnection() ? "MPu6050 connection successful" : "MPu6050 connection failed");
    Serial.println("PID tuning Each Dot = 100 readings");


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
    accelgyro.CalibrateAccel(10);
    accelgyro.CalibrateGyro(10);

    Serial.println("1000 Total Readings");
    accelgyro.PrintActiveOffsets();
    //Serial.println("\n\n Any of the above offsets will work nice \n\n"); 
    //Serial.println("\n\n Any of the above offsets will work nice \n\n Lets proof the PID tuning using another method:"); 
    
    accelgyro.setFullScaleAccelRange(2);

}

void read_IMU(void) 
{
    if(accelgyro.testConnection())
        accelgyro.getMotion6(&imu_ax, &imu_ay, &imu_az,
                             &imu_gx, &imu_gy, &imu_gz);
    return; 
}