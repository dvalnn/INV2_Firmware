void setupControllers() {
  cp5.addScrollableList("program")
    .setPosition(displayWidth*.05, displayHeight*.1)
    .setSize((int)(displayWidth*.15), (int)(displayHeight*.46))
    .setBarHeight((int)(displayHeight*.05))
    .setItemHeight((int)(displayHeight*.05))
    .addItems(programs)
    .setFont(font)
    .setColor(blue)
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER);

  // Textfields for parameters and limits
  for (int i = 1; i <= 3; i++) {
    textfields[i-1] = cp5.addTextfield("tp" + i)
      .setAutoClear(false)
      .setColor(blue)
      .setPosition(displayWidth*.15 + i * displayWidth*.09, displayHeight*.09)
      .setSize((int)(displayWidth*.08), (int)(displayHeight*.05))
      .setFont(font)
      .setInputFilter(ControlP5.FLOAT);
  }

  for (int i = 1; i <= 2; i++) {
    textfields[i+2] = cp5.addTextfield("tl" + i)
      .setAutoClear(false)
      .setColor(blue)
      .setPosition(displayWidth*.42 + i * displayWidth*.09, displayHeight*.09)
      .setSize((int)(displayWidth*.08), (int)(displayHeight*.05))
      .setFont(font)
      .setInputFilter(ControlP5.FLOAT);
  }

  // Send button
  cp5.addButton("Send")
    .setPosition(displayWidth*button_x, displayHeight*.05)
    .setSize((int)(displayWidth*.1), (int)(displayHeight*.04))
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);

  cp5.addButton("Start Filling")
    .setPosition(displayWidth*button_x, displayHeight*.1)
    .setSize((int)(displayWidth*.13), (int)(displayHeight*button_height))
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);

  cp5.addButton("Stop")
    .setPosition(displayWidth*button_x, displayHeight*.15)
    .setSize((int)(displayWidth*.05), (int)(displayHeight*button_height))
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);

  cp5.addButton("Resume")
    .setPosition(displayWidth*button_x, displayHeight*.20)
    .setSize((int)(displayWidth*.07), (int)(displayHeight*button_height))
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);

  cp5.addButton("Status")
    .setPosition(displayWidth*button_x, displayHeight*.25)
    .setSize((int)(displayWidth*.07), (int)(displayHeight*button_height))
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);

  cp5.addButton("Abort")
    .setPosition(displayWidth*button_x, displayHeight*.30)
    .setSize((int)(displayWidth*.07), (int)(displayHeight*button_height))
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);
  cp5.addButton("Arm")
    .setPosition(displayWidth*button_x, displayHeight*.35)
    .setSize((int)(displayWidth*.07), (int)(displayHeight*button_height))
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);

  rocketChart = cp5.addChart("Rocket P+L")
    .setPosition(displayWidth*.3, displayHeight*.5)
    .setSize((int)(displayWidth*.4), (int)(displayHeight*.15))
    .setRange(0, 1000) // TODO change min and max (maybe dynamic)
    .setView(Chart.LINE) // use Chart.LINE, Chart.PIE, Chart.AREA, Chart.BAR_CENTERED
    .setStrokeWeight(1.5)
    .setColorCaptionLabel(color(255))
    .setFont(font);
    
  fillingChart = cp5.addChart("Filling P+L")
    .setPosition(displayWidth*.3, displayHeight*.25)
    .setSize((int)(displayWidth*.4), (int)(displayHeight*.15))
    .setRange(0, 1000) // TODO change min and max (maybe dynamic)
    .setView(Chart.LINE) // use Chart.LINE, Chart.PIE, Chart.AREA, Chart.BAR_CENTERED
    .setStrokeWeight(1.5)
    .setColorCaptionLabel(color(255))
    .setFont(font);


  List<String> portNames = Arrays.asList(Serial.list());   // List available serial ports and add them to a new ScrollableList

  cp5.addScrollableList("serialPort")
    .setPosition(displayWidth*.05, displayHeight*.5)
    .setSize((int)(displayWidth*.2), (int)(displayHeight*.46))
    .setBarHeight((int)(displayHeight*.05))
    .setItemHeight((int)(displayHeight*.05))
    .addItems(portNames)
    .setFont(font)
    .setColorBackground(color(50, 50, 50))
    .setColorForeground(color(0, 144, 0))
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER);

  cp5.addScrollableList("Select ID")
    .setPosition(displayWidth*button_x, displayHeight*.5)
    .setSize((int)(displayWidth*.17), (int)(displayHeight*.5))
    .setBarHeight((int)(displayHeight*.05))
    .setItemHeight((int)(displayHeight*.05))
    .addItems(IDs)
    .setFont(font)
    .setColorBackground(color(50, 50, 50))
    .setColorForeground(color(0, 144, 0))
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER);

  log_display_rocket = cp5.addTextlabel("Rocket Log")
    .setText("Logging Packet goes here")
    .setPosition(displayWidth*.15, displayHeight*.8)
    .setFont(font);
  log_display_filling = cp5.addTextlabel("Filling Log")
    .setText("Logging Packet goes here")
    .setPosition(displayWidth*.6, displayHeight*.8)
    .setFont(font);
}
