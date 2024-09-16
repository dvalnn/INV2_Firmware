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
    dataPacket head = tx_queue.peek();
    if (millis() - last_cmd_sent_time > packet_loss_timeout) {
      if (millis() - last_r_ping > heartbeat_timeout || millis() - last_f_ping > heartbeat_timeout) {
        byte[] ping = {(byte)0x55, (byte)0xFF, (byte) 0x01, (byte) 0x00, (byte) 0x00, (byte) 0x00};
        myPort.write(ping);
        last_r_ping = millis();
        last_f_ping = millis();
      }
      if (head != null) {
        if (head.id == (byte) 0x01) {
          last_r_ping = millis();
        } else if (head.id == (byte) 0x02) {
          last_f_ping = millis();
        }
        myPort.write(head.getPacket());
        last_cmd_sent_time = millis();
        if (last_cmd_sent != (byte)0xff) {
          ack_packet_loss++;
        }
        last_cmd_sent = head.command;
        if (history_deque.size() == history_capacity) {
          history_deque.removeFirst();
        }
        history_deque.addLast("CMD -> " + command_names.get(last_cmd_sent));
        String history_string = "";
        for (String element : history_deque) {
          history_string += element + "\n";
        }
        history.setText("History: \n" + history_string);
        tx_queue.remove();
      }
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
  if (CMD == (byte) 0x00) {
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
  r_chamber_press = (ByteBuffer.wrap(Arrays.copyOfRange(rx_packet.payload, 26, 28))).getShort();

  liquid_height = (ByteBuffer.wrap(Arrays.copyOfRange(rx_packet.payload, 28, 30))).getShort();
  liquid_volume = (ByteBuffer.wrap(Arrays.copyOfRange(rx_packet.payload, 30, 32))).getShort();
  liquid_mass = (ByteBuffer.wrap(Arrays.copyOfRange(rx_packet.payload, 32, 34))).getShort();
  liquid_mass2 = (ByteBuffer.wrap(Arrays.copyOfRange(rx_packet.payload, 34, 36))).getShort();


  chamber_temps_label.setText("Chamber Temperatures\n1: " + df.format(chamber_temp1 * .1) + "\n2: " + df.format(chamber_temp2 * .1) + "\n3: " + df.format(chamber_temp3 * .1) + "\nChamber Pressure: " + df.format(r_chamber_press * .01));

  String bools = String.format("%8s", Integer.toBinaryString(r_bools & 0xFF)).replace(' ', '0');
  int log_running = Integer.parseInt(bools.substring(0, 1));
  if (log_running == 1) {
    r_flash_log = true;
  } else {
    r_flash_log = false;
  }

  log_display_rocket.setText("Rocket" + state);
  tt_label.setText("Tank Top\nT : " + String.format("%.2f", tank_top_temp * .1) + "\nP : " + String.format("%.2f", tank_top_press * .01));
  tb_label.setText("Tank Bottom\nT : " + String.format("%.2f", tank_bot_temp * .1) + "\nP : " + String.format("%.2f", tank_bot_press * .01));
  tl_label.setText("Liquid: " + String.format("%.2f", (100 - r_tank_liquid * .01)) + "%\n\n\n" + String.format("%.2f", liquid_height * .01) + "m\n\n\n" + String.format("%.2f", liquid_volume * .001) + "m3\n\n\n" + String.format("%.2f", liquid_mass * .01) + "kg\n\n\n" + String.format("%.2f", liquid_mass2 * .01) + "kg");
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

  String bools = String.format("%8s", Integer.toBinaryString(f_bools & 0xFF)).replace(' ', '0');
  int log_running = Integer.parseInt(bools.substring(0, 1));
  if (log_running == 1) {
    f_flash_log = true;
  } else {
    f_flash_log = false;
  }

  log_display_filling.setText("Filling Station" + state);
  he_label.setText("He\nT : " + String.format("%.2f", he_temp * .1) + "\nP : " + String.format("%.2f", he_press * .01));
  n2o_label.setText("N2O\nT : " + String.format("%.2f", n2o_temp * .1) + "\nP : " + String.format("%.2f", n2o_press * .01));
  line_label.setText("Line\nT : " + String.format("%.2f", line_temp * .1) + "\nP : " + String.format("%.2f", line_press * .01));
  ematch_label.setText("E-Match value : " + ematch_v);
}

void displayAck(int ackValue) {
  if ((byte) ackValue != (last_cmd_sent + (byte) cmd_size)) {
    ack_packet_loss++;
    updateLogStats();
  }
  last_cmd_sent = (byte) 0xff;
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
    if (rx_packet.payload[0] == (byte) 0x0c) { // flash ids cmd (2) + man command size (9) + 1
      int file_count = (int) rx_packet.payloadLength - 1;
      String id = str(rx_packet.payload[(ByteBuffer.wrap(Arrays.copyOfRange(rx_packet.payload, file_count - 1, file_count + 1))).getShort()]);
      ackName = "Flash Log ID: " + id;
    }
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
    status_toggle.setState(true);
    cp5.getController("Select ID").setValue(1);
    break;
  default:
    ackName = "Undefined";
    break;
  }
  ack_display.setText("Last Ack Received: \n" + ackName);
  if (history_deque.size() == history_capacity) {
    history_deque.removeFirst();
  }
  history_deque.addLast("Ack <- " + ackName);
  String history_string = "";
  for (String element : history_deque) {
    history_string += element + "\n";
  }
  history.setText("History: \n" + history_string);
}

void send(byte command, byte[] payload) {
  if (targetID == 0) {
    print("No ID selected\n");
    return;
  }
  println(command, payload);
  if (targetID == 3) {
    targetID = (byte)0xFF;
  }
  tx_packet = new dataPacket(command, targetID, payload);
  tx_packet.logPacket(LogEvent.MSG_SENT);
  if (myPort != null) {
    byte[] packet = tx_packet.getPacket();
    println(packet);
    tx_queue.add(tx_packet);
  } else {
    println("No serial port selected!");
  }
}
