List<String> man_commands = Arrays.asList("Flash Log Start", "Flash Log Stop", "Flash IDs", "Loadcell Calibrate", "Loadcell Tare", "Tank Tare");
List<String> valves = Arrays.asList("VPU Valve", "Engine Valve", "Chamber Valve", "He Valve", "N2O Valve", "Line Valve");

void maps() {
    boolean[] _bl1 = {true, true, false};
  prog_args.put("Safety Pressure", _bl1);
  boolean[] _bl2 = {true, false, false};
  prog_args.put("Purge Pressure", _bl2);
  boolean[] _bl3 = {false, false, true};
  prog_args.put("Purge Liquid", _bl3);
  boolean[] _bl4 = {true, false, false};
  prog_args.put("Fill He", _bl4);
  boolean[] _bl5 = {false, true, false};
  prog_args.put("Fill N2O", _bl5);
  boolean[] _bl6 = {true, false, false};
  prog_args.put("Purge Line", _bl6);

  state_map_rocket.put(0, "IDLE");
  state_map_rocket.put(1, "FUELING");
  state_map_rocket.put(2, "MANUAL");
  state_map_rocket.put(3, "SAFETY_PRESSURE");
  state_map_rocket.put(4, "PURGE_PRESSURE");
  state_map_rocket.put(5, "PURGE_LIQUID");
  state_map_rocket.put(6, "SAFETY_PRESSURE_ACTIVE");
  state_map_rocket.put(7, "READY");
  state_map_rocket.put(8, "ARMED");
  state_map_rocket.put(9, "LAUNCH");
  state_map_rocket.put(10, "ABORT");
  state_map_rocket.put(11, "IMU_CALIB");

  state_map_filling.put(0, "IDLE");
  state_map_filling.put(1, "FUELING");
  state_map_filling.put(2, "MANUEL");
  state_map_filling.put(3, "FILL_He");
  state_map_filling.put(4, "FILL_N2O");
  state_map_filling.put(5, "PURGE_LINE");
  state_map_filling.put(6, "SAFETY");
  state_map_filling.put(7, "ABORT");
  state_map_filling.put(8, "READY");
  state_map_filling.put(9, "ARMED");
  state_map_filling.put(10, "FIRE");
  state_map_filling.put(11, "LAUNCH");

  man_commands_map.put("Flash Log Start", (byte) 0);
  man_commands_map.put("Flash Log Stop", (byte) 1);
  man_commands_map.put("Flash IDs", (byte) 2);
  man_commands_map.put("Loadcell Calibrate", (byte) 7);
  man_commands_map.put("Loadcell Tare", (byte) 8);
  man_commands_map.put("Tank Tare", (byte) 9);

  valve_toggle_map.put(tt_toggle, (byte) 0x00);
  valve_toggle_map.put(tb_toggle, (byte) 0x01);
  valve_toggle_map.put(chamber_toggle, (byte) 0x02);
  valve_toggle_map.put(he_toggle, (byte) 0x03);
  valve_toggle_map.put(n2o_toggle, (byte) 0x04);
  valve_toggle_map.put(line_toggle, (byte) 0x05);

  command_names.put((byte) 0x00, "Status");
  command_names.put((byte) 0x02, "Abort");
  command_names.put((byte) 0x03, "Exec Prog");
  command_names.put((byte) 0x04, "Stop");
  command_names.put((byte) 0x05, "Start Filling");
  command_names.put((byte) 0x06, "Manual");
  command_names.put((byte) 0x07, "Manual Exec");
  command_names.put((byte) 0x08, "Ready");
  command_names.put((byte) 0x09, "Arm");
  command_names.put((byte) 0x0a, "Allow Launch");
  command_names.put((byte) 0x0b, "Resume");
  command_names.put((byte) 0x0c, "Fire");
}
