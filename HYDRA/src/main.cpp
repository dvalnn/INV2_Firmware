#include <AD5593R.h>
#include <Adafruit_MAX31856.h>
#include <Arduino.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#include "Wire.h"
#include <IO_Map.h> //? Library with all I/O Hardware mapping

Adafruit_MAX31856 maxthermo1 = Adafruit_MAX31856(spi_miso, spi_mosi, thermo1_cs, spi_sck);
Adafruit_MAX31856 maxthermo2 = Adafruit_MAX31856(spi_miso, spi_mosi, thermo2_cs, spi_sck);
Adafruit_MAX31856 maxthermo3 = Adafruit_MAX31856(spi_miso, spi_mosi, thermo3_cs, spi_sck);

AD5593R ad5593r = AD5593R(0x11);

void thermo_setup(void) {
    bool thermo1_config, thermo2_config = false;
    if (!maxthermo1.begin()) {
        Serial.println("Failed to initialize thermo1!");
        thermo1_config = true;
    }
    if (!maxthermo2.begin()) {
        Serial.println("Failed to initialize thermo2!");
        thermo2_config = true;
    }
    if (!maxthermo3.begin()) {
        Serial.println("Failed to initialize thermo3!");
        if (thermo1_config && thermo2_config)
            return;
    }

    maxthermo1.setThermocoupleType(MAX31856_TCTYPE_K);
    maxthermo2.setThermocoupleType(MAX31856_TCTYPE_K);
    maxthermo3.setThermocoupleType(MAX31856_TCTYPE_K);

    maxthermo1.setConversionMode(MAX31856_CONTINUOUS);
    maxthermo2.setConversionMode(MAX31856_CONTINUOUS);
    maxthermo3.setConversionMode(MAX31856_CONTINUOUS);

    maxthermo1.setAveragingSamples(16);
    maxthermo2.setAveragingSamples(16);
    maxthermo3.setAveragingSamples(16);

    maxthermo1.setNoiseFilter(MAX31856_NOISE_FILTER_50HZ);
    maxthermo2.setNoiseFilter(MAX31856_NOISE_FILTER_50HZ);
    maxthermo3.setNoiseFilter(MAX31856_NOISE_FILTER_50HZ);

    Serial.println("MAX31856 thermocouples initialized!");
}

void dac_adc_setup(void) {
    // Initialize I2C communication with AD5593R
    if (!ad5593r.begin()) {
        Serial.println("Failed to initialize AD5593R!");
        return;  // Exit if initialization fails
    }

    ad5593r.reset();  // Reset the device to ensure clean state
    delay(10);        // Small delay after reset

    ad5593r.setExternalReference(false, 2.5);  // Configure voltage reference (false = internal 2.5V reference)
    ad5593r.setMode("AAAATDDD");               // Set I/O configuration: 0-3 as ADC, 4 as THREESTATE (unused), 5-7 as DAC
    ad5593r.powerDownDac(4);                   // Power down I/O4 to save power (since it's unused)

    // ADC Configs:
    ad5593r.setADCRange2x(true);             // ADC input range: 0V to 5V (2x Vref)
    ad5593r.enableADCBuffer(true);           // Enable ADC buffer for better accuracy (optional but recommended)
    ad5593r.enableADCBufferPreCharge(true);  // Enable ADC buffer pre-charge for faster settling (optional)

    // DAC Configs:
    ad5593r.setDACRange2x(true);
    ad5593r.setLDACmode(AD5593R_LDAC_DIRECT);  // Set LDAC mode (direct write to DAC outputs)

    // With 2x range (5V) and 12-bit resolution: output = (Vout/5.0) * 4095
    uint16_t dac_output = (2.5 / 5.0) * 4095;  // 2.5 output voltage

    ad5593r.writeDAC(5, dac_output);  // Set DAC channel 5
    ad5593r.writeDAC(6, dac_output);  // Set DAC channel 6
    ad5593r.writeDAC(7, dac_output);  // Set DAC channel 7

    Serial.println("AD5593R initialized and configured!");
    Serial.print("Device address: 0x");
    Serial.println(ad5593r.getAddress(), HEX);
    Serial.println("DAC channels 5-7 set to 2.5V output");
    return;
}

void valves_setup(void) {
    pinMode(SOL_VALVE_1, OUTPUT);
    pinMode(SOL_VALVE_2, OUTPUT);
    pinMode(SOL_VALVE_3, OUTPUT);
    pinMode(QuickDisco_N2O, OUTPUT);
    pinMode(QuickDisco_N2, OUTPUT);
    pinMode(STVALVE_1, OUTPUT);
    pinMode(STVALVE_2, OUTPUT);
    pinMode(CAM_EN, OUTPUT);
    pinMode(PWM_Sig, OUTPUT);
}



void setup() {
    // Serial.begin(SERIAL_BAUD);    // USBC serial
    // Serial2.begin(SERIAL2_BAUD);  // RS485
    
    // Initialize I2C with custom pins from IO_Map.h
    Wire.setSDA(i2c_sda);   // Set SDA to pin 14
    Wire.setSCL(i2c_scl);   // Set SCL to pin 15
    Wire.begin();           // Initialize I2C with custom pins
    Wire.setClock(400000);  // Set I2C clock to 400kHz

    dac_adc_setup();
    thermo_setup();
    valves_setup();
}



//     Valves_Setup();

//     Pressure_Setup();

//     Spi_Thermo_Setup();
//     // Flash_Setup();

//     printf("Setup done\n");

//     state_machine[state].entry_time = millis();
//     for (int i = 0; i < MAX_WORK_SIZE; i++)
//         state_machine[state].work[i].begin =
//             state_machine[state].work[i].delay;

//     // while(1) {}
// }

// void loop() {
//     // static rocket_state_t state = IDLE;
//     rocket_state_t command_state = state,
//                    event_state = state;

//     /*
//         Execute the state function
//      */
//     bool work_performed = WORK_HANDLER();

//     /*
//      Event handling
//      */
//     if (work_performed) event_state = EVENT_HANDLER();
//     if (event_state == -1) event_state = state;

//     /*
//      Comms
//      */
//     int error;

//     // check if we have new data
//     command_t* cmd = read_command(&error, DEFAULT_CMD_INTERFACE);
//     if (cmd != NULL &&
//         error == CMD_READ_OK &&
//         run_command(cmd, state, DEFAULT_CMD_INTERFACE) == CMD_RUN_OK) {
//         // make transition to new state on the state machine
//         if (state_machine[state].comms[cmd->cmd] != -1)
//             // we have new state, use lookup table
//             command_state = (rocket_state_t)comm_transition[state][cmd->cmd];
//     }

//     /*
//      * Do state transition
//      */
//     if (command_state != state) {
//         // command change of state as priority over
//         // internal events changes of state
//         state = command_state;
//         state_machine[state].entry_time = millis();

//         // reset sensor timer
//         for (int i = 0; i < MAX_WORK_SIZE; i++)
//             state_machine[state].work[i].begin =
//                 state_machine[state].work[i].delay;

//         log(&state, 0, STATE_CHANGE);
//     }

//     // used as the time base when dealing with sensor sampling rate and delays
//     else if (event_state != state) {
//         // only if comms haven't changed the state we can
//         // acept a new state from internal events
//         state = event_state;
//         state_machine[state].entry_time = millis();

//         // reset sensor timer
//         for (int i = 0; i < MAX_WORK_SIZE; i++)
//             state_machine[state].work[i].begin =
//                 state_machine[state].work[i].delay;

//         log(&state, 0, STATE_CHANGE);
//     }

//     delay(1);
// }
