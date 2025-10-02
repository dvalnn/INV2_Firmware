#include <Arduino.h>
#include <AD5593R.h>
#include <MAX31856.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include "Wire.h"


MAX31856 maxthermo1 = MAX31856(8, 11, 9, 10);
MAX31856 maxthermo2 = MAX31856(8, 11, 13, 10);
MAX31856 maxthermo3 = MAX31856(8, 11, 29, 10);

AD5593R ad5593r = AD5593R(0x11);

void thermo_setup(void) {

}

void dac_adc_setup(void) {
    // Initialize I2C communication with AD5593R
    if (!ad5593r.begin()) {
        Serial.println("Failed to initialize AD5593R!");
        return;  // Exit if initialization fails
    }

    ad5593r.reset(); //Reset the device to ensure clean state
    delay(10);  // Small delay after reset

    ad5593r.setExternalReference(false, 2.5); // Configure voltage reference (false = internal 2.5V reference)
    ad5593r.setMode("AAAATDDD"); // Set I/O configuration: 0-3 as ADC, 4 as THREESTATE (unused), 5-7 as DAC
    
    // Power down I/O4 to save power (since it's unused)
    ad5593r.powerDownDac(4);

    // ADC Configs:
    ad5593r.setADCRange2x(true); // ADC input range: 0V to 5V (2x Vref) 
    ad5593r.enableADCBuffer(true); // Enable ADC buffer for better accuracy (optional but recommended)    
    ad5593r.enableADCBufferPreCharge(true); // Enable ADC buffer pre-charge for faster settling (optional)

    // DAC Configs:
    ad5593r.setDACRange2x(true);
    ad5593r.setLDACmode(AD5593R_LDAC_DIRECT); // Set LDAC mode (direct write to DAC outputs)
    
    // With 2x range (5V) and 12-bit resolution: output = (Vout/5.0) * 4095
    uint16_t dac_output = (2.5/5.0) * 4095;  // 2.5 output voltage
    
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

}

void setup() {
    // Serial.begin(SERIAL_BAUD);    // USBC serial
    // Serial2.begin(SERIAL2_BAUD);  // RS485

    Wire.begin();
    Wire.setClock(400000);

    dac_adc_setup();
    thermo_setup();
}


//     pinMode(TEMP_AMP1_SS_PIN, OUTPUT);
//     pinMode(TEMP_AMP2_SS_PIN, OUTPUT);
//     pinMode(TEMP_AMP3_SS_PIN, OUTPUT);
//     pinMode(LORA_SS_PIN, OUTPUT);
//     pinMode(Flash_SS_PIN, OUTPUT);
//     pinMode(EMATCH_IGNITE_PIN, OUTPUT);

//     digitalWrite(TEMP_AMP1_SS_PIN, HIGH);
//     digitalWrite(TEMP_AMP2_SS_PIN, HIGH);
//     digitalWrite(TEMP_AMP3_SS_PIN, HIGH);
//     digitalWrite(LORA_SS_PIN, HIGH);
//     digitalWrite(Flash_SS_PIN, HIGH);
//     digitalWrite(EMATCH_IGNITE_PIN, LOW);

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
