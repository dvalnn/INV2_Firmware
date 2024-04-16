//fucking great library, I'm the fucking best, my best code happens when I am 2 cereal milk deep into the night at 3 am, I don't even know what the fuck I am saying right now
//what you should take from this comment is that I'm fucking awesome

//To add: the ability for the user to input a number, an exact number either through typing it or through typing a formula (like 4/5) which the program then calculates and substitutes
//with an approximate number (like 3 decimal places or something like that) **tough tough**
//Maybe define values for the slider to lock into like lock it when it reaches pi or something like that **tough**

//add points to which the slider snaps!!! // already added!!! maybe add a visual indicator of where the snaps are? //already added!!
// take a look at mouseWheel events and stuff, might be cool to change the values using the scrolling action

//Haven't been able make a "set" function with satisfying functionality outside the object library without making a global variable for the min and max
//having that into account there is some possible refactoring to do later in which there is no longer need for those arguments in most function calls inside the "coolSlider" 
//function; I sure hope to be able to find a better way of making this without the need for the globals (global within the object of course)
//ok, but if it works it works, and... we can't flaw it for something that doesn't show if you don't look under the hood, can we?, yes, yes, we can, but there are not goint to 
//spawn any problems/bugs from this way of making things, I'm sure there aren't. Also, I just don't like the way it is being made because it's not the c way but maybe it's the
//processing way ... ¯\_( ͡❛ ⏏ ͡❛)_/¯

class slider {
  boolean hovering = true;
  boolean hold = false;
  int diametre = 20;
  boolean first = true;
  boolean selected = false;
  boolean old = false;
  boolean oldKey = false;
  boolean doIt = false;
  int t;
  //receiving
  PVector coordinates;
  PVector ellipse; //doesn't quite receive it, just calculates it with the coordinates
  float len;
  boolean horizontal;
  //receiving


  //THIS IS BAD CODE, TERRIBLE IDEA, AGAINST ALL PRINCIPLES //you don't even know what those are, stop pretending to know what the fuck you are doing
  float min, max;

  slider (float x, float y, float len, boolean horizontal) {
    coordinates = new PVector(x, y);
    this.len = len;
    this.horizontal = horizontal;
    if (horizontal) {
      ellipse = new PVector (x + len/2, y);
    } else {
      ellipse = new PVector (x, y + len/2);
    }
  }

  float coolSlider(String lable, float value, float min, float max, boolean released) {
    this.min = min;
    this.max = max;
    if (first) {
      if (horizontal) {
        ellipse.x = map(value, min, max, coordinates.x, coordinates.x + len);
      } else {
        ellipse.y = map(value, min, max, coordinates.y, coordinates.y + len);
      }
      first = false;
    }
    show(lable, value);
    movement(released);
    return output(min, max);
  }

  int coolSlider(String lable, int value, int min, int max, boolean released) {
    this.min = min;
    this.max = max;
    if (first) {
      if (horizontal) {
        ellipse.x = map(value, min, max, coordinates.x, coordinates.x + len);
      } else {
        ellipse.y = map(value, min, max, coordinates.y, coordinates.y + len);
      }
      first = false;
    }
    movement(released, min, max);
    int output = round(output(min, max));
    snap(output, min, max);
    show(lable, value);
    return output;
  }

  void snap(int place, int min, int max) {
    if (horizontal) {
      ellipse.x = map(place, min, max, coordinates.x, coordinates.x + len);
    } else {
      ellipse.y = map(place, min, max, coordinates.y, coordinates.y + len);
    }
  }

  void show(String lable, float value) {
    stroke(255);
    fill(255);
    textSize(15);
    if (horizontal) {
      line(coordinates.x, coordinates.y, coordinates.x + len, coordinates.y);
      textAlign(CENTER, CENTER);
      text(lable, coordinates.x, coordinates.y - 30);
      text(nf(value, 0, 2), coordinates.x + len, coordinates.y -30);
    } else {
      line(coordinates.x, coordinates.y, coordinates.x, coordinates.y + len);
      textAlign(CENTER, CENTER);
      text(nf(value, 0, 2), coordinates.x, coordinates.y + len + 20);
      text(lable, coordinates.x, coordinates.y - 25);
    }
    if (hovering) {
      fill(100);
      stroke(100);
      diametre = 25;
    } else if (!selected) {
      fill(255);
      stroke(255);
      diametre = 20;
    } else {
      fill(100);
      stroke(100);
      diametre = 20;
    }
    circle(ellipse.x, ellipse.y, diametre);
  }

  void show(String lable, int value) {
    stroke(255);
    fill(255);
    textSize(15);
    if (horizontal) {
      line(coordinates.x, coordinates.y, coordinates.x + len, coordinates.y);
      textAlign(CENTER, CENTER);
      text(lable, coordinates.x, coordinates.y - 30);
      text(value, coordinates.x + len, coordinates.y -30);
    } else {
      line(coordinates.x, coordinates.y, coordinates.x, coordinates.y + len);
      textAlign(CENTER, CENTER);
      text(value, coordinates.x, coordinates.y + len + 20);
      text(lable, coordinates.x, coordinates.y - 25);
    }
    if (hovering) {
      fill(100);
      stroke(100);
      diametre = 25;
    } else if (!selected) {
      fill(255);
      stroke(255);
      diametre = 20;
    } else {
      fill(100);
      stroke(100);
      diametre = 20;
    }
    circle(ellipse.x, ellipse.y, diametre);
  }

