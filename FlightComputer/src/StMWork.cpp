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

int32_t weight1;
int32_t weight2;

int16_t tank_pressure = 0;
float tank_liquid = 0;
float tank_liquid2 = 0;

int16_t tank_t1;
int16_t tank_t2;
int16_t tank_t3;
int16_t tank_t4;
int16_t tank_t5;
uint8_t tank_tactile_bits;

uint16_t arm_reset_timer;

void V_Vpu_close(void)
{
    digitalWrite(Tank_Top_Module.valve_pin, 0);
    Tank_Top_Module.valve_state = 0;
}
void V_Engine_close(void)
{
    digitalWrite(Tank_Bot_Module.valve_pin, 0);
    Tank_Bot_Module.valve_state = 0;
}

void V_Vpu_open(void)
{
    digitalWrite(Tank_Top_Module.valve_pin, 1);
    Tank_Top_Module.valve_state = 1;
}
void V_Engine_open(void)
{
    digitalWrite(Tank_Bot_Module.valve_pin, 1);
    Tank_Bot_Module.valve_state = 1;
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

    command_rep.data[5] = (Chamber_Module.temperature1 >> 8) & 0xff;
    command_rep.data[6] = (Chamber_Module.temperature1) & 0xff;
    command_rep.data[7] = (Chamber_Module.temperature2 >> 8) & 0xff;
    command_rep.data[8] = (Chamber_Module.temperature2) & 0xff;
    command_rep.data[9] = (Chamber_Module.temperature3 >> 8) & 0xff;
    command_rep.data[10] = (Chamber_Module.temperature3) & 0xff;

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
                                     (Tank_Bot_Module.valve_state << 5) |
                                     tank_tactile_bits);

    command_rep.data[20] = (Scale_Module.weight1 >> 8) & 0xff;
    command_rep.data[21] = (Scale_Module.weight1) & 0xff;

    command_rep.data[22] = (Scale_Module.weight2 >> 8) & 0xff;
    command_rep.data[23] = (Scale_Module.weight2) & 0xff;

    command_rep.data[24] = (Scale_Module.weight3 >> 8) & 0xff;
    command_rep.data[25] = (Scale_Module.weight3) & 0xff;

    ipressure = (int16_t)(Chamber_Module.pressure * 100);
    command_rep.data[26] = (ipressure >> 8) & 0xff;
    command_rep.data[27] = (ipressure) & 0xff;

    int16_t ialtura = (int16_t)(hL * 100);
    command_rep.data[28] = (ialtura >> 8) & 0xff;
    command_rep.data[29] = (ialtura) & 0xff;

    int16_t iVl = (int16_t)(Vl * 1000);
    command_rep.data[30] = (iVl >> 8) & 0xff;
    command_rep.data[31] = (iVl) & 0xff;

    int16_t iml = (int16_t)(ml * 100);
    command_rep.data[32] = (iml >> 8) & 0xff;
    command_rep.data[33] = (iml) & 0xff;

    iml = (int16_t)(ml2 * 100);
    command_rep.data[34] = (iml >> 8) & 0xff;
    command_rep.data[35] = (iml) & 0xff;

    command_rep.crc = crc((unsigned char *)&command_rep, command_rep.size + 3);

    write_command(&command_rep, DEFAULT_LOG_INFERFACE);
}

void read_weight1(void)
{
    if (Scale_Module.scale1.is_ready())
    {
        // float reading = Scale_Module.scale1.get_units(1);
        double read = Scale_Module.scale1.read();
        float read_scaled = read * Scale_Module.scale1_scale + Scale_Module.scale1_offset;

        Scale_Module.weight1 = (int16_t)read_scaled;

        // printf("HX711 1 %f\n", read_scaled);
    }
    else
    {
        // Serial.println("HX711 1 not found.");
    }
}

void read_weight2(void)
{
    if (Scale_Module.scale2.is_ready())
    {
        // float reading = Scale_Module.scale2.get_units(1);
        // Scale_Module.weight2 = (int16_t)reading;
        ////printf("HX711 2 %f\n", reading);
        double read = Scale_Module.scale2.read();
        float read_scaled = read * Scale_Module.scale2_scale + Scale_Module.scale2_offset;

        Scale_Module.weight2 = (int16_t)read_scaled;

        // printf("HX711 2 %f\n", read_scaled);
    }
    else
    {
        // Serial.println("HX711 2 not found.");
    }
}

void read_weight3(void)
{
    if (Scale_Module.scale3.is_ready())
    {
        // float reading = Scale_Module.scale3.get_units(1);
        // Scale_Module.weight3 = (int16_t)reading;
        // printf("HX711 3 %f\n", reading);
        double read = Scale_Module.scale3.read();
        float read_scaled = read * Scale_Module.scale3_scale + Scale_Module.scale3_offset;

        Scale_Module.weight3 = (int16_t)read_scaled;

        // printf("HX711 3 %f\n", read_scaled);
    }
    else
    {
        // Serial.println("HX711 3 not found.");
    }
}

