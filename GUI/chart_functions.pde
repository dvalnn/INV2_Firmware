void setupCharts() {
  fillingChart.addDataSet("Pressure");
  fillingChart.addDataSet("Temperature");
  fillingChart.addDataSet("Weight");

  launchChart.addDataSet("Altitude");
  launchChart.addDataSet("Velocity");
  launchChart.addDataSet("Acceleration");


  fillingChart.setColors("Pressure", color(0, 255, 0));
  fillingChart.setColors("Temperature", color(255, 150, 255));
  fillingChart.setColors("Weight", color(0, 255, 255));

  launchChart.setColors("Altitude", color(0, 255, 0));
  launchChart.setColors("Velocity", color(255, 255, 0));
  launchChart.setColors("Acceleration", color(255, 150, 150));

  pressureLabel = cp5.addLabel("Pressure: ")
    .setColor(color(0, 255, 0))
    .setFont(font)
    .moveTo("filling")
    .setPosition(displayWidth*.63, displayHeight*.16)
    .setSize((int)(displayWidth*.1), (int)(displayHeight*.1))
    ;
  temperatureLabel = cp5.addLabel("Temperature: ")
    .setColor(color(255, 150, 255))
    .setFont(font)
    .moveTo("filling")
    .setPosition(displayWidth*.63, displayHeight*.24)
    .setSize((int)(displayWidth*.1), (int)(displayHeight*.1))
    ;
  weightLabel = cp5.addLabel("Weight: ")
    .setColor(color(0, 255, 255))
    .setFont(font)
    .moveTo("filling")
    .setPosition(displayWidth*.63, displayHeight*.28)
    .setSize((int)(displayWidth*.1), (int)(displayHeight*.1))
    ;

  altitudeLabel = cp5.addLabel("Altitude: ")
    .setColor(color(0, 255, 0))
    .setFont(font)
    .moveTo("launch")
    .setPosition(displayWidth*.63, displayHeight*.16)
    .setSize((int)(displayWidth*.1), (int)(displayHeight*.1))
    ;
  velocityLabel = cp5.addLabel("Velocity: ")
    .setColor(color(255, 255, 0))
    .setFont(font)
    .moveTo("launch")
    .setPosition(displayWidth*.63, displayHeight*.19)
    .setSize((int)(displayWidth*.1), (int)(displayHeight*.1))
    ;
  accelerationLabel = cp5.addLabel("Acceleration: ")
    .setColor(color(255, 150, 150))
    .setFont(font)
    .moveTo("launch")
    .setPosition(displayWidth*.63, displayHeight*.22)
    .setSize((int)(displayWidth*.1), (int)(displayHeight*.1))
    ;
}

void updateCharts() {
  // filling chart
  float fp = float(rocket_data.tank.pressure_top) * .01,
    ft = float(rocket_data.tank.temp_bot) * .1,
    fw = float(filling_data.n2o.loadcell);

  fillingChart.addData("Pressure", fp);
  fillingChart.addData("Temperature", ft);
  fillingChart.addData("Weight", -fw);

  max_f = max(max_f, fp);
  max_f = max(max_f, ft);
  max_f = max(max_f, fw);

  fillingChart.setRange(0, max_f*1.3);

  pressureLabel.setText("Pressure: " + df.format(fp));
  temperatureLabel.setText("Temperature: " + df.format(ft));
  weightLabel.setText("Weight: " + df.format(fw));

  // launch chart
  float lalt = float(rocket_data.kalman.altitude) * .1,
    lvel = float(rocket_data.kalman.vel_z) * .1,
    lacel = float(rocket_data.kalman.acel_z) * .1;

  launchChart.addData("Altitude", lalt);
  launchChart.addData("Velocity", lvel);
  launchChart.addData("Acceleration", lacel);

  max_l = max(max_l, lalt);
  max_l = max(max_l, lvel);
  max_l = max(max_l, lacel);

  altitudeLabel.setText("Altitude: " + df.format(lalt));
  velocityLabel.setText("Velocity: " + df.format(lvel));
  accelerationLabel.setText("Acceleration: " + df.format(lacel));

  launchChart.setRange(0, max_l*1.3);
}
