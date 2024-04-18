// Packet Structure: START BYTE(0x55) | COMMAND | PAYLOAD LENGTH | PAYLOAD | CRC

class dataPacket {
  final byte START_BYTE = (byte)0x55;
  byte command;
  byte payloadLength;
  byte[] payload;
  byte id;
  byte crc;

  dataPacket(byte command, byte[] payload) {
    this.command = command;
    this.payload = payload;
    this.payloadLength = (byte)payload.length;
    this.id = (byte)0x01; // placeholder id
    this.crc = (byte) 0x00;
  }

  void logPacket() {
    // This method is for displaying the packet's content for debugging or verification
    log(new String(getPacket()));
  }
  
  byte[] getPacket() {
    byte[] finalPacket = {START_BYTE, command, payloadLength};
    for (int i = 0; i < payloadLength; i++) {
      finalPacket = append(finalPacket, payload[i]);
    }
    finalPacket = append(finalPacket, crc);
    finalPacket = append(finalPacket, crc);
    return finalPacket;
  }
}
