#include "StateMachine.h"
#include "GlobalVars.h"
#include "HardwareCfg.h"
#include "StMWork.h"

#define ADS1115_CHANNELS 4
#define ADS_MEASURE_DELAY 2 //MS

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
    ADS1.setCompareChannels(Tank_Top_Module.ADC_pressure_id);
    ADS1.startSingleMeasurement();
    
    ads_measure_time = millis();
    ads_target = ads_tank_top;
}

void read_pressure_tank_bot(void)
{
    ADS1.setCompareChannels(Tank_Bot_Module.ADC_pressure_id);
    ADS1.startSingleMeasurement();
    
    ads_measure_time = millis();
    ads_target = ads_tank_bot;
}

void read_chamber_pressure(void)
{
    ADS1.setCompareChannels(Chamber_Module.ADC_pressure_id);
    ADS1.startSingleMeasurement();
    
    ads_measure_time = millis();
    ads_target = ads_chamber;
}

bool ADS_event()
{
    return ((ads_target != ads_none) && (millis() - ads_measure_time > ADS_MEASURE_DELAY));
}

static float calibrated_pressure(float value, pressure_calib paramm)
{
    //Serial.printf("V: %f %f %f %f\n", ADS1.toVoltage(value), paramm.b, paramm.m, paramm.b + (ADS1.toVoltage(value) * paramm.m));
    return paramm.b + (value * paramm.m);
}

void ADS_reader(void)
{
    //Serial2.printf("Ads reader");
    //Serial2.flush();
    
    //while(ADS1.isBusy()) {}

    const float betha = 0.125f;
    float voltage = 0.0;
    voltage = ADS1.getResult_V(); // alternative: getResult_mV for Millivolt

    switch(ads_target)
    {
        case ads_tank_top: 
        {
            static float lfp_val = 0.0f;
            float pressure_calib = calibrated_pressure(voltage,
                                    Tank_Top_Module.pressure_serial);
        
            lfp_val = lfp_val - (Betha * (lpf - pressure_calib));
            Tank_Top_Module.pressure = lfp_val;
        }
        break;
        case ads_tank_bot:
        {
            static float lfp_val = 0.0f;
            float pressure_calib = calibrated_pressure(voltage,
                                    Tank_Bot_Module.pressure_serial);
        
            lfp_val = lfp_val - (Betha * (lpf - pressure_calib));
            Tank_Bot_Module.pressure = lfp_val;
        }
        break;
        case ads_chamber:
        {
            static float lfp_val = 0.0f;
            float pressure_calib = calibrated_pressure(voltage,
                                    Chamber_Module.pressure_serial);
        
            lfp_val = lfp_val - (Betha * (lpf - pressure_calib));
            Chamber_Module.pressure = lfp_val;
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
        {.channel = read_chamber_pressure}
    };

    if(ads_target != ads_none) return;

    unsigned long end = millis();
    unsigned long msec_best = (1 << 32 - 1);
    int8_t index_best = -1;

    for(int i = 0; i < ADS1115_CHANNELS; i++)
    {
        if(channel[i].channel == NULL)
            continue;
        
        //avoid overflows when it produce negative msec
        if(end < channel[i].begin)
            continue;
        
        unsigned long msec = end - channel[i].begin; 

        if(sample_times[i] > 0 &&
           msec > (unsigned long)sample_times[i] && 
           channel[i].begin < msec_best)
        {
            msec_best = channel[i].begin;
            index_best = i;
        }
        //else
        //{
            //Serial2.printf("channel %d %d %d %d ", i, 
                //sample_times[i] > 0, 
                //msec > (unsigned long)sample_times[i], 
                //channel[i].begin < msec_best);
        //}
    }

    if(index_best != -1)
    {
        channel[index_best].begin = end;
        channel[index_best].channel();
    }
    //else
    //{
        //Serial2.printf("-1\nchannel 0: %u channel 1: %u now %d", 
            //end - channel[0].begin,
            //end - channel[1].begin,
            //end);
    //}

    //Serial2.printf("ads channel %d", index_best);
    //Serial2.flush();
}

void ADS_handler_slow()
{
    int16_t arr[] = {1000,1000,-1,-1}; 
    ADS_handler(arr);
}

void ADS_handler_fast()
{
    int16_t arr[] = {50,50,-1,-1}; 
    ADS_handler(arr);
}

void ADS_handler_all_slow()
{
    int16_t arr[] = {500,500,500,-1};
    ADS_handler(arr);
}

void ADS_handler_all_fast()
{
    int16_t arr[] = {30,30,30,-1};
    ADS_handler(arr);
}