void read_temperature_chamber_1(void)
{
    Chamber_Module.thermocouple1.read();

    float temp = Chamber_Module.thermocouple1.getTemperature();
    Chamber_Module.temperature1 = (int16_t)(temp * 10.0);
}

void read_temperature_chamber_2(void)
{
    Chamber_Module.thermocouple2.read();

    float temp = Chamber_Module.thermocouple2.getTemperature();
    Chamber_Module.temperature2 = (int16_t)(temp * 10.0);
}

void read_temperature_chamber_3(void)
{
    Chamber_Module.thermocouple3.read();

    float temp = Chamber_Module.thermocouple3.getTemperature();
    Chamber_Module.temperature3 = (int16_t)(temp * 10.0);
}

void read_temperature_tank_top(void)
{
    // digitalWrite(TEMP_AMP3_SS_PIN, HIGH);

    float temp = Tank_Top_Module.thermocouple.getThermocoupleTemp();
    Tank_Top_Module.temperature = (int16_t)(temp * 10.0);

    // digitalWrite(TEMP_AMP3_SS_PIN, LOW);
}

void read_temperature_tank_bot(void)
{
    // digitalWrite(TEMP_AMP3_SS_PIN, HIGH);
    float temp = Tank_Bot_Module.thermocouple.getThermocoupleTemp();
    Tank_Bot_Module.temperature = (int16_t)(temp * 10.0);
    // digitalWrite(TEMP_AMP3_SS_PIN, LOW);
}

void read_tank_tactile(void)
{
    // tank_t1 = ADS1.readADC(3);
    // tank_t2 = ADS2.readADC(0);
    // tank_t3 = ADS2.readADC(1);
    // tank_t4 = ADS2.readADC(2);
    // tank_t5 = ADS2.readADC(3);

    tank_tactile_bits = 0;
    if (tank_t1 >= TACTILE_THREADHOLD)
        tank_tactile_bits |= TANK_T1;
    if (tank_t2 >= TACTILE_THREADHOLD)
        tank_tactile_bits |= TANK_T2;
    if (tank_t3 >= TACTILE_THREADHOLD)
        tank_tactile_bits |= TANK_T3;
    if (tank_t4 >= TACTILE_THREADHOLD)
        tank_tactile_bits |= TANK_T4;
    if (tank_t5 >= TACTILE_THREADHOLD)
        tank_tactile_bits |= TANK_T5;

    // Serial.printf("Tank tactile %d %d %d %d %d\n", tank_t1, tank_t2, tank_t3, tank_t4, tank_t5);
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

double hL2;      // altura liquido
double Vl2, Vg2; // volume liquido / gas
double x2;       // answer
double ml2, mg2;

static void MassaVolumica(void)
{

    float T4 = (Tank_Bot_Module.temperature / 10.0f) + 273.5;
    float T5 = (Tank_Top_Module.temperature / 10.0f) + 273.5;

    // mVL = 4e-9 * T4*T4*T4*T4*T4 - 5e-6 * T4*T4*T4*T4 + 0.0083 * T4*T4*T4 + 0.9577 * T4*T4 - 168.817 * T4 + 609918;
    // mVG = 4e-9 * T4*T4*T4*T4*T4 - 5e-6 * T4*T4*T4*T4 - 0.9467 * T4*T4*T4 + 166.727 * T4*T4 - 155877 * T4 + 603845;

    mVL = -0.0258 * T4 * T4 + 8.2291 * T4 + 572.66;
    mVG = 0.0059 * exp(0.0352 * T5);
}

static void alturafluido(void)
{
    float P1 = Tank_Bot_Module.pressure * 100000;
    float P2 = Tank_Top_Module.pressure * 100000;

    // put your main code here, to run repeatedly:
    hL = (P1 - P2 - mVG * g * h_tanque) / ((mVL - mVG) * g);
    hL2 = (P1 - P2) / (mVL * g);

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

    if (hL2 < h1)
    {                                         // h1 valor conhecido distancia do transdutor de pressao ao fundo do tanque em metros
        Vl2 = ((3.1416 * Do * Do) / 4) * hL2; // Dt- Diametro do tubo
        Vg2 = V_total - Vl2;                  // V_total = constante a determinar
        ml2 = mVL * Vl2;
        mg2 = mVG * Vg2;
        tank_liquid2 = mg2 / (mg2 + ml2);
    }
    if (hL2 > h1)
    {
        Vl2 = ((3.1416 * Do * Do) / 4) * h1 + ((3.1416 * Dt * Dt) / 4) * (hL2 - h1); // Do - diametro do tanque oxidante Dt- Diâmetro do tubo                                         //h1 - diferença de altura entre transdutor e fundo do tanque
        Vg2 = V_total - Vl2;
        ml2 = mVL * Vl2;
        mg2 = mVG * Vg2;
        tank_liquid2 = mg2 / (mg2 + ml2);
    }
    // tank_pressure = P1;
}

void calc_liquid(void)
{
    // calculate amount of liquid in the tank
    for (float val : ttp_values)
    {
        Tank_Top_Module.pressure += val;
    }
    Tank_Top_Module.pressure /= press_values_size;
    for (float val : tbp_values)
    {
        Tank_Bot_Module.pressure += val;
    }
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
