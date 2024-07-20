void setupCharts() {
  rocketChart.addDataSet("Pressure");
  rocketChart.addDataSet("Liquid");
  rocketChart.setData("Pressure", new float[100]);
  rocketChart.setData("Liquid", new float[100]);
  rocketChart.setColors("Pressure", color(0, 255, 0));
  rocketChart.setColors("Liquid", color(255, 255, 0));
  
  fillingChart.addDataSet("Pressure");
  fillingChart.addDataSet("Liquid");
  fillingChart.setData("Pressure", new float[100]);
  fillingChart.setData("Liquid", new float[100]);
  fillingChart.setColors("Pressure", color(0, 255, 0));
  fillingChart.setColors("Liquid", color(255, 255, 0));
}

void updateCharts(float rp, float rl, float fp, float fl) {
  // unshift: add data from left to right (first in)
  //myChart.unshift("incoming", (sin(frameCount*0.1)*20));

  // push: add data from right to left (last in)
  rocketChart.push("Pressure", rp);
  rocketChart.push("Liquid", rl);
  max_r = max(max_r, int(rp));
  max_r = max(max_r, int(rl));
  rocketChart.setRange(0, max_r);
  text("Pressure", rocketChart.getPosition()[0] + rocketChart.getWidth() + 10, rocketChart.getPosition()[1] + rocketChart.getHeight() * (1 - (rp / max_r)));
  text("Liquid", rocketChart.getPosition()[0] + rocketChart.getWidth() + 10, rocketChart.getPosition()[1] + rocketChart.getHeight() * (1 - (rl / max_r)));
  
  fillingChart.push("Pressure", fp);
  fillingChart.push("Liquid", fl);
  max_f = max(max_f, int(fp));
  max_r = max(max_f, int(fl));
  fillingChart.setRange(0, max_f);
  text("Pressure", fillingChart.getPosition()[0] + fillingChart.getWidth() + 10, fillingChart.getPosition()[1] + fillingChart.getHeight() * (1 - (fp / max_f)));
  text("Liquid", fillingChart.getPosition()[0] + fillingChart.getWidth() + 10, fillingChart.getPosition()[1] + fillingChart.getHeight() * (1 - (fl / max_f)));
}
