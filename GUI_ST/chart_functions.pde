void setupCharts() {
  rocketChart.addDataSet("Pressure");
  rocketChart.addDataSet("Liquid");
  rocketChart.setData("Pressure", new float[max_size]);
  rocketChart.setData("Liquid", new float[max_size]);
  rocketChart.setColors("Pressure", color(0, 255, 0));
  rocketChart.setColors("Liquid", color(255, 255, 0));
  
  //fillingChart.addDataSet("Pressure");
  //fillingChart.addDataSet("Liquid");
  //fillingChart.setData("Pressure", new float[10000]);
  //fillingChart.setData("Liquid", new float[10000]);
  //fillingChart.setColors("Pressure", color(0, 255, 0));
  //fillingChart.setColors("Liquid", color(255, 255, 0));
}

void updateCharts(int rp, int rl) {
  mock_value1 += random(-10, 10);
  mock_value2 += random(-5, 5);
  rocketChart.push("Pressure", mock_value1);
  rocketChart.push("Liquid", mock_value2);
  //max_r = max(max_r, rp);
  //max_r = max(max_r, rl);
  max_r = max(max_r, mock_value1);
  max_r = max(max_r, mock_value2);
  rocketChart.setRange(0, max_r*2);
  
  //fillingChart.push("Pressure", fp);
  //fillingChart.push("Liquid", fl);
  //max_f = max(max_f, fp);
  //max_r = max(max_f, fl);
  //fillingChart.setRange(0, max_f*2);
  //text("Pressure: " + str(fp), fillingChart.getPosition()[0] + fillingChart.getWidth() + 10, fillingChart.getPosition()[1] + fillingChart.getHeight() * (1 - (fp / max_f*.5)));
  //text("Liquid: " + str(fl), fillingChart.getPosition()[0] + fillingChart.getWidth() + 10, 10 + fillingChart.getPosition()[1] + fillingChart.getHeight() * (1 - (fl / max_f*.5)));
}
