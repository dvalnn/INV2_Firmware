void setupCharts() {
  rocketChart.addDataSet("Pressure");
  rocketChart.addDataSet("Liquid");
  rocketChart.addDataSet("Temperature");
  rocketChart.addDataSet("Weight");
  //rocketChart.setData("Pressure", new float[max_size]);
  //rocketChart.setData("Liquid", new float[max_size]);
  rocketChart.setColors("Pressure", color(0, 255, 0));
  rocketChart.setColors("Liquid", color(255, 255, 0));
  rocketChart.setColors("Temperature", color(255, 150, 255));
  rocketChart.setColors("Weight", color(0, 255, 255));

  //fillingChart.addDataSet("Pressure");
  //fillingChart.addDataSet("Liquid");
  //fillingChart.setData("Pressure", new float[10000]);
  //fillingChart.setData("Liquid", new float[10000]);
  //fillingChart.setColors("Pressure", color(0, 255, 0));
  //fillingChart.setColors("Liquid", color(255, 255, 0));
}

void updateCharts(int rp, int rl) {
  mock_value1 = random(0, 60);
  mock_value2 = random(0, 100);
  mock_value3 = random(0, 40);
  mock_value4 = random(0, 200);
  rocketChart.addData("Pressure", mock_value1);
  rocketChart.addData("Liquid", mock_value2);
  rocketChart.addData("Temperature", mock_value3);
  rocketChart.addData("Weight", mock_value4);
  //max_r = max(max_r, rp);
  //max_r = max(max_r, rl);
  max_r = max(max_r, mock_value1);
  max_r = max(max_r, mock_value2);
  max_r = max(max_r, mock_value3);
  max_r = max(max_r, mock_value4);
  rocketChart.setRange(0, max_r*1.3);

  //fillingChart.push("Pressure", fp);
  //fillingChart.push("Liquid", fl);
  //max_f = max(max_f, fp);
  //max_r = max(max_f, fl);
  //fillingChart.setRange(0, max_f*2);

  pressureLabel.setText("Pressure: " + mock_value1);
  liquidLabel.setText("Liquid: " + mock_value2);
  temperatureLabel.setText("Temperature: " + mock_value3);
  weightLabel.setText("Weight: " + mock_value4);
}
