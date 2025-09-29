#include "StateMachine.h"
#include "StMWork.h"
#include "GlobalVars.h"

float altitude = 0;
float maxAltitude = 0;
float maxAltitude_local = 0;

float ground_hPa = 0;

float gps_offset = 0;

alt_kalman alt_kal;
QuaternionFilter att;
Eigen::Matrix<float, 9, 1> alt_kalman_state; // altitude | vertical velocity | vertical acceleration
float q[4];

float imu_ax_local;
float imu_ay_local;
float imu_az_local;
float imu_gx_local;
float imu_gy_local;
float imu_gz_local;

float altitude_local;

float imu_ax,imu_ay,imu_az; //accel
float imu_gx,imu_gy,imu_gz; //gyro

float gps_lat, gps_lon;
float gps_altitude;
float gps_horizontal_vel;
uint16_t gps_satalites;

float kalman_altitude;
float kalman_velocity;
float kalman_accel;
float kalman_q[4];

unsigned long transmit_time;

void write_values(void)
{
    altitude = altitude_local;

    imu_ax = imu_ax_local; 
    imu_ay = imu_ay_local; 
    imu_az = imu_az_local; 

    imu_gx = imu_gx_local; 
    imu_gy = imu_gy_local; 
    imu_gz = imu_gz_local; 

    kalman_altitude = alt_kalman_state(6);
    kalman_velocity = alt_kalman_state(7);
    kalman_accel = alt_kalman_state(8);

    maxAltitude = maxAltitude_local;

    kalman_q[0] = q[0];
    kalman_q[1] = q[1];
    kalman_q[2] = q[2];
    kalman_q[3] = q[3];
}

void read_imu(void)
{
    static float avg_x_arr[10];
    static float avg_y_arr[10];
    static float avg_z_arr[10];

    static float sum_x = 0;
    static float sum_y = 0;
    static float sum_z = 0;

    static float imu_gx_last = 0;
    static float imu_gy_last = 0;
    static float imu_gz_last = 0;

    static uint8_t avg_index = 0;

    static float offset_vector[] = {0.008, -0.01, 0.0215};

    IMU.update_accel_gyro();

    float imu_ax_temp = IMU.getAccX();
    float imu_ay_temp = IMU.getAccY();
    float imu_az_temp = IMU.getAccZ();


    sum_x += imu_ax_temp;
    sum_x -= avg_x_arr[avg_index];
    avg_x_arr[avg_index] = imu_ax_temp;

    sum_y += imu_ay_temp;
    sum_y -= avg_y_arr[avg_index];
    avg_y_arr[avg_index] = imu_ay_temp;
    
    sum_z += imu_az_temp;
    sum_z -= avg_z_arr[avg_index];
    avg_z_arr[avg_index] = imu_az_temp;

    avg_index = ((avg_index + 1) % 10);

    imu_ax_local = sum_x / 10;
    imu_ay_local = sum_z / 10;
    //imu_az = -(sum_y / 10) - (9.9)/10.0;
    imu_az_local = (sum_y / 10) + (10.0)/10.0;
    //imu_az_local = (sum_y / 10) ;
    
    imu_gx_local = IMU.getGyroX();
    imu_gy_local = IMU.getGyroZ();
    imu_gz_local = IMU.getGyroY();

    //imu_ax_local = imu_ax_local - (imu_gx_local - imu_gx_last) * offset_vector[0] - imu_gx_local * (imu_gx_local * offset_vector[0]);
    //imu_ay_local = imu_ay_local - (imu_gy_local - imu_gy_last) * offset_vector[1] - imu_gy_local * (imu_gy_local * offset_vector[1]);
    //imu_az_local = imu_az_local - (imu_gz_local - imu_gz_last) * offset_vector[2] - imu_gz_local * (imu_gz_local * offset_vector[2]);

    imu_gx_last = imu_gx_local;
    imu_gy_last = imu_gy_local;
    imu_gz_last = imu_gz_local;

    //att.update(imu_ax_local, imu_ay_local, imu_az_local, imu_gx_local, imu_gy_local, imu_gz_local, 0, 0, 0, q);

    //Serial.printf("%f %f %f\n", imu_ax_local, imu_ay_local, imu_az_local);

    return;
}

