void setupControllers() {
  manual_setup();
  filling_setup();
  launch_setup();

  cp5.addScrollableList("program")
    .setPosition(displayWidth*.02, displayHeight*.05)
    .setSize((int)(displayWidth*.15), (int)(displayHeight*.46))
    .setBarHeight((int)(displayHeight*.05))
    .setItemHeight((int)(displayHeight*.05))
    .addItems(programs)
    .setFont(font)
    .setColor(defaultColor)
    .moveTo("filling")
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER);


  // Textfields for parameters and limits
  textfields[0] = cp5.addTextfield("Target Pressure")
    .setAutoClear(false)
    .setColor(defaultColor)
    .setPosition(displayWidth*.23, displayHeight*.05)
    .setSize((int)(displayWidth*.13), (int)(displayHeight*.05))
    .setFont(font)
    .setInputFilter(ControlP5.FLOAT)
    .moveTo("filling");

  textfields[1] = cp5.addTextfield("Trigger Pressure")
    .setAutoClear(false)
    .setColor(defaultColor)
    .setPosition(displayWidth*.37, displayHeight*.05)
    .setSize((int)(displayWidth*.14), (int)(displayHeight*.05))
    .setFont(font)
    .setInputFilter(ControlP5.FLOAT)
    .moveTo("filling");

  textfields[2] = cp5.addTextfield("Target Liquid")
    .setAutoClear(false)
    .setColor(defaultColor)
    .setPosition(displayWidth*.52, displayHeight*.05)
    .setSize((int)(displayWidth*.1), (int)(displayHeight*.05))
    .setFont(font)
    .setInputFilter(ControlP5.FLOAT)
    .moveTo("filling");

  // Send button
  cp5.addButton("Send")
    .setPosition(displayWidth*.63, displayHeight*.05)
    .setSize((int)(displayWidth*.1), (int)(displayHeight*button_height))
    .moveTo("filling")
    .setColor(defaultColor)
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);

  cp5.addButton("Start Filling")
    .setPosition(displayWidth*button_x1, displayHeight*.3)
    .setSize((int)(displayWidth*button_width), (int)(displayHeight*button_height))
    .moveTo("filling")
    .setColor(defaultColor)
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);

  cp5.addButton("Stop")
    .setPosition(displayWidth*button_x1, displayHeight*.1)
    .setSize((int)(displayWidth*button_width), (int)(displayHeight*button_height))
    .moveTo("global")
    .setColor(stopColor)
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);

  cp5.addButton("Resume")
    .setPosition(displayWidth*button_x1, displayHeight*.35)
    .setSize((int)(displayWidth*button_width), (int)(displayHeight*button_height))
    .moveTo("filling")
    .setColor(defaultColor)
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);

  cp5.addButton("Status")
    .setPosition(displayWidth*button_x1, displayHeight*.15)
    .setSize((int)(displayWidth*button_width), (int)(displayHeight*button_height))
    .moveTo("global")
    .setColor(defaultColor)
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);

  cp5.addButton("Abort")
    .setPosition(displayWidth*button_x1, displayHeight*.05)
    .setSize((int)(displayWidth*button_width), (int)(displayHeight*button_height))
    .setColor(abortColor)
    .moveTo("global")
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);

  cp5.addButton("Arm")
    .setPosition(displayWidth*button_x1, displayHeight*.27)
    .setSize((int)(displayWidth*button_width), (int)(displayHeight*button_height))
    .moveTo("launch")
    .setColor(defaultColor)
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);

  cp5.addButton("Ready")
    .setPosition(displayWidth*button_x1, displayHeight*.32)
    .setSize((int)(displayWidth*button_width), (int)(displayHeight*button_height))
    .moveTo("launch")
    .setColor(defaultColor)
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);

  cp5.addButton("Fire")
    .setPosition(displayWidth*button_x1, displayHeight*.42)
    .setSize((int)(displayWidth*button_width), (int)(displayHeight*button_height))
    .moveTo("launch")
    .setColor(defaultColor)
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);

  cp5.addButton("Allow Launch")
    .setPosition(displayWidth*button_x1, displayHeight*.47)
    .setSize((int)(displayWidth*button_width), (int)(displayHeight*button_height))
    .moveTo("launch")
    .setColor(defaultColor)
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);

  fillingChart = cp5.addChart("Filling Chart")
    .setPosition(displayWidth*.23, displayHeight*.16)
    .setSize((int)(displayWidth*.39), (int)(displayHeight*.15))
    .setRange(0, 1000) // TODO change min and max (maybe dynamic)
    .setView(Chart.LINE) // use Chart.LINE, Chart.PIE, Chart.AREA, Chart.BAR_CENTERED
    .setStrokeWeight(1.5)
    .setColor(defaultColor)
    .setColorCaptionLabel(color(255))
    .moveTo("filling")
    .setFont(font);

  launchChart = cp5.addChart("Launch Chart")
    .setPosition(displayWidth*.23, displayHeight*.16)
    .setSize((int)(displayWidth*.39), (int)(displayHeight*.15))
    .setRange(0, 1000) // TODO change min and max (maybe dynamic)
    .setView(Chart.LINE) // use Chart.LINE, Chart.PIE, Chart.AREA, Chart.BAR_CENTERED
    .setStrokeWeight(1.5)
    .setColor(defaultColor)
    .setColorCaptionLabel(color(255))
    .moveTo("launch")
    .setFont(font);

  cp5.addButton("Reset Chart")
    .setPosition(displayWidth*.49, displayHeight*.32)
    .setSize((int)(displayWidth*button_width), (int)(displayHeight*button_height))
    .moveTo("filling")
    .setColor(defaultColor)
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);

  cp5.addButton("Reset")
    .setPosition(displayWidth*.49, displayHeight*.32)
    .setSize((int)(displayWidth*button_width), (int)(displayHeight*button_height))
    .moveTo("launch")
    .setColor(defaultColor)
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);

  ematch_label = cp5.addTextlabel("eMatch Value")
    .setText("E-Match value goes here")
    .setColor(labelColor)
    .setPosition(displayWidth*.4, displayHeight*.05)
    .moveTo("launch")
    .setFont(font);

  List<String> portNames = Arrays.asList(Serial.list());   // List available serial ports and add them to a new ScrollableList

  cp5.addScrollableList("serialPort")
    .setPosition(displayWidth*.02, displayHeight*.65)
    .setSize((int)(displayWidth*.15), (int)(displayHeight*.46))
    .setBarHeight((int)(displayHeight*.05))
    .setItemHeight((int)(displayHeight*.05))
    .addItems(portNames)
    .setFont(font)
    .setColor(defaultColor)
    .moveTo("global")
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER);

  cp5.addScrollableList("Select ID")
    .setPosition(displayWidth*.02, displayHeight*.75)
    .setSize((int)(displayWidth*.17), (int)(displayHeight*.5))
    .setBarHeight((int)(displayHeight*.05))
    .setItemHeight((int)(displayHeight*.05))
    .addItems(IDs)
    .setFont(font)
    .setColor(defaultColor)
    .moveTo("global")
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER);

  log_display_rocket = cp5.addTextlabel("Rocket Log")
    .setText("Rocket logs go here")
    .setColor(labelColor)
    .setPosition(displayWidth*button_x1, displayHeight*.71)
    .moveTo("global")
    .setFont(font);

  log_display_filling = cp5.addTextlabel("Filling Log")
    .setText("Filling logs go here")
    .setColor(labelColor)
    .setPosition(displayWidth*button_x1, displayHeight*.8)
    .moveTo("global")
    .setFont(font);

  ack_display = cp5.addTextlabel("Ack Display")
    .setText("Acks go here")
    .setColor(labelColor2)
    .setPosition(displayWidth*button_x1, displayHeight*.54)
    .moveTo("global")
    .setFont(font);

  log_stats = cp5.addTextlabel("Log Stats")
    .setText("Log stats go here")
    .setColor(labelColor2)
    .setPosition(displayWidth*button_x1, displayHeight*.6)
    .moveTo("global")
    .setFont(font);

  history = cp5.addTextlabel("Comms history")
    .setText("comms history goes here")
    .setColor(labelColor2)
    .setPosition(displayWidth*.63, displayHeight*.54)
    .moveTo("global")
    .setFont(font);

  status_toggle = cp5.addToggle("Status Toggle")
    .setPosition(width*.02, height*.55)
    .setSize((int)(width*.05), (int)(width*.02))
    .setValue(false)
    .setMode(ControlP5.SWITCH)
    .setLabel("Toggle Status")
    .setFont(font)
    .moveTo("global")
    .setColorForeground(color(255, 0, 0))  // Red when off
    .setColorBackground(color(100, 0, 0))
    .setColorActive(color(255, 0, 0));

  // manual
  cp5.addButton("Start Manual")
    .setPosition(width*button_x1, height*.3)
    .setSize((int)(width*.17), (int)(height*.05))
    .moveTo("default")
    .setColor(defaultColor)
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);

  cp5.addScrollableList("Select Valve")
    .setPosition(width*.02, height*.05)
    .setSize((int)(width*.17), (int)(height*.5))
    .setBarHeight((int)(height*.05))
    .setItemHeight((int)(height*.05))
    .addItems(valves)
    .setFont(font)
    .setColor(defaultColor)
    .moveTo("default")
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER);

  cp5.addButton("Change Valve State")
    .setPosition(width*.02, height*.45)
    .setSize((int)(width*.17), (int)(height*.05))
    .moveTo("default")
    .setColor(defaultColor)
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setColor(labelColorDark)
    .setFont(font);

  valve_toggle = cp5.addToggle("Valve Toggle")
    .setPosition(width*.02, height*.37)
    .setSize((int)(width*.05), (int)(width*.02))
    .setValue(false)
    .setMode(ControlP5.SWITCH)
    .setLabel("Valve State")
    .setFont(font)
    .setColorForeground(color(255, 0, 0))  // Red when off
    .setColorBackground(color(100, 0, 0))
    .setColorActive(color(255, 0, 0))
    .moveTo("default");
    
    cp5.addToggle("Mode Toggle")
    .setPosition(width*.94, height*.01)
    .setSize((int)(width*.05), (int)(width*.02))
    .setValue(false)
    .setMode(ControlP5.SWITCH)
    .setLabel("")
    .setFont(font)
    .setColorForeground(color(0))  // Red when off
    .setColorBackground(color(50))
    .setColorActive(color(0))
    .moveTo("default");

  for (int i = 0; i < man_commands.size(); i++) {
    cp5.addButton(man_commands.get(i))
      .setPosition(width*.6, height*.05 + height*.06*i)
      .setSize((int)(width*.17), (int)(height*.05))
      .moveTo("default")
      .setColor(defaultColor)
      .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
      .setFont(font);
  }

  valve_ms = cp5.addTextfield("Valve open time")
    .setAutoClear(false)
    .setColor(defaultColor)
    .setPosition(displayWidth*.25, displayHeight*.05)
    .setSize((int)(displayWidth*.13), (int)(displayHeight*.05))
    .setFont(font)
    .setInputFilter(ControlP5.FLOAT)
    .moveTo("default");
  ;

  cp5.addButton("Open valve")
    .setPosition(width*.4, height*.05)
    .setSize((int)(width*.1), (int)(height*.05))
    .moveTo("default")
    .setColor(defaultColor)
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);

  chamber_temps_label = cp5.addTextlabel("Chamber Temps")
    .setText("Chamber temps")
    .setColor(labelColor)
    .setPosition(displayWidth*.63, displayHeight*.34)
    .moveTo("global")
    .setFont(font)
    .hide();
}

void launch_setup() {
  launchTab = cp5.addTab("launch")
    .activateEvent(true)
    .setColorLabel(labelColorDark)
    .setColorActive(red)
    .setColorForeground(blue)
    .setColorBackground(dark_blue)
    .setHeight((int)(displayHeight*.03))
    .setWidth((int)(displayWidth*.055))
    ;

  launchTab.getCaptionLabel()
    .setFont(font);
}

void filling_setup() {
  fillTab = cp5.addTab("filling")
    .activateEvent(true)
    .setColorLabel(labelColorDark)
    .setColorActive(red)
    .setColorForeground(blue)
    .setColorBackground(dark_blue)
    .setHeight((int)(displayHeight*.03))
    .setWidth((int)(displayWidth*.055))
    ;

  fillTab.getCaptionLabel()
    .setFont(font);
}

void manual_setup() {
  cp5.getTab("default")
    .activateEvent(true)
    .setColorLabel(labelColorDark)
    .setColorActive(red)
    .setColorForeground(blue)
    .setColorBackground(dark_blue)
    .setHeight((int)(displayHeight*.03))
    .setWidth((int)(displayWidth*.06))
    .setLabel("manual")
    .getCaptionLabel()
    .setFont(font);
}
