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

int16_t imu_ax;
int16_t imu_ay;
int16_t imu_az;

int16_t imu_gx;
int16_t imu_gy;
int16_t imu_gz;

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
    if (!accelgyro.testConnection())
        return;

    accelgyro.CalibrateAccel(10);
    accelgyro.CalibrateGyro(10);

    Serial.println("1000 Total Readings");
    accelgyro.PrintActiveOffsets();
    // Serial.println("\n\n Any of the above offsets will work nice \n\n");
    // Serial.println("\n\n Any of the above offsets will work nice \n\n Lets proof the PID tuning using another method:");

    accelgyro.setFullScaleAccelRange(2);
}

void read_IMU(void)
{
    // if(accelgyro.testConnection())
    // accelgyro.getMotion6(&imu_ax, &imu_ay, &imu_az,
    //&imu_gx, &imu_gy, &imu_gz);
    return;
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

    // command_rep.size = 100; //test

    // tank_temp1 = chamber_temp3;
    // tank_temp2 = chamber_temp4;

    command_rep.size = 36;
    command_rep.data[0] = state;
    command_rep.data[1] = (Tank_Top_Module.temperature >> 8) & 0xff;
    command_rep.data[2] = (Tank_Top_Module.temperature) & 0xff;
    command_rep.data[3] = (Tank_Bot_Module.temperature >> 8) & 0xff;
    command_rep.data[4] = (Tank_Bot_Module.temperature) & 0xff;

    int16_t ipressure = (int16_t)(Tank_Top_Module.pressure * 100);
    command_rep.data[11] = (ipressure >> 8) & 0xff;
    command_rep.data[12] = (ipressure) & 0xff;

    ipressure = (int16_t)(Tank_Bot_Module.pressure * 100);
    command_rep.data[13] = (ipressure >> 8) & 0xff;
    command_rep.data[14] = (ipressure) & 0xff;

    command_rep.data[15] = (tank_pressure >> 8) & 0xff;
    command_rep.data[16] = (tank_pressure) & 0xff;

    int16_t itank_liquid = (int16_t)(tank_liquid * 10000);
    command_rep.data[17] = (itank_liquid >> 8) & 0xff;
    command_rep.data[18] = (itank_liquid) & 0xff;

    command_rep.data[19] = (uint8_t)((log_running << 7) |
                                     (Tank_Top_Module.valve_state << 6) |
                                     (Tank_Bot_Module.valve_state << 5));


    ipressure = (int16_t)(Chamber_Module.pressure * 100);
    command_rep.data[26] = (ipressure >> 8) & 0xff;
    command_rep.data[27] = (ipressure) & 0xff;

    int16_t he_moles_i = (int16_t)(he_mol * 10);
    command_rep.data[28] = (he_moles_i >> 8) & 0xff;
    command_rep.data[29] = (he_moles_i) & 0xff;

    int16_t tank_mol_lost_i = (int16_t)(tank_mol_lost * 10);
    command_rep.data[30] = (tank_mol_lost_i >> 8) & 0xff;
    command_rep.data[31] = (tank_mol_lost_i) & 0xff;

    int16_t hL_i = (int16_t)(he_mol * 10);
    command_rep.data[32] = (hL_i >> 8) & 0xff;
    command_rep.data[33] = (hL_i) & 0xff;

    int16_t ml_i = (int16_t)(tank_mol_lost * 10);
    command_rep.data[34] = (ml_i >> 8) & 0xff;
    command_rep.data[35] = (ml_i) & 0xff;

    command_rep.crc = crc((unsigned char *)&command_rep, command_rep.size + 3);

    write_command(&command_rep, DEFAULT_LOG_INFERFACE);
}

void read_temperature_tank_top(void)
{
    if(fast_reboot) return;

    // digitalWrite(TEMP_AMP3_SS_PIN, HIGH);

    float temp = Tank_Top_Module.thermocouple.getThermocoupleTemp();
    Tank_Top_Module.temperature = (int16_t)(temp * 10.0);

    // digitalWrite(TEMP_AMP3_SS_PIN, LOW);
}

