button filling, launch;

void initializeStartScreen() {
  filling = new button(width/2-200, height/2-50, 150, 100, color(0), color(255), 10, "FILL");
  launch = new button(width/2 + 50, height/2 - 50, 150, 100, color(0), color(255), 10, "LAUNCH");
}

void startScreen() {
  if (filling.pushButton()) {
    state = 1;
  }
  if (launch.pushButton()) {
    state = 2;
  }
}