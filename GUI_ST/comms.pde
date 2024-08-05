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
    currentParseState = ParseState.CRC2;
    break;
  case CRC2:
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
    if (ID == (byte) 0x02) {
      displayLogRocket();
      updateLogStats(1);
    } else if (ID == (byte) 0x01) {
      displayLogFilling();
      updateLogStats(2);
    }
  } else if (CMD == (byte) 0x0e) { // STATUS ACK
    if (targetID == 1) {
      displayStatusRocket();
    } else if (targetID == 2) {
      displayStatusFilling();
    }
  } else if (CMD >= (byte)0x0f && CMD <= (byte)0x1a) {
    displayAck((int)CMD);
  }
}

void updateLogStats(int id) {
  if (id == 1) {
    int r_log_interval = millis() - last_r_log_time;
    last_r_log_time = millis();
    r_log_rate = 1000.00 / r_log_interval;
  } else if (id == 2) {
    int f_log_interval = millis() - last_f_log_time;
    last_f_log_time = millis();
    f_log_rate = 1000.00 / f_log_interval;
  }
  log_stats.setText("Rocket Log Rate: " + String.format("%.2f", r_log_rate) + "\n" + "Filling Log Rate: " + String.format("%.2f", f_log_rate));
}
void displayLogRocket() {
  String state = "\n" + "State: " + state_map_rocket.get(Byte.toUnsignedInt(rx_packet.payload[0]));
  tank_top_temp = float((Byte.toUnsignedInt(rx_packet.payload[1]) << 8) | Byte.toUnsignedInt(rx_packet.payload[2]));
  tank_bot_temp = float((Byte.toUnsignedInt(rx_packet.payload[3]) << 8) | Byte.toUnsignedInt(rx_packet.payload[4]));
  chamber_temp1 = float((Byte.toUnsignedInt(rx_packet.payload[5]) << 8) | Byte.toUnsignedInt(rx_packet.payload[6]));
  chamber_temp2 = float((Byte.toUnsignedInt(rx_packet.payload[7]) << 8) | Byte.toUnsignedInt(rx_packet.payload[8]));
  chamber_temp3 = float((Byte.toUnsignedInt(rx_packet.payload[9]) << 8) | Byte.toUnsignedInt(rx_packet.payload[10]));
  tank_top_press = float((Byte.toUnsignedInt(rx_packet.payload[11]) << 8) | Byte.toUnsignedInt(rx_packet.payload[12]));
  tank_bot_press = float((Byte.toUnsignedInt(rx_packet.payload[13]) << 8) | Byte.toUnsignedInt(rx_packet.payload[12]));
  r_tank_press = float((Byte.toUnsignedInt(rx_packet.payload[15]) << 8) | Byte.toUnsignedInt(rx_packet.payload[16]));
  r_tank_liquid = float((Byte.toUnsignedInt(rx_packet.payload[17]) << 8) | Byte.toUnsignedInt(rx_packet.payload[18]));
  tank_tactile = rx_packet.payload[19];

  String ttt = "\n" + "Tank Top Temperature: " + str(tank_top_temp);
  String tbt = "\n" + "Tank Bottom Temperature: " + str(tank_bot_temp);
  String ct1 = "\n" + "Chamber Temperature 1: " + str(chamber_temp1);
  String ct2 = "\n" + "Chamber Temperature 2: " + str(chamber_temp2);
  String ct3 = "\n" + "Chamber Temperature 3: " + str(chamber_temp3);
  String ttp = "\n" + "Tank Top Pressure: " + str(tank_top_press);
  String tbp = "\n" + "Tank Bottom Pressure: " + str(tank_bot_press);
  String rtp = "\n" + "Tank Pressure: " + str(r_tank_press);
  String rtl = "\n" + "Tank Liquid: " + str(r_tank_liquid);
  String tt = "\n" + "Tank Tactile: " + String.format("%8s", Integer.toBinaryString(tank_tactile & 0xFF)).replace(' ', '0');

  log_display_rocket.setText("Rocket\n" + state + ttt + tbt + ct1 + ct2 + ct3 + ttp + tbp + rtp + rtl + tt);
}