  void movement(boolean released) {
    if ((pow(mouseX-ellipse.x, 2) + pow(mouseY-ellipse.y, 2)) <= pow(diametre/2, 2) && old == false) {
      hovering = true;
    } else {
      hovering = false;
    }

    if (mousePressed && hovering && old == false) {
      hold = true;
      selected = true;
      old = true;
    } else if (!mousePressed) {
      hold = false;
      old = false;
    } else if (mousePressed && old == false) {
      selected = false;
      old = true;
    }

    if (hold) {
      if (horizontal) {
        ellipse.x = constrain(mouseX, coordinates.x, coordinates.x + len);
      } else {
        ellipse.y = constrain(mouseY, coordinates.y, coordinates.y + len);
      }
    }

    if (selected) {
      if (keyPressed && (!oldKey || doIt) && !released) {
        if (!oldKey) {
          t = millis();
        }
        oldKey = true;
        switch(keyCode) {
        case UP:
          if (!horizontal) {
            ellipse.y = constrain(ellipse.y -= 0.7, coordinates.y, coordinates.y + len);
          }
          break;
        case DOWN:
          if (!horizontal) {
            ellipse.y = constrain(ellipse.y += 0.7, coordinates.y, coordinates.y + len);
          }
          break;
        case LEFT:
          if (horizontal) {
            ellipse.x = constrain(ellipse.x -= 0.7, coordinates.x, coordinates.x + len);
          }
          break;
        case RIGHT:
          if (horizontal) {
            ellipse.x = constrain(ellipse.x += 0.7, coordinates.x, coordinates.x + len);
          }
          break;
        }
      }
      if (!keyPressed) {
        oldKey = false;
        doIt = false;
      }
      if (keyPressed) {
        if (millis() - t > 500) {
          doIt = true;
        }
      }
    }
  }

  void movement(boolean released, int min, int max) {
    float movement = len / abs(max-min);
    showSnapSpots(movement);
    if ((pow(mouseX-ellipse.x, 2) + pow(mouseY-ellipse.y, 2)) <= pow(diametre/2, 2) && old == false) {
      hovering = true;
    } else {
      hovering = false;
    }

    if (mousePressed && hovering && old == false) {
      hold = true;
      selected = true;
      old = true;
    } else if (!mousePressed) {
      hold = false;
      old = false;
    } else if (mousePressed && old == false) {
      selected = false;
      old = true;
    }

    if (hold) {
      if (horizontal) {
        ellipse.x = constrain(mouseX, coordinates.x, coordinates.x + len);
      } else {
        ellipse.y = constrain(mouseY, coordinates.y, coordinates.y + len);
      }
    }

    if (selected) {
      if (keyPressed && (!oldKey || doIt) && !released) {
        if (!oldKey) {
          t = millis();
        }
        oldKey = true;
        switch(keyCode) {
        case UP:
          if (!horizontal) {
            ellipse.y = constrain(ellipse.y -= movement, coordinates.y, coordinates.y + len);
          }
          break;
        case DOWN:
          if (!horizontal) {
            ellipse.y = constrain(ellipse.y += movement, coordinates.y, coordinates.y + len);
          }
          break;
        case LEFT:
          if (horizontal) {
            ellipse.x = constrain(ellipse.x -= movement, coordinates.x, coordinates.x + len);
          }
          break;
        case RIGHT:
          if (horizontal) {
            ellipse.x = constrain(ellipse.x += movement, coordinates.x, coordinates.x + len);
          }
          break;
        }
      }
      if (!keyPressed) {
        oldKey = false;
        doIt = false;
      }
      if (keyPressed) {
        if (millis() - t > 500) {
          doIt = true;
        }
      }
    }
  }

  float output(float min, float max) {
    if (horizontal) {
      return map(ellipse.x, coordinates.x, coordinates.x + len, min, max);
    } else {
      return map(ellipse.y, coordinates.y, coordinates.y + len, min, max);
    }
  }

  void showSnapSpots(float stepSize) {
    rectMode(CENTER);
    fill(255);
    stroke(255);
    if (horizontal) {
      for (float i = 0; i < len; i += stepSize) {
        rect(coordinates.x + i, coordinates.y, 2, 8, 2);
      }
    } else {
      for (float i = 0; i < len; i += stepSize) {
        rect(coordinates.x, coordinates.y + i, 8, 2, 2);
      }
    }
  }


  void set(float value){
    if (horizontal){
      ellipse.x = map(value, min, max, coordinates.x, coordinates.x + len);
    } else {
      ellipse.y = map(value, min, max, coordinates.y, coordinates.y + len);
    }
  }
}
