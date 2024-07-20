void serialThread() {
  myPort = new Serial(this, selectedPort, baudRate);
  port_selected = true;
  myPort.clear();
  while (port_selected) {
    while (myPort.available() > 0) {
      byte rx_byte = (byte) myPort.read();
      parseIncomingByte(rx_byte);
    }

    // Send packets in queue
    byte[] head = tx_queue.peek();
    while (head != null) {
      myPort.write(head);
      tx_queue.remove();
      head = tx_queue.peek();
    }
  }
}

void parseIncomingByte(byte rx_byte) {

  if (last_read_time == 0 || millis() - last_read_time > TIMEOUT) {
    currentParseState = ParseState.START;
  }
  last_read_time = millis();
  switch(currentParseState) {
  case START:
    if (rx_byte == (byte) 0x55) {
      currentParseState = ParseState.CMD;
    } else {
      // println("Start byte not received");
    }
    break;
  case CMD:
    CMD = rx_byte;
    currentParseState = ParseState.ID;
    //println("Command Received: " + (int) CMD);
    break;
  case ID:
    ID = rx_byte;
    currentParseState = ParseState.PAYLOAD_LENGTH;
    println("Reading ID : " + ID);
    break;
  case PAYLOAD_LENGTH:
    PLEN = rx_byte;
    //println("Payload length " + (int) PLEN);
    if ((int) PLEN > 0) {
      currentParseState = ParseState.PAYLOAD;
      rx_payload_index = 0;
    } else {
      currentParseState = ParseState.CRC1;
    }
    break;
  case PAYLOAD:
    rx_payload[rx_payload_index] = rx_byte;
    rx_payload_index++;
    if (rx_payload_index >= PLEN) {
      currentParseState = ParseState.CRC1;
    }
    break;
  case CRC1:
    println("Reading CRC1");
    // CRC1 = rx_byte;
    currentParseState = ParseState.CRC2;
    break;
  case CRC2:
    println("Reading CRC2");
    // CRC2 = rx_byte;
    processPacket();
    currentParseState = ParseState.START;
  }
}

void processPacket() {
  dataPacket new_packet = new dataPacket(CMD, ID, rx_payload);
  new_packet.logPacket();
  rx_packet = new_packet;
  String stringID = str(Byte.toUnsignedInt(rx_packet.id));
  if (CMD == (byte) 0x00) { // LOG COMANDO PLACEHOLDER 0x01 se tudo correr bem
    if (ID == (byte) 0x01) {
      displayLogRocket();
    } else if (ID == (byte) 0x02) {
      displayLogFilling();
    }
  } else if (CMD == (byte) 0x13) {
    if (targetID == 1) {
      displayStatusRocket();
    } else if (targetID == 2) {
      displayStatusFilling();
    }
  }
}

void displayLogRocket() {
  String state = "State: " + str(Byte.toUnsignedInt(rx_packet.payload[0])) + "\n";
  
  rp = float((Byte.toUnsignedInt(rx_packet.payload[1]) << 8) | Byte.toUnsignedInt(rx_packet.payload[2]));
  String t1 = "Tank Pressure: " + str(rp) + "\n";
  rl = float((Byte.toUnsignedInt(rx_packet.payload[3]) << 8) | Byte.toUnsignedInt(rx_packet.payload[4]));
  String t2 = "Tank L: " + str(rl) + "\n";

  log_display_rocket.setText("Log rocket: " + state + t1 + t2);
}

void displayLogFilling() {
  String state = "Filling State: " + str(Byte.toUnsignedInt(rx_packet.payload[0])) + "\n";
  
  fp = float((Byte.toUnsignedInt(rx_packet.payload[1]) << 8) | Byte.toUnsignedInt(rx_packet.payload[2]));
  String t1 = "Tank Pressure: " + str(fp) + "\n";
  fl = float((Byte.toUnsignedInt(rx_packet.payload[3]) << 8) | Byte.toUnsignedInt(rx_packet.payload[4]));
  String t2 = "Tank L: " + str(fl) + "\n";
  
  log_display_filling.setText("Log filling: " + state + t1 + t2);
}

void displayStatusRocket() {
  String state = "State: " + str(Byte.toUnsignedInt(rx_packet.payload[0])) + "\n";
  
  rp = float((Byte.toUnsignedInt(rx_packet.payload[1]) << 8) | Byte.toUnsignedInt(rx_packet.payload[2]));
  String t1 = "Tank Pressure: " + str(rp) + "\n";
  rl = float((Byte.toUnsignedInt(rx_packet.payload[3]) << 8) | Byte.toUnsignedInt(rx_packet.payload[4]));
  String t2 = "Tank L: " + str(rl) + "\n";

  log_display_rocket.setText("Status ACK rocket: " + state + t1 + t2);
}

void displayStatusFilling() {
  String state = "Filling State: " + str(Byte.toUnsignedInt(rx_packet.payload[0])) + "\n";
  
  fp = float((Byte.toUnsignedInt(rx_packet.payload[1]) << 8) | Byte.toUnsignedInt(rx_packet.payload[2]));
  String t1 = "Tank Pressure: " + str(fp) + "\n";
  fl = float((Byte.toUnsignedInt(rx_packet.payload[3]) << 8) | Byte.toUnsignedInt(rx_packet.payload[4]));
  String t2 = "Tank L: " + str(fl) + "\n";
  
  log_display_filling.setText("Status ACK filling: " + state + t1 + t2);
}
