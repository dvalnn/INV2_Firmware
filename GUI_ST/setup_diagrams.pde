PImage fill_diagram;
Icon testIcon;

void setupDiagrams() {
  fill_diagram = loadImage(fill_img);
  he_label = cp5.addLabel("He\nT : ####\nP : ####")
    .setColor(color(255, 255, 255))
    .setFont(font)
    .moveTo("filling")
    .setPosition(displayWidth*.5, displayHeight*.55)
    .setSize((int)(displayWidth*.1), (int)(displayHeight*.1))
    ;
  n2o_label = cp5.addLabel("N2O\nT : ####\nP : ####")
    .setColor(color(255, 255, 255))
    .setFont(font)
    .moveTo("filling")
    .setPosition(displayWidth*.5, displayHeight*.75)
    .setSize((int)(displayWidth*.1), (int)(displayHeight*.1))
    ;
  line_label = cp5.addLabel("Line\nT : ####\nP : ####")
    .setColor(color(255, 255, 255))
    .setFont(font)
    .moveTo("filling")
    .setPosition(displayWidth*.4, displayHeight*.72)
    .setSize((int)(displayWidth*.1), (int)(displayHeight*.1))
    ;
  tt_label = cp5.addLabel("Tank Top\nT : ####\nP : ####")
    .setColor(color(255, 255, 255))
    .setFont(font)
    .moveTo("filling")
    .setPosition(displayWidth*.33, displayHeight*.41)
    .setSize((int)(displayWidth*.1), (int)(displayHeight*.1))
    ;
  tb_label = cp5.addLabel("Tank Bottom\nT : ####\nP : ####")
    .setColor(color(255, 255, 255))
    .setFont(font)
    .moveTo("filling")
    .setPosition(displayWidth*.35, displayHeight*.91)
    .setSize((int)(displayWidth*.1), (int)(displayHeight*.1))
    ;
}

void updateDiagrams() {
  background(bgColor);
  if (fillTab.isActive()) {
    image(fill_diagram, width*.25, height*.45, fill_diagram.width* 1.2 * width/1920, fill_diagram.height * 1.2 * height/1080); // scale image with display size
    String fbools = String.format("%8s", Integer.toBinaryString(f_bools & 0xFF)).replace(' ', '0');
    String rbools = String.format("%8s", Integer.toBinaryString(r_bools & 0xFF)).replace(' ', '0');
    
    int tt_valve = Integer.parseInt(rbools.substring(1, 2));
    int tb_valve = Integer.parseInt(rbools.substring(2, 3));

    int he_valve = Integer.parseInt(fbools.substring(1, 2));
    int n2o_valve = Integer.parseInt(fbools.substring(2, 3));
    int line_valve = Integer.parseInt(fbools.substring(3, 4));

    // he valve
    if (he_valve == 1) {
      fill(0, 255, 0);
    } else {
      fill(255, 0, 0);
    }
    circle(width*.56, height*.633, height*.018);

    // n2o valve
    if (n2o_valve == 1) {
      fill(0, 255, 0);
    } else {
      fill(255, 0, 0);
    }
    circle(width*.56, height*.73, height*.018);

    // line valve
    if (line_valve == 1) {
      fill(0, 255, 0);
    } else {
      fill(255, 0, 0);
    }
    circle(width*.423, height*.698, height*.018);
    
     // tt valve
    if (tt_valve == 1) {
      fill(0, 255, 0);
    } else {
      fill(255, 0, 0);
    }
    circle(width*.304, height*.46, height*.018);
    
    // tb valve
    if (tb_valve == 1) {
      fill(0, 255, 0);
    } else {
      fill(255, 0, 0);
    }
    circle(width*.304, height*.935, height*.018);
  }
}