void read_temperature_tank_bot(void)
{
    if(fast_reboot) return;

    // digitalWrite(TEMP_AMP3_SS_PIN, HIGH);
    float temp = Tank_Bot_Module.thermocouple.getThermocoupleTemp();
    Tank_Bot_Module.temperature = (int16_t)(temp * 10.0);
    // digitalWrite(TEMP_AMP3_SS_PIN, LOW);
}

// constans
// g - 9.8
// Dt - 1/2 inch 0.5 (calcular SI, m)
// Do - diametro tanke oxi (m)
// h1 - diferença de altura entre transdutor e fundo do tanque (m)
// h_tanque - altura tanque (m)

// P1(bot) P2(top) are in kpah (bar * 100)
// T4 (bot) T5(top) in kelvin

// x is the answer?

float g = 9.8;
float Dt = 0.152f;
float Do = 0.0127f;
float h1 = 0.05f;
float h_tanque = 1.07f;
float V_total = 0.022; // 22l

double mVL = 0;
double mVG; // massa volumica Liquido(bot) e Gas(Top)

double hL;     // altura liquido
double Vl, Vg; // volume liquido / gas
double x;      // answer
double ml, mg;

static void MassaVolumica(void)
{

    float T4 = (Tank_Bot_Module.temperature / 10.0f) + 273.5;
    float T5 = (Tank_Top_Module.temperature / 10.0f) + 273.5;

    mVL = -0.0258 * T4 * T4 + 8.2291 * T4 + 572.66;
    mVG = 0.0059 * exp(0.0352 * T5);
}

static void alturafluido(void)
{
    float P1 = Tank_Bot_Module.pressure * 100000;
    float P2 = Tank_Top_Module.pressure * 100000;

    // put your main code here, to run repeatedly:
    hL = (P1 - P2 - mVG * g * h_tanque) / ((mVL - mVG) * g);

    if (hL < h1)
    {                                       // h1 valor conhecido distancia do transdutor de pressao ao fundo do tanque em metros
        Vl = ((3.1416 * Do * Do) / 4) * hL; // Dt- Diametro do tubo
        Vg = V_total - Vl;                  // V_total = constante a determinar
        ml = mVL * Vl;
        mg = mVG * Vg;
        tank_liquid = mg / (mg + ml);
    }
    if (hL > h1)
    {
        Vl = ((3.1416 * Do * Do) / 4) * h1 + ((3.1416 * Dt * Dt) / 4) * (hL - h1); // Do - diametro do tanque oxidante Dt- Diâmetro do tubo                                         //h1 - diferença de altura entre transdutor e fundo do tanque
        Vg = V_total - Vl;
        ml = mVL * Vl;
        mg = mVG * Vg;
        tank_liquid = mg / (mg + ml);
    }
}

void calc_liquid(void)
{
    // calculate amount of liquid in the tank
    Tank_Top_Module.pressure = 0;
    Tank_Bot_Module.pressure = 0;

    for (float val : ttp_values)
    {
        Tank_Top_Module.pressure += val;
    }
    for (float val : tbp_values)
    {
        Tank_Bot_Module.pressure += val;
    }
    
    Tank_Top_Module.pressure /= press_values_size;
    Tank_Bot_Module.pressure /= press_values_size;

    MassaVolumica();
    alturafluido();

    // only for debug
    tank_pressure = (int16_t)(Tank_Top_Module.pressure * 100);
    // tank_liquid = Tank_Top_Module.temperature;

    return;
}

//---------TIMERS---------------
void reset_timers(void)
{
    arm_reset_timer = 0;
}

void timer_tick(uint16_t *timer) { (*timer)++; }

void arm_timer_tick(void) { timer_tick(&arm_reset_timer); }
