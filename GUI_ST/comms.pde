enum ParseState {
  START,
    CMD,
    ID,
    PAYLOAD_LENGTH,
    PAYLOAD,
    CRC1,
    CRC2,
};

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

  if (last_read_time == 0 || millis() - last_read_time > packet_read_timeout) {
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
    break;
  case PAYLOAD_LENGTH:
    PLEN = rx_byte;
    //println("Payload length " + (int) PLEN);
    if ((int) PLEN > 0) {
      currentParseState = ParseState.PAYLOAD;
      rx_payload = new byte[PLEN];
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
    currentParseState = ParseState.CRC2;
    break;
  case CRC2:
    processPacket();
    currentParseState = ParseState.START;
  }
}

void processPacket() {
  dataPacket new_packet = new dataPacket(CMD, ID, rx_payload);
  rx_packet = new_packet;
  rx_packet.logPacket(LogEvent.MSG_RECEIVED);
  println(rx_packet.getPacket());
  if (CMD == (byte) 0x00) { // LOG COMANDO PLACEHOLDER 0x01 se tudo correr bem
    if (ID == (byte) 0x02) {
      displayLogRocket();
      updateLogStats(1);
    } else if (ID == (byte) 0x01) {
      displayLogFilling();
      updateLogStats(2);
    }
  } else if (CMD == (byte) 0x0e) { // STATUS ACK
    displayAck((int)0x0e);
    if (targetID == 1) {
      displayLogRocket();
    } else if (targetID == 2) {
      displayLogFilling();
    }
  } else if (CMD >= (byte)0x0f && CMD <= (byte)0x1a) {
    displayAck((int)CMD);
  }
}

void updateLogStats(int id) {
  if (last_received_log_id == id) {
    log_packet_loss++;
  }
  if (id == 1) {
    int r_log_interval = millis() - last_r_log_time;
    last_r_log_time = millis();
    r_log_rate = 1000.00 / r_log_interval;
    last_received_log_id = 1;
  } else if (id == 2) {
    int f_log_interval = millis() - last_f_log_time;
    last_f_log_time = millis();
    f_log_rate = 1000.00 / f_log_interval;
    last_received_log_id = 2;
  }

  log_stats.setText("Rocket Log Rate: " + String.format("%.2f", r_log_rate) + "\nFilling Log Rate: " + String.format("%.2f", f_log_rate) + "\nLog Packets Lost: " + log_packet_loss + "\nAck Packets Lost: " + ack_packet_loss);
}

void updateLogStats() {
  log_stats.setText("Rocket Log Rate: " + String.format("%.2f", r_log_rate) + "\nFilling Log Rate: " + String.format("%.2f", f_log_rate) + "\nLog Packets Lost: " + log_packet_loss + "\nAck Packets Lost: " + ack_packet_loss);
}

