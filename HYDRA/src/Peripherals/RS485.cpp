#include "Peripherals/RS485.h"
#include "Peripherals/IO_Map.h"
#include "HardwareCfg.h"

void rs485_init() {
    pinMode(ENABLE_RS_PIN, OUTPUT);
    digitalWrite(ENABLE_RS_PIN, LOW);   // start in receive mode
    Serial1.begin(RS485_BAUD_RATE); // RS485
}

void rs485_send(uint8_t *data, size_t length) {
    digitalWrite(ENABLE_RS_PIN, HIGH);  // switch to transmit mode
    Serial1.write(data, length);
    Serial1.flush();
    digitalWrite(ENABLE_RS_PIN, LOW);   // switch back to receive mode
}