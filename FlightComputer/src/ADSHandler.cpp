#include "StateMachine.h"
#include "GlobalVars.h"
#include "HardwareCfg.h"
#include "StMWork.h"

#define ADS1115_CHANNELS 4
#define ADS_MEASURE_DELAY 8 // MS

typedef enum
{
    ads_none,
    ads_tank_top,
    ads_tank_bot,
    ads_chamber,
    ads_tactile1
} ads_targets_t;

ads_targets_t ads_target = ads_none;
unsigned long ads_measure_time;

void read_pressure_tank_top(void)
{
    ADS.setCompareChannels(Tank_Top_Module.ADC_pressure_id);
    ADS.startSingleMeasurement();

    
    ads_measure_time = millis();
    ads_target = ads_tank_top;
}

void read_pressure_tank_bot(void)
{
    ADS.setCompareChannels(Tank_Bot_Module.ADC_pressure_id);
    ADS.startSingleMeasurement();

    ads_measure_time = millis();
    ads_target = ads_tank_bot;
}

void read_chamber_pressure(void)
{
    ADS.setCompareChannels(Chamber_Module.ADC_pressure_id);
    ADS.startSingleMeasurement();

    ads_measure_time = millis();
    ads_target = ads_chamber;
}

bool ADS_event()
{
    return ((ads_target != ads_none) && (millis() - ads_measure_time > ADS_MEASURE_DELAY));
}

static float calibrated_pressure(float value, pressure_calib paramm)
{
    // Serial.printf("V: %f %f %f %f\n", ADS1.toVoltage(value), paramm.b, paramm.m, paramm.b + (ADS1.toVoltage(value) * paramm.m));
    return paramm.b + (value * paramm.m);
}

void ADS_reader(void)
{
    //Serial2.printf("Ads reader");
    //Serial2.flush();

    // while(ADS1.isBusy()) {}

    const float betha = 0.250f;
    float voltage = 0.0;
    voltage = ADS.getResult_V(); // alternative: getResult_mV for Millivolt

    switch (ads_target)
    {
    case ads_tank_top:
    {
        static float lpf_val = 0.0f;
        float pressure_calib = calibrated_pressure(voltage,
                                                   Tank_Top_Module.pressure_serial);

        lpf_val = lpf_val - (betha * (lpf_val - pressure_calib));
        ttp_values[ttp_index] = lpf_val;
        ttp_index = (ttp_index + 1) % press_values_size;
        Tank_Top_Module.pressure = lpf_val;
    }
    break;
    case ads_tank_bot:
    {
        static float lpf_val = 0.0f;
        float pressure_calib = calibrated_pressure(voltage,
                                                   Tank_Bot_Module.pressure_serial);

        lpf_val = lpf_val - (betha * (lpf_val - pressure_calib));
        tbp_values[tbp_index] = lpf_val;
        tbp_index = (tbp_index + 1) % press_values_size;
        Tank_Bot_Module.pressure = lpf_val;
    }
    break;
    case ads_chamber:
    {
        static float lpf_val = 0.0f;
        float pressure_calib = calibrated_pressure(voltage,
                                                   Chamber_Module.pressure_serial);

        lpf_val = lpf_val - (betha * (lpf_val - pressure_calib));
        chp_values[chp_index] = lpf_val;
        chp_index = (chp_index + 1) % press_values_size;
        Chamber_Module.pressure = lpf_val;
    }
    break;
    }

    ads_target = ads_none;

    return;
}

void ADS_handler(int16_t sample_times[ADS1115_CHANNELS])
{
    static Work_t channel[ADS1115_CHANNELS] =
        {
            {.channel = read_pressure_tank_top},
            {.channel = read_pressure_tank_bot},
            {.channel = read_chamber_pressure}};

    if (ads_target != ads_none)
        return;

    unsigned long end = millis();
    unsigned long msec_best = (1 << 32 - 1);
    int8_t index_best = -1;

    for (int i = 0; i < ADS1115_CHANNELS; i++)
    {
        if (channel[i].channel == NULL)
            continue;

        // avoid overflows when it produce negative msec
        if (end < channel[i].begin)
            continue;

        unsigned long msec = end - channel[i].begin;

        if (sample_times[i] > 0 &&
            msec > (unsigned long)sample_times[i] &&
            channel[i].begin < msec_best)
        {
            msec_best = channel[i].begin;
            index_best = i;
        }
    }

    if (index_best != -1)
    {
        channel[index_best].begin = end;
        channel[index_best].channel();
    }
}

void ADS_handler_slow()
{
    int16_t arr[] = {1000, 1000, -1, -1};
    ADS_handler(arr);
}

void ADS_handler_fast()
{
    int16_t arr[] = {50, 50, -1, -1};
    ADS_handler(arr);
}

void ADS_handler_all_slow()
{
    int16_t arr[] = {500, 500, 500, -1};
    ADS_handler(arr);
}

void ADS_handler_all_fast()
{
    int16_t arr[] = {50, 50, 50, -1};
    ADS_handler(arr);
}
