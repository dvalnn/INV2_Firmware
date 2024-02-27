// Packet Structure: START BYTE(0x55) | COMMAND | PAYLOAD LENGTH | PAYLOAD | CRC

class dataPacket {
  final byte START_BYTE = 0x55;
  byte command;
  byte payloadLength;
  byte[] payload;
  byte crc;

  dataPacket(byte command, byte[] payload) {
    this.command = command;
    this.payload = payload;
    this.payloadLength = (byte)payload.length;
    this.crc = calculateCRC();
  }

  private byte calculateCRC() {
    // Placeholder for CRC calculation logic
    // This method should calculate the CRC based on the command, payload length, and payload
    // For demonstration, it returns a dummy value
    return 0x00;
  }

  void displayPacket() {
    // This method is for displaying the packet's content for debugging or verification
    text("Packet Structure:", 100, 100);
    text("START BYTE: " + START_BYTE, 100, 120);
    text("COMMAND: " + command, 100, 140);
    text("PAYLOAD LENGTH: " + payloadLength, 100, 160);
    String payloadString = new String(payload);
    text("PAYLOAD: " + payloadString, 100, 180);
    text("\nCRC: " + crc, 100, 200);
  }
  
  byte[] getPacket() {
    byte[] finalPacket = {START_BYTE, command, payloadLength};
    for (int i = 0; i < payloadLength; i++) {
      finalPacket = append(finalPacket, payload[i]);
    }
    finalPacket = append(finalPacket, crc);
    return finalPacket;
  }
}
