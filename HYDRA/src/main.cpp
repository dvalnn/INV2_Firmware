#include <Arduino.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#include <Wire.h>

#include "IO_Map.h"
#include "HardwareCfg.h" 

#include "Thermo.h"
#include "Pressures.h"
#include "Valves.h"
#include "Buzzer.h"

bool setup_error = false;

void setup() {
    setup_buzzer();
    Serial.begin(USB_BAUD_RATE);    // USBC serial
    // Serial2.begin(RS485_BAUD_RATE);  // RS485

    // Initialize I2C with custom pins from IO_Map.h
    //Wire.setSDA(I2C_SDA);   // Set SDA to pin 14
    //Wire.setSCL(I2C_SCL);   // Set SCL to pin 15
    //Wire.begin();           // Initialize I2C with custom pins
    //Wire.setClock(400000);  // Set I2C clock to 400kHz
    
    //setup_error = pressures_setup();
    setup_error = thermo_setup();
    setup_error = valves_setup();
    
    // setup status leds and buzzer
    pinMode(RED_STATUS, OUTPUT);
    pinMode(GREEN_STATUS, OUTPUT);

    if(setup_error) {
        Serial.println("Setup error detected!");
        play_buzzer_error();
    } else {
        Serial.println("Setup complete!");
        play_buzzer_success();
    }
}

void loop() {
}
