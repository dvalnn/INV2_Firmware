class button {
  //at the time it makes a rectangular button using the top left corner's coordinates
  //and the width and height of the rectangular button, it might a have a bit of rounding with fuction overloading,
  //some refactoring might make it so that you can make round buttons or buttons with some other shapes (me from the future here, image buttons!)
  PVector pos;    //saves the coordinates of the top left corner of the button
  PVector size;   //x saves width, y saves height
  PVector new_pos, new_size; // used to make the animations gradual
  color fill;     //button filling color
  color stroke;   //button stroke filling color
  float rounding; //the radius of the rectangle's corners
  String label;   //saves whatever the fuck the name of the button should be

  float textSize; // pretty self-explanatory
  float new_textSize; // used to make the animations gradual 

  boolean smol = false;
  float hoveringCoefficient = 1.2;

  boolean bigger = false;
  float pressingCoefficient = 0.85;

  boolean outside = false;

  //toggle variables
  boolean state = false;
  boolean current = false;
  boolean old = false;
  //toggle variables

  button(float x, float y, float _width, float _height, color background, color stroke, float rounding, String label) { 
    this.pos = new PVector(x, y);
    this.size = new PVector(_width, _height);
    this.fill = background;
    this.stroke = stroke;
    this.rounding = rounding;
    this.label = label;

    textSize = constrain((size.x/label.length())*1.6, 0, size.y / 2); //maybe make it a bit smaller? It is a bit too close to the edges of the button;done! 

    new_pos = new PVector(pos.x, pos.y);
    new_size = new PVector(size.x, size.y);
    new_textSize = textSize;
  }

  void show() {
    rectMode(CORNER);
    stroke(stroke);
    fill(fill);
    rect(pos.x, pos.y, size.x, size.y, rounding);
    textAlign(CENTER);
    fill(stroke);
    textSize(textSize);
    text(label, pos.x+size.x/2, pos.y+size.y/2 + textSize/3);
  }

  void animateChangeSize(){
    float changeRate = 0.3;
    pos.x += changeRate * (new_pos.x - pos.x);
    pos.y += changeRate * (new_pos.y - pos.y);

    size.x += changeRate * (new_size.x - size.x);
    size.y += changeRate * (new_size.y - size.y);

    textSize += changeRate * (new_textSize - textSize);
  }

  void hoveringAnimation() { // super rad change in size when hovered
    if (!smol){
      new_pos.x += (new_size.x - new_size.x * hoveringCoefficient)/2;
      new_pos.y += (new_size.y - new_size.y * hoveringCoefficient)/2;
      new_size.mult(hoveringCoefficient);
      new_textSize *= hoveringCoefficient;
      smol = true;
    }
  }

  void pressedAnimation() { // super rad change in size when pressed
    if (!bigger) {
      new_pos.x += (new_size.x - new_size.x*pressingCoefficient)/2;
      new_pos.y += (new_size.y - new_size.y*pressingCoefficient)/2;
      new_size.mult(pressingCoefficient);
      new_textSize *= pressingCoefficient;
      bigger = true;
    }
  }

  void killHoveringAnimation() { // idk if this is the best way to make this but whatever, it works, it kills the animation when the mouse stops hovering
    if (smol) {
      new_size.mult(1/hoveringCoefficient);
      new_pos.x -= (new_size.x - new_size.x * hoveringCoefficient)/2;
      new_pos.y -= (new_size.y - new_size.y * hoveringCoefficient)/2;
      new_textSize /= hoveringCoefficient;
      smol = false;
    }
  }

  void killPressedAnimation() { // the same things I said about the killHoveringAnimation apply for this one, except for the clicking 
    if (bigger) {
      new_size.mult(1/pressingCoefficient);
      new_pos.x -= (new_size.x - new_size.x*pressingCoefficient)/2;
      new_pos.y -= (new_size.y - new_size.y*pressingCoefficient)/2;
      new_textSize /= pressingCoefficient;
      bigger = false;
    }
  }

  boolean hovering() {
    if (mouseX > pos.x && mouseX < pos.x + size.x && mouseY > pos.y && mouseY < pos.y + size.y) {
      if (!mousePressed) {
        outside = false;
      }
      return true;
    } else {
      if (mousePressed) {
        outside = true;
      } else {
        outside = false;
      }
      return false;
    }
  }

  boolean pressed() {
    if (mousePressed && !outside) {
      return true;
    } else {
      return false;
    }
  }

  boolean pushButton() { // call this function if you want the button to act like a pushButton, on when clicked and the off when unclicked
    show();
    animateChangeSize();
    if (hovering()) {
      hoveringAnimation();
      if (pressed()) {
        pressedAnimation();
        return true;
      } else {
        killPressedAnimation();
      }
    } else {
      killHoveringAnimation();
      killPressedAnimation();
    }
    return false;
  }

  boolean toggle() { // call this function when you want the button to act like a toggle, you switch it on and off whenever you push it, really cool stuff 
    if (pushButton()) {
      if (!old) {
        old = true;
        state = !state;
      }
    } else {
      old = false;
    }
    return state;
  }
}
