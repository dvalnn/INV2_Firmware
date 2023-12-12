button filling, launch;

void initializeStartScreen() {
  filling = new button(width/2-200, height/2-50, 150, 100, color(0), color(255), 10, "FILL");
  launch = new button(width/2 + 50, height/2 - 50, 150, 100, color(0), color(255), 10, "LAUNCH");

  log("INITIALIZED START SCREEN");
}

void startScreen() {
  if (state != prevState){
    log("START SCREEN");
    prevState = state;
  }
  if (filling.pushButton()) {
    state = 1;
    log("CHANGED STATE - " + state);
  }
  if (launch.pushButton()) {
    state = 2;
    log("CHANGED STATE - " + state);
  }
}