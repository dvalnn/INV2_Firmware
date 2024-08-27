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
  testIcon = cp5.addIcon("testIcon", 1);
}

void updateDiagrams() {
  background(bgColor);
  if (fillTab.isActive()) {
    image(fill_diagram, width*.25, height*.45, fill_diagram.width* 1.2 * width/1920, fill_diagram.height * 1.2 * height/1080); // scale image with display size
  }
}
