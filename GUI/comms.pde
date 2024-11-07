import java.util.BitSet;

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
      if (head != null) {
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
  //print((char)rx_byte);
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
      rx_payload = empty_payload;
      rx_payload_index = 0;
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
  //println();
  //println(rx_packet.getPacket());
  rx_packet.logPacket(LogEvent.MSG_RECEIVED);
  if (rx_packet.command == (byte) 0x00) {
    if (rx_packet.id == (byte) 0x02) {
      updateData(rx_packet);
      updateLogStats(1);
    } else if (rx_packet.id == (byte) 0x01) {
      updateData(rx_packet);
      updateLogStats(2);
    }
  } else if (rx_packet.command == (byte) 0x01) {
    updateData(rx_packet);
    //updateLogStats(1);
  } else if (rx_packet.command == (byte) 0x0e) { // STATUS ACK
    displayAck((int)0x0e);
    updateData(rx_packet);
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

void updateData(dataPacket packet) {
  short asks = (short) ((packet.payload[0] << 8) | (packet.payload[1] & 0xff));
  BitSet bits = BitSet.valueOf(new long[]{asks});
  int index = 2;
  for (int i = 0; i < 16; i++) {
    if (bits.get(i)) {
      AskData ask = AskData.values()[i];
      switch (ask) {
      case rocket_flags_state:
        if (packet.payloadLength < index + 2) {
          print("Index out of bounds: " + ask);
          return;
        }
        last_r_ping = millis();
        rocket_data.state = packet.payload[index];
        byte rocket_flags = packet.payload[index + 1];
        rocket_data.flash_running = (rocket_flags & (0x01 << 7)) != 0 ? true : false;
        rocket_data.valves.purge_top = (rocket_flags & (0x01 << 6)) != 0 ? true : false;
        rocket_data.valves.purge_bot = (rocket_flags & (0x01 << 5)) != 0 ? true : false;
        rocket_data.valves.chamber = (rocket_flags & (0x01 << 4)) != 0 ? true : false;
        index += 2;
        break;
      case tank_pressures:
        if (packet.payloadLength < index + 6) {
          print("Index out of bounds: " + ask);
          return;
        }
        rocket_data.tank.pressure_top = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index, index + 2)).getShort();
        rocket_data.tank.pressure_bot = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 2, index + 4)).getShort();
        rocket_data.chamber_pressure = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 4, index + 6)).getShort();
        index += 6;
        break;
      case tank_temps:
        if (packet.payloadLength < index + 4) {
          print("Index out of bounds: " + ask);
          return;
        }
        rocket_data.tank.temp_top = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index, index + 2)).getShort();
        rocket_data.tank.temp_bot = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 2, index + 4)).getShort();
        index += 4;
        break;
      case gps_data:
        if (packet.payloadLength < index + 13) {
          print("Index out of bounds: " + ask);
          return;
        }
        rocket_data.gps.satellite_count = packet.payload[index];
        rocket_data.gps.altitude = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 1, index + 3)).getShort();
        rocket_data.gps.latitude = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 3, index + 7)).getFloat();
        rocket_data.gps.longitude = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 7, index + 11)).getFloat();
        rocket_data.gps.horizontal_velocity = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 11, index + 13)).getShort();
        index += 13;
        break;
      case barometer_altitude:
        if (packet.payloadLength < index + 2) {
          print("Index out of bounds: " + ask);
          return;
        }
        rocket_data.barometer_altitude = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index, index + 2)).getShort();
        index += 2;
        break;
      case imu_data:
        if (packet.payloadLength < index + 12) {
          print("Index out of bounds: " + ask);
          return;
        }
        rocket_data.imu.accel_x = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index, index + 2)).getShort();
        rocket_data.imu.accel_y = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 2, index + 4)).getShort();
        rocket_data.imu.accel_z = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 4, index + 6)).getShort();
        rocket_data.imu.gyro_x = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 6, index + 8)).getShort();
        rocket_data.imu.gyro_y = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 8, index + 10)).getShort();
        rocket_data.imu.gyro_z = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 10, index + 12)).getShort();
        index += 12;
        break;
      case kalman_data:
        if (packet.payloadLength < index + 16) {
          print("Index out of bounds: " + ask);
          return;
        }
        rocket_data.kalman.altitude = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index, index + 2)).getShort();
        rocket_data.kalman.max_altitude = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 2, index + 4)).getShort();
        rocket_data.kalman.vel_z = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 4, index + 6)).getShort();
        rocket_data.kalman.acel_z = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 6, index + 8)).getShort();
        rocket_data.kalman.q1 = Short.toUnsignedInt(ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 8, index + 10)).getShort());
        rocket_data.kalman.q2 = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 10, index + 12)).getShort();
        rocket_data.kalman.q3 = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 12, index + 14)).getShort();
        rocket_data.kalman.q4 = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 14, index + 16)).getShort();
        index += 16;
        break;
      case parachutes_ematches:
        if (packet.payloadLength < index + 2) {
          print("Index out of bounds: " + ask);
          return;
        }
        rocket_data.parachute.main_ematch = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index, index + 2)).getShort();
        rocket_data.parachute.drogue_ematch = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 2, index + 4)).getShort();
        index += 2;
        break;
      case fill_station_state:
        if (packet.payloadLength < index + 2) {
          print("Index out of bounds: " + ask);
          return;
        }
        last_f_ping = millis();
        filling_data.state = packet.payload[index];
        byte filling_flags = packet.payload[index + 1];
        filling_data.flash_running = (filling_flags & (0x01 << 7)) != 0 ? true : false;
        filling_data.he.valve = (filling_flags & (0x01 << 6)) != 0 ? true : false;
        filling_data.n2o.valve = (filling_flags & (0x01 << 5)) != 0 ? true : false;
        filling_data.line.valve = (filling_flags & (0x01 << 4)) != 0 ? true : false;
        index += 2;
        break;
      case fill_pressures:
        if (packet.payloadLength < index + 6) {
          print("Index out of bounds: " + ask);
          return;
        }
        filling_data.he.pressure = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index, index + 2)).getShort();
        filling_data.n2o.pressure = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 2, index + 4)).getShort();
        filling_data.line.pressure = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 4, index + 6)).getShort();
        index += 6;
        break;
      case fill_temps:
        if (packet.payloadLength < index + 6) {
          print("Index out of bounds: " + ask);
          return;
        }
        filling_data.he.temperature = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index, index + 2)).getShort();
        filling_data.n2o.temperature = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 2, index + 4)).getShort();
        filling_data.line.temperature = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 4, index + 6)).getShort();
        index += 6;
        break;
      case nitro_loadcell:
        if (packet.payloadLength < index + 2) {
          print("Index out of bounds: " + ask);
          return;
        }
        filling_data.n2o.loadcell = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index, index + 2)).getShort();
        index += 2;
        break;
      case ignition_station_state:
        if (packet.payloadLength < index + 1) {
          print("Index out of bounds: " + ask);
          return;
        }
        last_i_ping = millis();
        ignition_data.state = packet.payload[index];
        index += 1;
        break;
      case chamber_trigger_temp:
        if (packet.payloadLength < index + 2) {
          print("Index out of bounds: " + ask);
          return;
        }
        ignition_data.chamber_trigger_temp = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index, index + 2)).getShort();
        index += 2;
        break;
      case main_ematch:
        if (packet.payloadLength < index + 1) {
          print("Index out of bounds: " + ask);
          return;
        }
        ignition_data.main_ematch = packet.payload[index];
        index += 1;
        break;
      default:
        println("Unknown ask: " + ask);
        break;
      }
    }
  }
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
    if (rx_packet.payload[0] == (byte) 0x0d) { // flash ids cmd (2) + man command size (10) + 1
      int file_count = (int) rx_packet.payloadLength - 1;
      //println(file_count);
      //println(rx_packet.getPacket());

      String id = str((ByteBuffer.wrap(Arrays.copyOfRange(rx_packet.payload, file_count - 1, file_count + 1))).getShort());
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
    cp5.getController("Select ID").setValue(0);
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
  //println(command, payload);
  if (targetID == 4) {
    targetID = (byte)0xFF;
  }
  tx_packet = new dataPacket(command, targetID, payload);
  tx_packet.logPacket(LogEvent.MSG_SENT);
  if (myPort != null) {
    //byte[] packet = tx_packet.getPacket();
    //for(byte b : packet) {
    //println(hex(b));
    //}
    //println();
    //println(tx_packet.getPacket());
    tx_queue.add(tx_packet);
  } else {
    println("No serial port selected!");
  }
}

