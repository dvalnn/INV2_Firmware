void setupCharts() {
  fillingChart.addDataSet("Pressure");
  fillingChart.addDataSet("Liquid");
  fillingChart.addDataSet("Temperature");
  fillingChart.addDataSet("Weight");

  launchChart.addDataSet("Weight 1");
  launchChart.addDataSet("Weight 2");
  launchChart.addDataSet("Weight 3");
  launchChart.addDataSet("Tank Pressure");
  launchChart.addDataSet("Chamber Pressure");

  fillingChart.setColors("Pressure", color(0, 255, 0));
  fillingChart.setColors("Liquid", color(255, 255, 0));
  fillingChart.setColors("Temperature", color(255, 150, 255));
  fillingChart.setColors("Weight", color(0, 255, 255));

  launchChart.setColors("Weight 1", color(0, 255, 0));
  launchChart.setColors("Weight 2", color(255, 255, 0));
  launchChart.setColors("Weight 3", color(255, 150, 255));
  launchChart.setColors("Tank Pressure", color(0, 255, 255));
  launchChart.setColors("Chamber Pressure", color(255, 0, 255));
  
  pressureLabel = cp5.addLabel("Pressure: ")
    .setColor(color(0, 255, 0))
    .setFont(font)
    .moveTo("filling")
    .setPosition(displayWidth*.63, displayHeight*.16)
    .setSize((int)(displayWidth*.1), (int)(displayHeight*.1))
    ;
  liquidLabel = cp5.addLabel("Liquid: ")
    .setColor(color(255, 255, 0))
    .setFont(font)
    .moveTo("filling")
    .setPosition(displayWidth*.63, displayHeight*.20)
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

  weight1Label = cp5.addLabel("Weight 1: ")
    .setColor(color(0, 255, 0))
    .setFont(font)
    .moveTo("launch")
    .setPosition(displayWidth*.63, displayHeight*.16)
    .setSize((int)(displayWidth*.1), (int)(displayHeight*.1))
    ;
  weight2Label = cp5.addLabel("Weight 2: ")
    .setColor(color(255, 255, 0))
    .setFont(font)
    .moveTo("launch")
    .setPosition(displayWidth*.63, displayHeight*.19)
    .setSize((int)(displayWidth*.1), (int)(displayHeight*.1))
    ;
  weight3Label = cp5.addLabel("Weight 3: ")
    .setColor(color(255, 150, 150))
    .setFont(font)
    .moveTo("launch")
    .setPosition(displayWidth*.63, displayHeight*.22)
    .setSize((int)(displayWidth*.1), (int)(displayHeight*.1))
    ;
  tankPressureLabel = cp5.addLabel("Tank Pressure: ")
    .setColor(color(0, 255, 255))
    .setFont(font)
    .moveTo("launch")
    .setPosition(displayWidth*.63, displayHeight*.25)
    .setSize((int)(displayWidth*.1), (int)(displayHeight*.1))
    ;
  chamberPressureLabel = cp5.addLabel("Chamber Pressure: ")
    .setColor(color(255, 0, 250))
    .setFont(font)
    .moveTo("launch")
    .setPosition(displayWidth*.63, displayHeight*.28)
    .setSize((int)(displayWidth*.1), (int)(displayHeight*.1))
    ;
}

void updateCharts(int p, int l, int t, int w0, int w1, int w2, int w3, int tp, int cp) {
  // filling chart
  fillingChart.addData("Pressure", p);
  fillingChart.addData("Liquid", l);
  fillingChart.addData("Temperature", t);
  fillingChart.addData("Weight", w0);
  max_f = max(max_f, p);
  max_f = max(max_f, l);
  max_f = max(max_f, t);
  max_f = max(max_f, w0);

  fillingChart.setRange(0, max_f*1.3);

  pressureLabel.setText("Pressure: " + p);
  liquidLabel.setText("Liquid: " + l);
  temperatureLabel.setText("Temperature: " + t);
  weightLabel.setText("Weight: " + w0);
  
  // launch chart
  launchChart.addData("Weight 1", w1);
  launchChart.addData("Weight 2", w2);
  launchChart.addData("Weight 3", w3);
  launchChart.addData("Tank Pressure", tp);
  launchChart.addData("Chamber Pressure", cp);

  max_l = max(max_l, w1);
  max_l = max(max_l, w2);
  max_l = max(max_l, w3);
  max_l = max(max_l, tp);
  max_l = max(max_l, cp);
  
  weight1Label.setText("Weight 1: " + w1);
  weight2Label.setText("Weight 2: " + w2);
  weight3Label.setText("Weight 3: " + w3);
  tankPressureLabel.setText("Tank Pressure: " + tp);
  chamberPressureLabel.setText("Chamber Pressure: " + cp);

  fillingChart.setRange(0, max_l*1.3);
}
