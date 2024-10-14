void updateControllersData() {
  if (millis() - last_f_ping > doubt_timeout) {
    log_display_filling.setText("Filling: " + filling_data.getState() + "?");
  } else {
    log_display_filling.setText("Filling: " + filling_data.getState());
  }
  String he_data = "He\n" +
    "P: " + df.format(float(filling_data.he.pressure) * .01) + " bar\n" +
    "T: " + df.format(float(filling_data.he.temperature) * .1) + " ºC";
  he_label.setText(he_data);
  String n2o_data = "N2O\n" +
    "P: " + df.format(float(filling_data.n2o.pressure) * .01) + " bar\n" +
    "T: " + df.format(float(filling_data.n2o.temperature)* .1) + " ºC\n" +
    "W: " + df.format(float(filling_data.n2o.loadcell) * .1) + " kg";
  n2o_label.setText(n2o_data);
  String line_data = "Line\n" +
    "P: " + df.format(float(filling_data.line.pressure) * .01) + " bar\n" +
    "T: " + df.format(float(filling_data.line.temperature) * .1) + " ºC";
  line_label.setText(line_data);

  if (millis() - last_r_ping > doubt_timeout) {
    log_display_rocket.setText("Rocket: " + rocket_data.getState() + "?");
  } else {
    log_display_rocket.setText("Rocket: " + rocket_data.getState());
  }
  String debug = cp5.getTab("default").isActive() ? "DEBUG: ttp - " + df.format((float)filling_data.rs_press * .01) + " bar" : "";
  String tt_data = "Tank Top\n" +
    "P: " + df.format(float(rocket_data.tank.pressure_top) * .01) + " bar\n" +
    "T: " + df.format(float(rocket_data.tank.temp_top) * .1) + " ºC" + "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"+debug;
  tt_label.setText(tt_data);
  String tb_data = "Tank Bottom\n" +
    "P: " + df.format(float(rocket_data.tank.pressure_bot) * .01) + " bar\n" +
    "T: " + df.format(float(rocket_data.tank.temp_bot) * .1) + " ºC";
  tb_label.setText(tb_data);

  if (millis() - last_i_ping > doubt_timeout) {
    log_display_ignition.setText("Ignition: " + ignition_data.getState() + "?");
  } else {
    log_display_ignition.setText("Ignition: " + ignition_data.getState());
  }
  String chamber_data = "Chamber\n" +
    "P: " + df.format(float(rocket_data.chamber_pressure) * .01) + " bar\n" +
    "T: " + df.format(float(ignition_data.chamber_trigger_temp) * .1) + " ºC";
  chamber_label.setText(chamber_data);

  // other
  ematch_label.setText("Launch e-Match: " + df.format(ignition_data.main_ematch)
    + "\nDrogue e-Match: " + df.format(rocket_data.parachute.drogue_ematch)
    + "\nMain e-Match: " + df.format(rocket_data.parachute.main_ematch));

  String gps_data = "GPS          " +
    "Sat: " + str(int(rocket_data.gps.satellite_count)) +
    "\nLat: " + rocket_data.gps.latitude + "    " +
    "Alt: " + df.format(float(rocket_data.gps.altitude)) + "\n" +
    "Lon: " + rocket_data.gps.longitude + "    " +
    "hVel: " + df.format(float(rocket_data.gps.horizontal_velocity) * .1) + "\n";

  gps_label.setText(gps_data);

  String bar_data = "Barometer\n\n" +
    "Alt: " + df.format(float(rocket_data.barometer_altitude));
  bar_label.setText(bar_data);

  String imu_data = "IMU\n" +
    "\nAx: " + df.format(float(rocket_data.imu.accel_x) * .1) + "\n" +
    "Ay: " + df.format(float(rocket_data.imu.accel_y) * .1) + "\n" +
    "Az: " + df.format(float(rocket_data.imu.accel_z) * .1) + "\n" +
    "\nGx: " + df.format(float(rocket_data.imu.gyro_x) * .1) + "\n" +
    "Gy: " + df.format(float(rocket_data.imu.gyro_y) * .1) + "\n" +
    "Gz: " + df.format(float(rocket_data.imu.gyro_z) * .1);
  imu_label.setText(imu_data);

  String kalman_data = "Kalman\n\n" +
    "Alt: " + df.format(float(rocket_data.kalman.altitude)*.1) + "\n" +
    "Max Alt: " + df.format(float(rocket_data.kalman.altitude)*.1) + "\n" +
    "Vel: " + df.format(float(rocket_data.kalman.vel_z)*.1) + "\n" +
    "Acel: " + df.format(float(rocket_data.kalman.acel_z)*.1) + "\n" +
    "q1: " + df.format(float(rocket_data.kalman.q1)) + "\n" +
    "q2: " + df.format(float(rocket_data.kalman.q2)) + "\n" +
    "q3: " + df.format(float(rocket_data.kalman.q3)) + "\n" +
    "q4: " + df.format(float(rocket_data.kalman.q4));
  kalman_label.setText(kalman_data);
}

