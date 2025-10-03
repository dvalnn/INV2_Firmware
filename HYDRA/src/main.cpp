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

bool setup_error = false;

void setup() {
    Serial.begin(USB_BAUD_RATE);    // USBC serial
    // Serial2.begin(RS485_BAUD_RATE);  // RS485

    // Initialize I2C with custom pins from IO_Map.h
    Wire.setSDA(I2C_SDA);   // Set SDA to pin 14
    Wire.setSCL(I2C_SCL);   // Set SCL to pin 15
    Wire.begin();           // Initialize I2C with custom pins
    Wire.setClock(400000);  // Set I2C clock to 400kHz

    pressures_setup();
    thermo_setup();
    valves_setup();
}

void loop() {
    // Main loop code
    if(setup_error) {
        Serial.println("Setup error detected. Halting execution.");
    } else {
        Serial.println("System running normally.");
    }
    delay(5000); // Delay for demonstration purposes
}
