#include <SPI.h>
#include <LoRa.h>

int counter = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("LoRa Sender");

  LoRa.setPins(5,4,36);
  LoRa.setSignalBandwidth(300E3);
  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

void loop() {
  uint8_t buff[100] = {0};
  int size = 0;
  while(Serial.available())
  {
    buff[size++] = Serial.read();
  }

  if(size > 0)
  {
    //Serial.println("Got bytes");
    LoRa.beginPacket();
    LoRa.write(buff, size);
    LoRa.endPacket(true);
  }

  // read packet
  int packetSize = LoRa.parsePacket();
  while (packetSize != 0 && LoRa.available()) {
    Serial.write((char)LoRa.read());
  }

}