void updateControllersPos(String tab) {
  if (tab == "default") {
    // fill
    he_label.setPosition(width*.25, height*.05);
    n2o_label.setPosition(width*.35, height*.05);
    line_label.setPosition(width*.45, height*.05);
    //rocket
    tt_label.setPosition(width*.25, height*.2);
    tb_label.setPosition(width*.35, height*.2);
    //ignition
    chamber_label.setPosition(width*.45, height*.2);

    //fill
    he_toggle.setPosition(width*.25, height*.15);
    n2o_toggle.setPosition(width*.35, height*.15);
    line_toggle.setPosition(width*.45, height*.15);
    // rocket
    tt_toggle.setSize((int)(width*toggle_width), (int)(height*toggle_height));
    tb_toggle.setSize((int)(width*toggle_width), (int)(height*toggle_height));
    chamber_toggle.setSize((int)(width*toggle_width), (int)(height*toggle_height));
    tt_toggle.setPosition(width*.25, height*.28);
    tb_toggle.setPosition(width*.35, height*.28);
    chamber_toggle.setPosition(width*.45, height*.28);

    //other
    ematch_label.setPosition(width*.55, height*.05);
    gps_label.setPosition(width*.55, height*.15);
    bar_label.setPosition(width*.45, height*.32);
    imu_label.setPosition(width*.25, height*.32);
    kalman_label.setPosition(width*.35, height*.32);
  } else if (tab == "filling") {
    he_label.setPosition(displayWidth*.5, displayHeight*.47);
    n2o_label.setPosition(displayWidth*.5, displayHeight*.69);
    line_label.setPosition(displayWidth*.4, displayHeight*.48);
    tt_label.setPosition(displayWidth*.31, displayHeight*.35);
    tb_label.setPosition(displayWidth*.39, displayHeight*.83);
    chamber_label.setPosition(width*.25, height*.89);

    he_toggle.setPosition(width*.5, height*.55);
    n2o_toggle.setPosition(width*.5, height*.65);
    line_toggle.setPosition(width*.385, height*.62);

    tt_toggle.setSize((int)(width*toggle_width), (int)(height*toggle_height));
    tb_toggle.setSize((int)(width*toggle_width), (int)(height*toggle_height));
    chamber_toggle.setSize((int)(width*toggle_width), (int)(height*toggle_height));
    tt_toggle.setPosition(width*.265, height*.41);
    tb_toggle.setPosition(width*.32, height*.84);
    chamber_toggle.setPosition(width*.265, height*.8);
  } else if (tab == "launch") {
    ematch_label.setPosition(displayWidth*.4, displayHeight*.05);
    chamber_label.setPosition(displayWidth*.55, displayHeight*.05);
    gps_label.setPosition(width*.23, height*.4);
    kalman_label.setPosition(width*.25, height*.5);

    tt_toggle.setPosition(width*.095, height*.07);
    tb_toggle.setPosition(width*.14, height*.3);
    chamber_toggle.setPosition(width*.095, height*.45);

    tt_toggle.setSize((int)(width*.02), (int)(height*.02));
    tb_toggle.setSize((int)(width*.02), (int)(height*.02));
    chamber_toggle.setSize((int)(width*.02), (int)(height*.02));
  }
}