void displayLogFilling() {
  String state = "\n" + "State: " + state_map_filling.get(Byte.toUnsignedInt(rx_packet.payload[0]));
  f_tank_press = float((Byte.toUnsignedInt(rx_packet.payload[1]) << 8) | Byte.toUnsignedInt(rx_packet.payload[2]));
  f_tank_liquid = float((Byte.toUnsignedInt(rx_packet.payload[3]) << 8) | Byte.toUnsignedInt(rx_packet.payload[4]));
  he_temp = float((Byte.toUnsignedInt(rx_packet.payload[5]) << 8) | Byte.toUnsignedInt(rx_packet.payload[6]));
  n2o_temp = float((Byte.toUnsignedInt(rx_packet.payload[7]) << 8) | Byte.toUnsignedInt(rx_packet.payload[8]));
  line_temp = float((Byte.toUnsignedInt(rx_packet.payload[9]) << 8) | Byte.toUnsignedInt(rx_packet.payload[10]));
  he_press = float((Byte.toUnsignedInt(rx_packet.payload[11]) << 8) | Byte.toUnsignedInt(rx_packet.payload[12]));
  n2o_press = float((Byte.toUnsignedInt(rx_packet.payload[13]) << 8) | Byte.toUnsignedInt(rx_packet.payload[12]));
  line_press = float((Byte.toUnsignedInt(rx_packet.payload[15]) << 8) | Byte.toUnsignedInt(rx_packet.payload[16]));
  ematch_v = float((Byte.toUnsignedInt(rx_packet.payload[17]) << 8) | Byte.toUnsignedInt(rx_packet.payload[18]));
  weight1 = float(((Byte.toUnsignedInt(rx_packet.payload[19]) << 24) | Byte.toUnsignedInt(rx_packet.payload[20]) << 16) | ((Byte.toUnsignedInt(rx_packet.payload[21]) << 8) | Byte.toUnsignedInt(rx_packet.payload[22])));

  String ftp = "\n" + "Tank Temperature: " + str(f_tank_press);
  String ftl = "\n" + "Tank Liquid: " + str(f_tank_liquid);
  String ht = "\n" + "He Temperature: " + str(he_temp);
  String nt = "\n" + "N2O Temperature: " + str(n2o_temp);
  String lt = "\n" + "Line Temperature: " + str(line_temp);
  String hp = "\n" + "He Pressure: " + str(he_press);
  String np = "\n" + "N2O Pressure: " + str(n2o_press);
  String lp = "\n" + "Line Pressure: " + str(line_press);
  String ev = "\n" + "eMatch reading: " + str(ematch_v);
  String w1 = "\n" + "Weight 1: " + str(weight1);

  log_display_filling.setText("Filling Station\n" + state + ftp + ftl + ht + nt + lt + hp + np + lp + ev + w1);
}

void displayStatusRocket() {
  String state = "\n" + "State: " + state_map_rocket.get(Byte.toUnsignedInt(rx_packet.payload[0]));
  tank_top_temp = float((Byte.toUnsignedInt(rx_packet.payload[1]) << 8) | Byte.toUnsignedInt(rx_packet.payload[2]));
  tank_bot_temp = float((Byte.toUnsignedInt(rx_packet.payload[3]) << 8) | Byte.toUnsignedInt(rx_packet.payload[4]));
  chamber_temp1 = float((Byte.toUnsignedInt(rx_packet.payload[5]) << 8) | Byte.toUnsignedInt(rx_packet.payload[6]));
  chamber_temp2 = float((Byte.toUnsignedInt(rx_packet.payload[7]) << 8) | Byte.toUnsignedInt(rx_packet.payload[8]));
  chamber_temp3 = float((Byte.toUnsignedInt(rx_packet.payload[9]) << 8) | Byte.toUnsignedInt(rx_packet.payload[10]));
  tank_top_press = float((Byte.toUnsignedInt(rx_packet.payload[11]) << 8) | Byte.toUnsignedInt(rx_packet.payload[12]));
  tank_bot_press = float((Byte.toUnsignedInt(rx_packet.payload[13]) << 8) | Byte.toUnsignedInt(rx_packet.payload[12]));
  r_tank_press = float((Byte.toUnsignedInt(rx_packet.payload[15]) << 8) | Byte.toUnsignedInt(rx_packet.payload[16]));
  r_tank_liquid = float((Byte.toUnsignedInt(rx_packet.payload[17]) << 8) | Byte.toUnsignedInt(rx_packet.payload[18]));
  tank_tactile = rx_packet.payload[19];

  String ttt = "\n" + "Tank Top Temperature: " + str(tank_top_temp);
  String tbt = "\n" + "Tank Bottom Temperature: " + str(tank_bot_temp);
  String ct1 = "\n" + "Chamber Temperature 1: " + str(chamber_temp1);
  String ct2 = "\n" + "Chamber Temperature 2: " + str(chamber_temp2);
  String ct3 = "\n" + "Chamber Temperature 3: " + str(chamber_temp3);
  String ttp = "\n" + "Tank Top Pressure: " + str(tank_top_press);
  String tbp = "\n" + "Tank Bottom Pressure: " + str(tank_bot_press);
  String rtp = "\n" + "Tank Pressure: " + str(r_tank_press);
  String rtl = "\n" + "Tank Liquid: " + str(r_tank_liquid);
  String tt = "\n" + "Tank Tactile: " + String.format("%8s", Integer.toBinaryString(tank_tactile & 0xFF)).replace(' ', '0');

  log_display_rocket.setText("Rocket\n" + state + ttp + tbt + ct1 + ct2 + ct3 + ttp + tbp + rtp + rtl + tt);
}