//float calc_n2o_loss() {
//  if (tank_mol_loss > he_mol) {
//    return (((tank_mol_loss - he_mol) * .1) * 44.012);
//  } else {
//    return 0.0;
//  }
//}

short createAskDataMask(AskData[] asks) {
  short mask = 0;
  for (AskData ask : asks) {
    mask |= (1 << ask.ordinal());
  }
  return mask;
}

void auto_status() {
  if (millis() - last_status_time > status_interval && status_toggle_state == 1) {
    byte oldID = targetID;
    if (cp5.getTab("default").isActive()) {
      if (last_status_id == 1) {
        targetID = 2;
        last_status_id = 2;
      } else if (last_status_id == 2) {
        targetID = 3;
        last_status_id = 3;
      } else {
        targetID = 1;
        last_status_id = 1;
      }
    } else if (cp5.getTab("filling").isActive()) {
      if (last_status_id == 1) {
        targetID = 2;
        last_status_id = 2;
      } else {
        targetID = 1;
        last_status_id = 1;
      }
    } else if (cp5.getTab("launch").isActive()) {
      if (last_status_id == 1) {
        targetID = 3;
        last_status_id = 3;
      } else {
        targetID = 1;
        last_status_id = 1;
      }
    }
    request_status();
    last_status_time = millis();
    targetID = oldID;
  }
}

void request_status() {
  AskData[] asks = {};
  if (targetID == 1) { // roket
    if (cp5.getTab("default").isActive()) {
      asks = rocket_data.man_ask;
    } else if (cp5.getTab("filling").isActive()) {
      asks = rocket_data.fill_ask;
    } else if (cp5.getTab("launch").isActive()) {
      asks = rocket_data.launch_ask;
    }
  } else if (targetID == 2) { // filing
    if (cp5.getTab("default").isActive()) {
      asks = filling_data.man_ask;
    } else if (cp5.getTab("filling").isActive()) {
      asks = filling_data.fill_ask;
    } else if (cp5.getTab("launch").isActive()) {
      asks = filling_data.launch_ask;
    }
  } else if (targetID == 3) { // ignixon
    if (cp5.getTab("default").isActive()) {
      asks = ignition_data.man_ask;
    } else if (cp5.getTab("filling").isActive()) {
      asks = ignition_data.fill_ask;
    } else if (cp5.getTab("launch").isActive()) {
      asks = ignition_data.launch_ask;
    }
  }
  if (asks.length == 0) {
    print("No Asks");
    return;
  }
  short askShort = createAskDataMask(asks);
  byte[] asksBytes = ByteBuffer.allocate(2).putShort(askShort).array();
  send((byte)0x00, asksBytes);
}