void read_barometer(void)
{
    static float lpf_alt = 0.0f;

    bool flag = false;
    if(millis() - transmit_time < LOW_AFTER_TRANSMIT)
    {
        //bmp.readAltitude(ground_hPa);
        flag = true;
    }
    if(flag) return;
        

    // LOW PASS altitude
    lpf_alt = bmp.readAltitude(ground_hPa);
    altitude_local += (lpf_alt - altitude_local) * betha_alt;

    //Serial.printf("Altitude barometer %f\n", altitude_local);

    //Serial.printf("Pressure %f Temp %d\n", bmp.readPressure(), bmp.readTemperature());
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
    static float gps_alt_offset, alt_offset, last_lat, last_long;
    static bool first = true;

    static unsigned long last = millis();

    //Serial.printf("dt: %d\n", millis() - last);
    //last = millis();
    //Serial.flush();

    if (first)
    {
        acc << imu_ax_local, imu_ay_local, imu_az_local;
        norm_acc = acc / acc.norm();
        axis = norm_acc.cross(norm_g);
        axis = axis / axis.norm();

        float t = iacos(norm_acc.dot(norm_g));

        q[0] = icos(t / 2.0);
        q[1] = axis(0) * isin(t / 2);
        q[2] = axis(1) * isin(t / 2);
        q[3] = axis(2) * isin(t / 2);
        first = false;
        
        //float preferences_altitude = preferences.getFloat("altitude", 0);
        //Z_2 << 0, 0, 0, 0, 0, 0, preferences_altitude, 0, 0;
        Z_2 << 0, 0, 0, 0, 0, 0, 0, 0, 0;
    }

    //Q.qx = q[0];
    //Q.qy = q[1];
    //Q.qz = q[2];
    //Q.qw = q[3];

    //Q = euler_to_quaternion(0,0,0);

    Q.qw = 1;
    Q.qx = 0;
    Q.qy = 0;
    Q.qz = 0;


    //quaternion_to_euler(Q, angles);

    //Serial.printf("%f %f %f \n", angles[0], angles[1], angles[2]);
    //Serial.flush();

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
    Acc << imu_ax_local, imu_ay_local, imu_az_local;

    U_2 << 0, 0, Acc(0), 0, 0, Acc(1), 0, 0, Acc(2);
    Z_2 << 0, 0, Acc(0), 0, 0, Acc(1), altitude_local, 0, Acc(2); 
    //Z_2 << 0, 0, 0, 0, 0, 0, altitude, 0, 0; 


#ifdef KALMAN_DEBBUG
    Serial.println("measurements done");
    Serial.println("Starting kalman");
#endif
    alt_kalman_state = alt_kal.cicle(Q, Z_2, U_2);

    if (Launch)
        maxAltitude_local = max(maxAltitude_local, alt_kalman_state(6));

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

    //Serial.print(" |altitude1:");
    //Serial.print(alt_kalman_state(6));
    //Serial.print(" |vel_Z:");
    //Serial.print(alt_kalman_state(7));
    //Serial.print(" |Acc_Z:");
    //Serial.print(alt_kalman_state(8));
    //Serial.print(" |Update:");
    //Serial.print(debug_flag);
    //Serial.print(" |GPS:");
    //Serial.print(gps.altitude.meters());
    //Serial.printf("(%d)", gps.satellites.value());
    //Serial.println();
    //Serial.flush();

    debug_flag = false;

#ifdef KALMAN_DEBBUG
    Serial.println("altitude done");
#endif
}

void control_work(void* parameters)
{

    Work_t control_tasks[] = { 
      {.channel = read_barometer, .sample = BMP_READ_TIME}, 
      {.channel = read_imu, .sample = IMU_READ_TIME}, 
      {.channel = kalman, .sample = 20},
      {.channel = write_values, .sample = 20},
    };

    Serial.printf("got to control work %d\n\r", sizeof(control_tasks) / sizeof(Work_t));
    Serial.flush();

    unsigned long entry_time = 0;

    for(int i = 0; i < sizeof(control_tasks) / sizeof(Work_t); i++)
        control_tasks[i].begin = control_tasks[i].delay;

    while(true)
    {
        for (int i = 0; i < sizeof(control_tasks) / sizeof(Work_t); i++)
        {
            unsigned long end = millis() - entry_time;

            if (control_tasks[i].channel == NULL)
                continue;

            unsigned long msec = (end - control_tasks[i].begin);

            // avoid overflows when it produce negative msec
            if (end < control_tasks[i].begin)
                continue;

            if (msec > control_tasks[i].sample)
            {
                control_tasks[i].begin = end;
                control_tasks[i].channel(); // execute sample function
            }
        }
    }
}