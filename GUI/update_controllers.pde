void updateControllers() {
  log_display_filling.setText(filling_data.getState());
      String he_data = "He:\n" +
                    "P: " + filling_data.he.pressure + " bar\n" +
                   "T: " + filling_data.he.temperature + " ºC\n";
    he_label.setText(he_data);
    String n2o_data = "N2O:\n" +
                      "P: " + filling_data.n2o.pressure + " bar\n" +
                      "T: " + filling_data.n2o.temperature + " ºC\n" +
                      "W: " + filling_data.n2o.loadcell + " kg\n";
    n2o_label.setText(n2o_data);
    String line_data = "Line:\n" +
                       "P: " + filling_data.line.pressure + " bar\n" +
                       "T: " + filling_data.line.temperature + " ºC\n";
    line_label.setText(line_data);

    log_display_rocket.setText(rocket_data.getState());
    String tt_data = "Tank Top:\n" +
                     "P: " + rocket_data.tank.pressure_top + " bar\n" +
                     "T: " + rocket_data.tank.temp_top + " ºC\n";
    tt_label.setText(tt_data);
    String tb_data = "Tank Bottom:\n" +
                     "P: " + rocket_data.tank.pressure_bot + " bar\n" +
                     "T: " + rocket_data.tank.temp_bot + " ºC\n";
    tb_label.setText(tb_data);
}
