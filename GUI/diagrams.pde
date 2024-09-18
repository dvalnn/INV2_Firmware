PImage fill_diagram;
Icon testIcon;

void setupDiagrams() {
  fill_diagram = loadImage(fill_img);
  he_label = cp5.addLabel("He\nT : ####\nP : ####")
    .setColor(color(255, 255, 255))
    .setFont(font)
    .moveTo("global")
    .setPosition(displayWidth*.5, displayHeight*.55)
    .setSize((int)(displayWidth*.1), (int)(displayHeight*.1))
    .setLock(true)
    ;

  he_toggle = cp5.addToggle("He Toggle")
    .setPosition(width*.53, height*.54)
    .setSize((int)(width*.02), (int)(height*.02))
    .setValue(false)
    .setMode(ControlP5.SWITCH)
    .setLabel("")
    .setFont(font)
    .setColorForeground(color(255, 0, 0))  // Red when off
    .setColorBackground(color(100, 0, 0))
    .setColorActive(color(255, 0, 0))
    .moveTo("default")
    ;

  n2o_label = cp5.addLabel("N2O\nT : ####\nP : ####")
    .setColor(color(255, 255, 255))
    .setFont(font)
    .moveTo("global")
    .setPosition(displayWidth*.5, displayHeight*.75)
    .setSize((int)(displayWidth*.1), (int)(displayHeight*.1))
    .setLock(true)
    ;
  n2o_toggle = cp5.addToggle("N2O Toggle")
    .setPosition(width*.5, height*.83)
    .setSize((int)(width*.02), (int)(width*.01))
    .setValue(false)
    .setMode(ControlP5.SWITCH)
    .setLabel("")
    .setFont(font)
    .setColorForeground(color(255, 0, 0))  // Red when off
    .setColorBackground(color(100, 0, 0))
    .setColorActive(color(255, 0, 0))
    .moveTo("default");

  line_label = cp5.addLabel("Line\nT : ####\nP : ####")
    .setColor(color(255, 255, 255))
    .setFont(font)
    .moveTo("global")
    .setPosition(displayWidth*.4, displayHeight*.72)
    .setSize((int)(displayWidth*.1), (int)(displayHeight*.1))
    .setLock(true)
    ;
  line_toggle = cp5.addToggle("Line Toggle")
    .setPosition(width*.43, height*.72)
    .setSize((int)(width*.02), (int)(width*.01))
    .setValue(false)
    .setMode(ControlP5.SWITCH)
    .setLabel("")
    .setFont(font)
    .setColorForeground(color(255, 0, 0))  // Red when off
    .setColorBackground(color(100, 0, 0))
    .setColorActive(color(255, 0, 0))
    .moveTo("default");

  tt_label = cp5.addLabel("Tank Top\nT : ####\nP : ####")
    .setColor(color(255, 255, 255))
    .setFont(font)
    .moveTo("global")
    .setPosition(displayWidth*.33, displayHeight*.41)
    .setSize((int)(displayWidth*.1), (int)(displayHeight*.1))
    .setLock(true)
    ;
  tt_toggle = cp5.addToggle("VPU Toggle")
    .setPosition(width*.39, height*.41)
    .setSize((int)(width*.02), (int)(width*.01))
    .setValue(false)
    .setMode(ControlP5.SWITCH)
    .setLabel("")
    .setFont(font)
    .setColorForeground(color(255, 0, 0))  // Red when off
    .setColorBackground(color(100, 0, 0))
    .setColorActive(color(255, 0, 0))
    .moveTo("default");

  tb_label = cp5.addLabel("Tank Bottom\nT : ####\nP : ####")
    .setColor(color(255, 255, 255))
    .setFont(font)
    .moveTo("global")
    .setPosition(displayWidth*.35, displayHeight*.91)
    .setSize((int)(displayWidth*.1), (int)(displayHeight*.1))
    .setLock(true)
    ;
  tb_toggle = cp5.addToggle("Engine Toggle")
    .setPosition(width*.43, height*.91)
    .setSize((int)(width*.02), (int)(width*.01))
    .setValue(false)
    .setMode(ControlP5.SWITCH)
    .setLabel("")
    .setFont(font)
    .setColorForeground(color(255, 0, 0))  // Red when off
    .setColorBackground(color(100, 0, 0))
    .setColorActive(color(255, 0, 0))
    .moveTo("default");

  tl_label = cp5.addTextlabel("Tank Liquid")
    .setText("Liquid:XX.xx%\n\n\nXX.xxm\n\n\nXX.xxm3\n\n\nXX.xxkg\n\n\nXX.xxkg")
    .setPosition(displayWidth*.26, displayHeight*.565)
    .moveTo("global")
    .setFont(font);


  diagram_labels = Arrays.asList(he_label, n2o_label, line_label, tt_label, tb_label, tl_label);
  valve_toggles = Arrays.asList(tt_toggle, tb_toggle, he_toggle, n2o_toggle, line_toggle);
}

