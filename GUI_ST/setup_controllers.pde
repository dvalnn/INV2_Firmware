void setupControllers() {
  fillTab = cp5.addTab("extra")
    .setColorBackground(color(0, 160, 100))
    .setColorLabel(color(255))
    .setColorActive(color(255, 128, 0))
    .setHeight((int)(displayHeight*.03))
    .setWidth((int)(displayHeight*.08))
    ;

  fillTab.getCaptionLabel()
    .setFont(font);

  cp5.getTab("default")
    .getCaptionLabel()
    .setFont(font);

  cp5.addScrollableList("program")
    .setPosition(displayWidth*.02, displayHeight*.05)
    .setSize((int)(displayWidth*.15), (int)(displayHeight*.46))
    .setBarHeight((int)(displayHeight*.05))
    .setItemHeight((int)(displayHeight*.05))
    .addItems(programs)
    .setFont(font)
    .setColor(blue)
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER);

  // Textfields for parameters and limits
  textfields[0] = cp5.addTextfield("Target Pressure")
    .setAutoClear(false)
    .setColor(blue)
    .setPosition(displayWidth*.23, displayHeight*.05)
    .setSize((int)(displayWidth*.13), (int)(displayHeight*.05))
    .setFont(font)
    .setInputFilter(ControlP5.FLOAT);
  textfields[1] = cp5.addTextfield("Trigger Pressure")
    .setAutoClear(false)
    .setColor(blue)
    .setPosition(displayWidth*.37, displayHeight*.05)
    .setSize((int)(displayWidth*.14), (int)(displayHeight*.05))
    .setFont(font)
    .setInputFilter(ControlP5.FLOAT);
  textfields[2] = cp5.addTextfield("Target Liquid")
    .setAutoClear(false)
    .setColor(blue)
    .setPosition(displayWidth*.52, displayHeight*.05)
    .setSize((int)(displayWidth*.1), (int)(displayHeight*.05))
    .setFont(font)
    .setInputFilter(ControlP5.FLOAT);

  // Send button
  cp5.addButton("Send")
    .setPosition(displayWidth*button_x1, displayHeight*.05)
    .setSize((int)(displayWidth*button_width), (int)(displayHeight*button_height))
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);

  cp5.addButton("Start Filling")
    .setPosition(displayWidth*button_x1, displayHeight*.1)
    .setSize((int)(displayWidth*button_width), (int)(displayHeight*button_height))
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);

  cp5.addButton("Stop")
    .setPosition(displayWidth*button_x1, displayHeight*.15)
    .setSize((int)(displayWidth*button_width), (int)(displayHeight*button_height))
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);

  cp5.addButton("Resume")
    .setPosition(displayWidth*button_x1, displayHeight*.20)
    .setSize((int)(displayWidth*button_width), (int)(displayHeight*button_height))
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);

  cp5.addButton("Status")
    .setPosition(displayWidth*button_x1, displayHeight*.25)
    .setSize((int)(displayWidth*button_width), (int)(displayHeight*button_height))
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);

  cp5.addButton("Abort")
    .setPosition(displayWidth*button_x1, displayHeight*.30)
    .setSize((int)(displayWidth*button_width), (int)(displayHeight*button_height))
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);
  cp5.addButton("Arm")
    .setPosition(displayWidth*button_x1, displayHeight*.35)
    .setSize((int)(displayWidth*button_width), (int)(displayHeight*button_height))
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);
  cp5.addButton("Ready")
    .setPosition(displayWidth*button_x1, displayHeight*.40)
    .setSize((int)(displayWidth*button_width), (int)(displayHeight*button_height))
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);
  cp5.addButton("Manual")
    .setPosition(displayWidth*button_x1, displayHeight*.50)
    .setSize((int)(displayWidth*button_width), (int)(displayHeight*button_height))
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);
  cp5.addButton("Fire")
    .setPosition(displayWidth*button_x2, displayHeight*.05)
    .setSize((int)(displayWidth*button_width), (int)(displayHeight*button_height))
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);
  cp5.addButton("Allow Launch")
    .setPosition(displayWidth*button_x2, displayHeight*.1)
    .setSize((int)(displayWidth*button_width), (int)(displayHeight*button_height))
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);

  rocketChart = cp5.addChart("Rocket P+L")
    .setPosition(displayWidth*.23, displayHeight*.16)
    .setSize((int)(displayWidth*.39), (int)(displayHeight*.15))
    .setRange(0, 1000) // TODO change min and max (maybe dynamic)
    .setView(Chart.LINE) // use Chart.LINE, Chart.PIE, Chart.AREA, Chart.BAR_CENTERED
    .setStrokeWeight(1.5)
    .setColorCaptionLabel(color(255))
    .setFont(font);

  //fillingChart = cp5.addChart("Filling P+L")
  //.setPosition(displayWidth*.23, displayHeight*.35)
  //.setSize((int)(displayWidth*.39), (int)(displayHeight*.15))
  //.setRange(0, 1000) // TODO change min and max (maybe dynamic)
  //.setView(Chart.LINE) // use Chart.LINE, Chart.PIE, Chart.AREA, Chart.BAR_CENTERED
  //.setStrokeWeight(1.5)
  //.setColorCaptionLabel(color(255))
  //.setFont(font);


  List<String> portNames = Arrays.asList(Serial.list());   // List available serial ports and add them to a new ScrollableList

  cp5.addScrollableList("serialPort")
    .setPosition(displayWidth*.02, displayHeight*.5)
    .setSize((int)(displayWidth*.15), (int)(displayHeight*.46))
    .setBarHeight((int)(displayHeight*.05))
    .setItemHeight((int)(displayHeight*.05))
    .addItems(portNames)
    .setFont(font)
    .setColorBackground(color(50, 50, 50))
    .setColorForeground(color(0, 144, 0))
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER);

  cp5.addScrollableList("Select ID")
    .setPosition(displayWidth*button_x1, displayHeight*.6)
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
    .setPosition(displayWidth*.23, displayHeight*.54)
    .setFont(font);

  log_display_filling = cp5.addTextlabel("Filling Log")
    .setText("Logging Packet goes here")
    .setPosition(displayWidth*.46, displayHeight*.54)
    .setFont(font);

  ack_display = cp5.addTextlabel("Ack Display")
    .setText("Acks go here")
    .setPosition(displayWidth*button_x2, displayHeight*.17)
    .setFont(font);

  log_stats = cp5.addTextlabel("Log Stats")
    .setText("Log stats go here")
    .setPosition(displayWidth*button_x2, displayHeight*.25)
    .setFont(font);

  status_toggle = cp5.addToggle("Status Toggle")
    .setPosition(width*.02, height*.42)
    .setSize((int)(width*.05), (int)(width*.02))
    .setValue(false)
    .setMode(ControlP5.SWITCH)
    .setLabel("Toggle Status")
    .setFont(font)
    .setColorForeground(color(255, 0, 0))  // Red when off
    .setColorBackground(color(100, 0, 0))
    .setColorActive(color(255, 0, 0));
}