void displayLogRocket() {
  String state = "\n" + "State: " + state_map_rocket.get(Byte.toUnsignedInt(rx_packet.payload[0]));
  tank_top_temp = (ByteBuffer.wrap(Arrays.copyOfRange(rx_packet.payload, 1, 3))).getShort();
  tank_bot_temp = (ByteBuffer.wrap(Arrays.copyOfRange(rx_packet.payload, 3, 5))).getShort();
  chamber_temp1 = (ByteBuffer.wrap(Arrays.copyOfRange(rx_packet.payload, 5, 7))).getShort(); // launch
  chamber_temp2 = (ByteBuffer.wrap(Arrays.copyOfRange(rx_packet.payload, 7, 9))).getShort(); // launch
  chamber_temp3 = (ByteBuffer.wrap(Arrays.copyOfRange(rx_packet.payload, 9, 11))).getShort(); // launch
  tank_top_press = (ByteBuffer.wrap(Arrays.copyOfRange(rx_packet.payload, 11, 13))).getShort();
  tank_bot_press = (ByteBuffer.wrap(Arrays.copyOfRange(rx_packet.payload, 13, 15))).getShort();
  r_tank_press = (ByteBuffer.wrap(Arrays.copyOfRange(rx_packet.payload, 15, 17))).getShort();
  r_tank_liquid = (ByteBuffer.wrap(Arrays.copyOfRange(rx_packet.payload, 17, 19))).getShort();
  r_bools = rx_packet.payload[19];
  r_weight1 = (ByteBuffer.wrap(Arrays.copyOfRange(rx_packet.payload, 20, 22))).getShort(); // launch
  r_weight2 = (ByteBuffer.wrap(Arrays.copyOfRange(rx_packet.payload, 22, 24))).getShort(); // launch
  r_weight3 = (ByteBuffer.wrap(Arrays.copyOfRange(rx_packet.payload, 24, 26))).getShort(); // launch

  //String ttt = "\n" + "Tank Top Temperature: " + str(tank_top_temp); // fill diagram
  //String tbt = "\n" + "Tank Bottom Temperature: " + str(tank_bot_temp); // fill diagram
  //String ct1 = "\n" + "Chamber Temperature 1: " + str(chamber_temp1);
  //String ct2 = "\n" + "Chamber Temperature 2: " + str(chamber_temp2);
  //String ct3 = "\n" + "Chamber Temperature 3: " + str(chamber_temp3);
  //String ttp = "\n" + "Tank Top Pressure: " + str(tank_top_press); // fill diagram
  //String tbp = "\n" + "Tank Bottom Pressure: " + str(tank_bot_press); // fill diagram
  //String rtp = "\n" + "Tank Pressure: " + str(r_tank_press);
  //String rtl = "\n" + "Tank Liquid: " + str(r_tank_liquid);
  //String w1 = "\n" + "Weight 1: " + str(r_weight1);
  //String w2 = "\n" + "Weight 2: " + str(r_weight2);
  //String w3 = "\n" + "Weight 3: " + str(r_weight3);

  String bools = String.format("%8s", Integer.toBinaryString(r_bools & 0xFF)).replace(' ', '0');
  String log_running = "\nLog Running: " + bools.substring(0, 1);
  String tt_valve = "\nTank Top Valve: " + bools.substring(1, 2);
  String tb_valve = "\nTank Bottom Valve: " + bools.substring(2, 3);
  String tactiles = "\nTactiles: " + bools.substring(3);

  log_display_rocket.setText("Rocket" + state);
  tt_label.setText("Tank Top\nT : " + str(tank_top_temp) + "\nP : " + str(tank_top_press));
  tb_label.setText("Tank Bottom\nT : " + str(tank_bot_temp) + "\nP : " + str(tank_bot_press));
}

