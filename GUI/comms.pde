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
      if (millis() - last_r_ping > heartbeat_timeout || millis() - last_f_ping > heartbeat_timeout) {
        //byte[] ping = {(byte)0x55, (byte)0xFF, (byte) 0x01, (byte) 0x00, (byte) 0x00, (byte) 0x00};
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
  println(rx_packet.getPacket());
  rx_packet.logPacket(LogEvent.MSG_RECEIVED);
  if (rx_packet.command == (byte) 0x00) {
    if (rx_packet.id == (byte) 0x02) {
      updateData(rx_packet);
      updateLogStats(1);
    } else if (rx_packet.id == (byte) 0x01) {
      updateData(rx_packet);
      updateLogStats(2);
    }
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
      if(packet.payloadLength < index) {
      println(ask + "? nao ha mais payload filho");
      return;
      }
      switch (ask) {
      case rocket_flags_state:
        rocket_data.state = packet.payload[index];
        byte rocket_flags = packet.payload[index + 1];
        rocket_data.flash_running = (rocket_flags & (0x01 << 7)) != 0 ? true : false;
        rocket_data.valves.purge_top = (rocket_flags & (0x01 << 6)) != 0 ? true : false;
        rocket_data.valves.purge_bot = (rocket_flags & (0x01 << 5)) != 0 ? true : false;
        rocket_data.valves.chamber = (rocket_flags & (0x01 << 4)) != 0 ? true : false;
        index += 2;
        break;
      case tank_pressures:
        rocket_data.tank.pressure_top = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index, index + 2)).getShort();
        rocket_data.tank.pressure_bot = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 2, index + 4)).getShort();
        index += 4;
        break;
      case tank_temps:
        rocket_data.tank.temp_top = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index, index + 2)).getShort();
        rocket_data.tank.temp_bot = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 2, index + 4)).getShort();
        index += 4;
        break;
      case gps_data:
        rocket_data.gps.satellite_count = packet.payload[index];
        rocket_data.gps.altitude = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 1, index + 3)).getShort();
        rocket_data.gps.latitude = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 3, index + 7)).getInt();
        rocket_data.gps.longitude = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 7, index + 11)).getInt();
        rocket_data.gps.horizontal_velocity = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 11, index + 13)).getShort();
        index += 13;
        break;
      case barometer_altitude:
        rocket_data.barometer_altitude = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index, index + 2)).getShort();
        index += 2;
        break;
      case imu_data:
        rocket_data.imu.accel_x = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index, index + 2)).getShort();
        rocket_data.imu.accel_y = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 2, index + 4)).getShort();
        rocket_data.imu.accel_z = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 4, index + 6)).getShort();
        rocket_data.imu.gyro_x = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 6, index + 8)).getShort();
        rocket_data.imu.gyro_y = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 8, index + 10)).getShort();
        rocket_data.imu.gyro_z = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 10, index + 12)).getShort();
        rocket_data.imu.mag_x = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 12, index + 14)).getShort();
        rocket_data.imu.mag_y = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 14, index + 16)).getShort();
        rocket_data.imu.mag_z = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 16, index + 18)).getShort();
        index += 18; 
        break;
      case kalman_data:
        rocket_data.kalman.pos_x = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index, index + 2)).getShort();
        rocket_data.kalman.pos_y = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 2, index + 4)).getShort();
        rocket_data.kalman.orient_x = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 4, index + 6)).getShort();
        rocket_data.kalman.orient_y = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 6, index + 8)).getShort();
        rocket_data.kalman.orient_z = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 8, index + 10)).getShort();
        rocket_data.kalman.vel_z = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 10, index + 12)).getShort();
        rocket_data.kalman.acel_z = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 12, index + 14)).getShort();
        rocket_data.kalman.altitude = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 14, index + 16)).getShort();
        index += 16;
        println("Kalman Data: ");
        println(rocket_data.kalman.pos_x);
        println(rocket_data.kalman.pos_y);
        println(rocket_data.kalman.orient_x);
        println(rocket_data.kalman.orient_y);
        println(rocket_data.kalman.orient_z);
        println(rocket_data.kalman.vel_z);
        println(rocket_data.kalman.acel_z);
        println(rocket_data.kalman.altitude);
        break;
      case parachutes_ematches:
        rocket_data.parachute.main_ematch = packet.payload[index];
        rocket_data.parachute.drogue_ematch = packet.payload[index + 1];
        index += 2;
        break;
      case fill_station_state:
        filling_data.state = packet.payload[index];
        index += 1;
        break;
      case fill_pressures:
        filling_data.he.pressure = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index, index + 2)).getShort();
        filling_data.n2o.pressure = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 2, index + 4)).getShort();
        filling_data.line.pressure = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 4, index + 6)).getShort();
        index += 6;
        break;
      case fill_temps:
        filling_data.he.temperature = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index, index + 2)).getShort();
        filling_data.n2o.temperature = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 2, index + 4)).getShort();
        filling_data.line.temperature = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index + 4, index + 6)).getShort();
        index += 6;
        break;
      case nitro_loadcell:
        filling_data.n2o.loadcell = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index, index + 2)).getShort();
        index += 2;
        break;
      case ignition_station_state:
        ignition_data.state = packet.payload[index];
        index += 1;
        break;
      case chamber_trigger_temp:
        ignition_data.chamber_trigger_temp = ByteBuffer.wrap(Arrays.copyOfRange(packet.payload, index, index + 2)).getShort();
        index += 2;
        break;
      case main_ematch:
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
      println(file_count);
      println(rx_packet.getPacket());

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

void request_status() {
  if (millis() - last_status_request > status_interval && status_toggle_state == 1) {
    byte oldID = targetID;
    if(last_status_id == 1) {
      targetID = 2;
      last_status_id = 2;
    }
    else {
      targetID = 1;
      last_status_id = 1;
    }
    AskData[] asks = {AskData.rocket_flags_state};
    short askShort = createAskDataMask(asks);
    byte[] asksBytes = ByteBuffer.allocate(2).putShort(askShort).array();
    //byte[] asksBytes = {(byte)0xff, (byte)0xff};
    send((byte)0x00, asksBytes);
    targetID = oldID;
    last_status_request = millis();
  }
}