void displayStatusFilling() {
  String state = "\n" + "State: " + state_map_filling.get(Byte.toUnsignedInt(rx_packet.payload[0]));
  f_tank_press = float((Byte.toUnsignedInt(rx_packet.payload[1]) << 8) | Byte.toUnsignedInt(rx_packet.payload[2]));
  f_tank_liquid = float((Byte.toUnsignedInt(rx_packet.payload[3]) << 8) | Byte.toUnsignedInt(rx_packet.payload[4]));
  he_temp = float((Byte.toUnsignedInt(rx_packet.payload[5]) << 8) | Byte.toUnsignedInt(rx_packet.payload[6]));
  n2o_temp = float((Byte.toUnsignedInt(rx_packet.payload[7]) << 8) | Byte.toUnsignedInt(rx_packet.payload[8]));
  line_temp = float((Byte.toUnsignedInt(rx_packet.payload[9]) << 8) | Byte.toUnsignedInt(rx_packet.payload[10]));
  he_press = float((Byte.toUnsignedInt(rx_packet.payload[11]) << 8) | Byte.toUnsignedInt(rx_packet.payload[12]));
  n2o_press = float((Byte.toUnsignedInt(rx_packet.payload[13]) << 8) | Byte.toUnsignedInt(rx_packet.payload[12]));
  line_press = float((Byte.toUnsignedInt(rx_packet.payload[15]) << 8) | Byte.toUnsignedInt(rx_packet.payload[16]));
  ematch_v = float((Byte.toUnsignedInt(rx_packet.payload[17]) << 8) | Byte.toUnsignedInt(rx_packet.payload[18]));
  weight1 = float(((Byte.toUnsignedInt(rx_packet.payload[19]) << 24) | Byte.toUnsignedInt(rx_packet.payload[20]) << 16) | ((Byte.toUnsignedInt(rx_packet.payload[21]) << 8) | Byte.toUnsignedInt(rx_packet.payload[22])));

  String ftp = "\n" + "Tank Temperature: " + str(f_tank_press);
  String ftl = "\n" + "Tank Liquid: " + str(f_tank_liquid);
  String ht = "\n" + "He Temperature: " + str(he_temp);
  String nt = "\n" + "N2O Temperature: " + str(n2o_temp);
  String lt = "\n" + "Line Temperature: " + str(line_temp);
  String hp = "\n" + "He Pressure: " + str(he_press);
  String np = "\n" + "N2O Pressure: " + str(n2o_press);
  String lp = "\n" + "Line Pressure: " + str(line_press);
  String ev = "\n" + "eMatch reading: " + str(ematch_v);
  String w1 = "\n" + "Weight 1: " + str(weight1);

  log_display_filling.setText("Filling Station\n" + state + ftp + ftl + ht + nt + lt + hp + np + lp + ev + w1);
}

void displayAck(int ackValue) {
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
