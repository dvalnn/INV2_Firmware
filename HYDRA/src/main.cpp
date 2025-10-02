#include <AD5593R.h>
#include <Arduino.h>
#include <MAX31856.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif

MAX31856 maxthermo1 = MAX31856(8, 11, 9, 10);
MAX31856 maxthermo2 = MAX31856(8, 11, 13, 10);
MAX31856 maxthermo3 = MAX31856(8, 11, 29, 10);

AD5593R ad5593r = AD5593R(0x11);

void thermo_setup(void) {
}

void adc_setup(void) {
    if (!ad5593r.begin()) {
        Serial.println("Failed to initialize AD5593R!");
    }
    //? Set I/O 0 to 3 as ADC and I/O 5 to 7 as DAC
    ad5593r.setMode("AAAAODDD");
    ad5593r.setExternalReference(false, 2.5);
    ad5593r.setDACRange2x(true);
    
    Serial.println("AD5593R initialized and configured!");
}

void valves_setup(void) {

}

void setup() {  
    Serial.begin(SERIAL_BAUD);    // USBC serial
    Serial2.begin(SERIAL2_BAUD);  // RS485

// join I2C bus (I2Cdev library doesn't do this automatically)
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    Wire.begin();
    Wire.setClock(100000);
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
    Fastwire::setup(400, true);
#endif

    pinMode(TEMP_AMP1_SS_PIN, OUTPUT);
    pinMode(TEMP_AMP2_SS_PIN, OUTPUT);
    pinMode(TEMP_AMP3_SS_PIN, OUTPUT);
    pinMode(LORA_SS_PIN, OUTPUT);
    pinMode(Flash_SS_PIN, OUTPUT);
    pinMode(EMATCH_IGNITE_PIN, OUTPUT);

    digitalWrite(TEMP_AMP1_SS_PIN, HIGH);
    digitalWrite(TEMP_AMP2_SS_PIN, HIGH);
    digitalWrite(TEMP_AMP3_SS_PIN, HIGH);
    digitalWrite(LORA_SS_PIN, HIGH);
    digitalWrite(Flash_SS_PIN, HIGH);
    digitalWrite(EMATCH_IGNITE_PIN, LOW);

    Valves_Setup();

    Pressure_Setup();

    Spi_Thermo_Setup();
    // Flash_Setup();

    printf("Setup done\n");

    state_machine[state].entry_time = millis();
    for (int i = 0; i < MAX_WORK_SIZE; i++)
        state_machine[state].work[i].begin =
            state_machine[state].work[i].delay;

    // while(1) {}
}

void loop() {
    // static rocket_state_t state = IDLE;
    rocket_state_t command_state = state,
                   event_state = state;

    /*
        Execute the state function
     */
    bool work_performed = WORK_HANDLER();

    /*
     Event handling
     */
    if (work_performed) event_state = EVENT_HANDLER();
    if (event_state == -1) event_state = state;

    /*
     Comms
     */
    int error;

    // check if we have new data
    command_t* cmd = read_command(&error, DEFAULT_CMD_INTERFACE);
    if (cmd != NULL &&
        error == CMD_READ_OK &&
        run_command(cmd, state, DEFAULT_CMD_INTERFACE) == CMD_RUN_OK) {
        // make transition to new state on the state machine
        if (state_machine[state].comms[cmd->cmd] != -1)
            // we have new state, use lookup table
            command_state = (rocket_state_t)comm_transition[state][cmd->cmd];
    }

    /*
     * Do state transition
     */
    if (command_state != state) {
        // command change of state as priority over
        // internal events changes of state
        state = command_state;
        state_machine[state].entry_time = millis();

        // reset sensor timer
        for (int i = 0; i < MAX_WORK_SIZE; i++)
            state_machine[state].work[i].begin =
                state_machine[state].work[i].delay;

        log(&state, 0, STATE_CHANGE);
    }

    // used as the time base when dealing with sensor sampling rate and delays
    else if (event_state != state) {
        // only if comms haven't changed the state we can
        // acept a new state from internal events
        state = event_state;
        state_machine[state].entry_time = millis();

        // reset sensor timer
        for (int i = 0; i < MAX_WORK_SIZE; i++)
            state_machine[state].work[i].begin =
                state_machine[state].work[i].delay;

        log(&state, 0, STATE_CHANGE);
    }

    delay(1);
}
