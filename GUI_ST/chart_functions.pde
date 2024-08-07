void setupCharts() {
  rocketChart.addDataSet("Pressure");
  rocketChart.addDataSet("Liquid");
  rocketChart.setData("Pressure", new float[1000]);
  rocketChart.setData("Liquid", new float[1000]);
  rocketChart.setColors("Pressure", color(0, 255, 0));
  rocketChart.setColors("Liquid", color(255, 255, 0));
  
  fillingChart.addDataSet("Pressure");
  fillingChart.addDataSet("Liquid");
  fillingChart.setData("Pressure", new float[100]);
  fillingChart.setData("Liquid", new float[100]);
  fillingChart.setColors("Pressure", color(0, 255, 0));
  fillingChart.setColors("Liquid", color(255, 255, 0));
}

void updateCharts(int rp, int rl, int fp, int fl) {
  rocketChart.push("Pressure", rp);
  rocketChart.push("Liquid", rl);
  max_r = max(max_r, rp);
  max_r = max(max_r, rl);
  rocketChart.setRange(0, max_r*2);
  text("Pressure: " + str(rp), rocketChart.getPosition()[0] + rocketChart.getWidth() + 10, rocketChart.getPosition()[1] + rocketChart.getHeight() * (1 - ((float)rp / max_r*.5)));
  text("Liquid: " + str(rl), rocketChart.getPosition()[0] + rocketChart.getWidth() + 10, 10 + rocketChart.getPosition()[1] + rocketChart.getHeight() * (1 - ((float)rl / max_r*.5)));
  
  fillingChart.push("Pressure", fp);
  fillingChart.push("Liquid", fl);
  max_f = max(max_f, fp);
  max_r = max(max_f, fl);
  fillingChart.setRange(0, max_f*2);
  text("Pressure: " + str(fp), fillingChart.getPosition()[0] + fillingChart.getWidth() + 10, fillingChart.getPosition()[1] + fillingChart.getHeight() * (1 - (fp / max_f*.5)));
  text("Liquid: " + str(fl), fillingChart.getPosition()[0] + fillingChart.getWidth() + 10, 10 + fillingChart.getPosition()[1] + fillingChart.getHeight() * (1 - (fl / max_f*.5)));
}