void updateDiagrams() {
  background(bgColor);
  image(fill_diagram, width*.25, height*.45, fill_diagram.width* 1.2 * width/1920, fill_diagram.height * 1.2 * height/1080); // scale image with display size
  String fbools = String.format("%8s", Integer.toBinaryString(f_bools & 0xFF)).replace(' ', '0');
  String rbools = String.format("%8s", Integer.toBinaryString(r_bools & 0xFF)).replace(' ', '0');

  int tt_valve = Integer.parseInt(rbools.substring(1, 2));
  int tb_valve = Integer.parseInt(rbools.substring(2, 3));
  int he_valve = Integer.parseInt(fbools.substring(1, 2));
  int n2o_valve = Integer.parseInt(fbools.substring(2, 3));
  int line_valve = Integer.parseInt(fbools.substring(3, 4));

  for ( Toggle toggle : valve_toggles) {
    toggle.setBroadcast(false);
  }

  // he valve
  if (he_valve == 1) {
    fill(0, 255, 0);
    he_toggle.setState(true);
  } else {
    fill(255, 0, 0);
    he_toggle.setState(false);
  }
  circle(width*.56, height*.633, height*.018);

  // n2o valve
  if (n2o_valve == 1) {
    fill(0, 255, 0);
    n2o_toggle.setState(true);
  } else {
    fill(255, 0, 0);
    n2o_toggle.setState(false);
  }
  circle(width*.56, height*.73, height*.018);

  // line valve
  if (line_valve == 1) {
    fill(0, 255, 0);
    line_toggle.setState(true);
  } else {
    fill(255, 0, 0);
    line_toggle.setState(false);
  }
  circle(width*.423, height*.698, height*.018);

  // tt valve
  if (tt_valve == 1) {
    fill(0, 255, 0);
    tt_toggle.setState(true);
  } else {
    fill(255, 0, 0);
    tt_toggle.setState(false);
  }
  circle(width*.304, height*.46, height*.018);

  // tb valve
  if (tb_valve == 1) {
    fill(0, 255, 0);
    tb_toggle.setState(true);
  } else {
    fill(255, 0, 0);
    tb_toggle.setState(false);
  }
  circle(width*.304, height*.935, height*.018);

  for (Toggle toggle : valve_toggles) {
    boolean v_state = toggle.getState();
    if (v_state == true) {
      toggle.setColorForeground(color(0, 255, 0))
        .setColorBackground(color(0, 100, 0))
        .setColorActive(color(0, 255, 0));     // Green when on
    } else if (v_state == false) {
      toggle.setColorForeground(color(255, 0, 0))// Red when off
        .setColorActive(color(255, 0, 0))    // Red when off
        .setColorBackground(color(100, 0, 0));
    }
    toggle.setBroadcast(true);
  }
}

void multi_tab_controllers(String tab) {
    if (tab == "launch") {
      weightLabel.hide();
    } else {
      weightLabel.show();
    }
}