void displayLogFilling() {
  String state = "\n" + "State: " + state_map_filling.get(Byte.toUnsignedInt(rx_packet.payload[0]));
  f_tank_press = (ByteBuffer.wrap(Arrays.copyOfRange(rx_packet.payload, 1, 3))).getShort();
  f_tank_liquid = (ByteBuffer.wrap(Arrays.copyOfRange(rx_packet.payload, 3, 5))).getShort();
  he_temp = (ByteBuffer.wrap(Arrays.copyOfRange(rx_packet.payload, 5, 7))).getShort();
  n2o_temp = (ByteBuffer.wrap(Arrays.copyOfRange(rx_packet.payload, 7, 9))).getShort();
  line_temp = (ByteBuffer.wrap(Arrays.copyOfRange(rx_packet.payload, 9, 11))).getShort();
  he_press = (ByteBuffer.wrap(Arrays.copyOfRange(rx_packet.payload, 11, 13))).getShort();
  n2o_press = (ByteBuffer.wrap(Arrays.copyOfRange(rx_packet.payload, 13, 15))).getShort();
  line_press = (ByteBuffer.wrap(Arrays.copyOfRange(rx_packet.payload, 15, 17))).getShort();
  ematch_v = (ByteBuffer.wrap(Arrays.copyOfRange(rx_packet.payload, 17, 19))).getShort();
  f_weight1 = (ByteBuffer.wrap(Arrays.copyOfRange(rx_packet.payload, 19, 21))).getShort();
  f_bools = rx_packet.payload[21];

  //String ftp = "\n" + "Tank Temperature: " + str(f_tank_press);
  //String ftl = "\n" + "Tank Liquid: " + str(f_tank_liquid);
  //String ht = "\n" + "He Temperature: " + str(he_temp); // fill diagram
  //String nt = "\n" + "N2O Temperature: " + str(n2o_temp); // fill diagram
  //String lt = "\n" + "Line Temperature: " + str(line_temp); // fill diagram
  //String hp = "\n" + "He Pressure: " + str(he_press); // fill diagram
  //String np = "\n" + "N2O Pressure: " + str(n2o_press); // fill diagram
  //String lp = "\n" + "Line Pressure: " + str(line_press); // fill diagram
  //String ev = "\n" + "eMatch reading: " + str(ematch_v);
  //String w1 = "\n" + "Weight 1: " + str(f_weight1); // filling graph

  //String bools = String.format("%8s", Integer.toBinaryString(f_bools & 0xFF)).replace(' ', '0');
  //String log_running = "\nLog Running: " + bools.substring(0, 1);
  //String he_valve = "\nHelium Valve: " + bools.substring(1, 2);
  //String n2o_valve = "\nN2O Valve: " + bools.substring(2, 3);
  //String line_valve = "\nLine Valve: " + bools.substring(3, 4);

  log_display_filling.setText("Filling Station" + state);
  he_label.setText("He\nT : " + str(he_temp) + "\nP : " + str(he_press));
  n2o_label.setText("N2O\nT : " + str(n2o_temp) + "\nP : " + str(n2o_press));
  he_label.setText("Line\nT : " + str(line_temp) + "\nP : " + str(line_press));
}

void displayAck(int ackValue) {
  if ((byte) ackValue != (last_cmd_sent + (byte) cmd_size)) {
    ack_packet_loss++;
    updateLogStats();
  }
  last_cmd_sent = 0;
  String ackName;
  switch (ackValue) {
  case 14: // Status Ack
    ackName = "Status";
    break;
  case 15: // Log Ack
    ackName = "Log";
    break;
  case 16: // Abort Ack
    ackName = "Abort";
    break;
  case 17: // Exec Prog Ack
    ackName = "Exec Prog";
    break;
  case 18: // Stop Prog Ack
    ackName = "Stop Prog";
    break;
  case 19: // Fueling Ack
    ackName = "Fueling";
    break;
  case 20: // Manual Ack
    ackName = "Manual";
    break;
  case 21: // Manual Exec Ack
    ackName = "Manual Exec";
    break;
  case 22: // Ready Ack
    ackName = "Ready";
    break;
  case 23: // Arm Ack
    ackName = "Arm";
    break;
  case 24: // Allow Launch Ack
    ackName = "Allow Launch";
    break;
  case 25: // Resume Prog Ack
    ackName = "Resume Prog";
    break;
  case 26: // Fire Pyro Ack
    ackName = "Fire Pyro";
    break;
  default:
    ackName = "Undefined";
    break;
  }
  ack_display.setText("Last Ack Received: \n" + ackName);
}

void send(byte command, byte[] payload) {
  println(command, payload);
  if (targetID == 3) {
    targetID = (byte)0xFF;
  }
  tx_packet = new dataPacket(command, targetID, payload);
  // tx_packet.logPacket(LogEvent.MSG_SENT);
  if (myPort != null) {
    byte[] packet = tx_packet.getPacket();
    println(packet);
    tx_queue.add(packet);
    if (last_cmd_sent != 0) {
      ack_packet_loss++;
    }
    last_cmd_sent = command;
    last_cmd_sent_time = millis();
  } else {
    println("No serial port selected!");
  }
}
