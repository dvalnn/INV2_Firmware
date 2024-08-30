// Packet Structure: START BYTE(0x55) | COMMAND | PAYLOAD LENGTH | PAYLOAD | CRC

class dataPacket {
  final byte START_BYTE = (byte)0x55;
  byte command;
  byte id;
  byte payloadLength;
  byte[] payload;
  byte crc;

  dataPacket(byte command, byte id, byte[] payload) {
    this.command = command;
    this.payload = payload;
    this.payloadLength = (byte)payload.length;
    this.id = id; 
    this.crc = (byte) 0x00;
  }

  void logPacket(LogEvent event) {
    flash_log(this, event);
  }
  
  byte[] getPacket() {
    byte[] finalPacket = {START_BYTE, command, id, payloadLength};
    for (int i = 0; i < payloadLength; i++) {
      finalPacket = append(finalPacket, payload[i]);
    }
    finalPacket = append(finalPacket, crc);
    finalPacket = append(finalPacket, crc);
    return finalPacket;
  }
}
