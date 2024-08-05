import processing.core.PApplet;
import controlP5.*;

public class Window extends PApplet {
  private boolean open = false;
  private ControlP5 cp5;
  
  PFont font;
 
  public void settings() {
    size((int)(displayWidth*.7), (int)(displayHeight*.7));
  }

  public void setup() {
    font = createFont("arial", displayWidth*.013);
    cp5 = new ControlP5(this);
    background(50);
    
    cp5.addTextlabel("Close Warning")
    .setText("Do not close! Only minimize")
    .setPosition(width*.02, height*.02)
    .setFont(font);
  }

  public void draw() {
    // Nothing needed here if the UI is static
  }

  public void open() {
    if (!open) {
      PApplet.runSketch(new String[]{"Window"}, this);
      open = true;
    }
  }

  public void close() {
    if (open) {
      dispose();
      open = false;
    }
  }

  public boolean isOpen() {
    return open;
  }

  @Override
  public void exit() {
    close();
  }
}
