import processing.core.PApplet;
import controlP5.*;

public class Window extends PApplet {
  private boolean open = false;
  private ControlP5 cp5;

  PFont font;

  Toggle valve_toggle;
  int valve_toggle_state = 0;

  List<String> man_commands = Arrays.asList("Flash Log Start", "Flash Log Stop", "Flash IDs", "Loadcell Calibrate", "Loadcell Tare");
  HashMap<String, Byte> man_commands_map = new HashMap<String, Byte>();
  List<String> valves = Arrays.asList("VPU Valve", "Engine Valve", "He Valve", "N2O Valve", "Line Valve");

  int valve_selected = -1;

  public void settings() {
    size((int)(displayWidth*.7), (int)(displayHeight*.7));
  }

  public void setup() {
    background(100);
    font = createFont("arial", displayWidth*.013);
    cp5 = new ControlP5(this);

    man_commands_map.put("Flash Log Start", (byte) 0);
    man_commands_map.put("Flash Log Stop", (byte) 1);
    man_commands_map.put("Flash IDs", (byte) 2);
    man_commands_map.put("Loadcell Calibrate", (byte) 6);
    man_commands_map.put("Loadcell Tare", (byte) 7);
    cp5.addTextlabel("Close Warning")
      .setText("Do not close! Only minimize")
      .setPosition(width*.02, height*.02)
      .setFont(font);

    cp5.addButton("Start Manual")
      .setPosition(width*.75, height*.1)
      .setSize((int)(width*.22), (int)(height*.05))
      .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
      .setFont(font);

    cp5.addScrollableList("Select Valve")
      .setPosition(width*.1, height*.1)
      .setSize((int)(width*.17), (int)(height*.5))
      .setBarHeight((int)(height*.05))
      .setItemHeight((int)(height*.05))
      .addItems(valves)
      .setFont(font)
      .setColorBackground(color(10, 10, 10))
      .setColorForeground(color(0, 144, 0))
      .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER);

    cp5.addButton("Change Valve State")
      .setPosition(width*.1, height*.7)
      .setSize((int)(width*.22), (int)(height*.05))
      .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
      .setFont(font);

    valve_toggle = cp5.addToggle("Valve Toggle")
      .setPosition(width*.1, height*.5)
      .setSize((int)(width*.05), (int)(width*.02))
      .setValue(false)
      .setMode(ControlP5.SWITCH)
      .setLabel("Valve State")
      .setFont(font)
      .setColorForeground(color(255, 0, 0))  // Red when off
      .setColorBackground(color(100, 0, 0))
      .setColorActive(color(255, 0, 0));

    for (int i = 0; i < man_commands.size(); i++) {
      cp5.addButton(man_commands.get(i))
        .setPosition(width*.75, height*.45 + height*.06*i)
        .setSize((int)(width*.22), (int)(height*.05))
        .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
        .setFont(font);
    }
  }


  public void controlEvent(ControlEvent event) {
    for (int i = 0; i < man_commands.size(); i++) {
      if (event.isFrom(man_commands.get(i))) {
        byte[] man_payload = {man_commands_map.get(man_commands.get(i))};
        send((byte)0x07, man_payload);
      }
    }
    if (event.isFrom("Start Manual")) {
      send((byte)0x06, empty_payload);
    } else if (event.isFrom("Change Valve State")) {
      if (valve_selected > -1) {
        byte[] man_payload = {(byte) 0x04, (byte) valve_selected, (byte) valve_toggle_state};
        send((byte)0x07, man_payload);
      }
    } else if (event.isFrom("Select Valve")) {
      valve_selected = (int)event.getValue();
    } else if (event.isFrom("")) {
    } else if (event.isFrom(valve_toggle)) {
      valve_toggle_state = (int) event.getController().getValue();
      if (valve_toggle_state == 1) {
        valve_toggle.setColorForeground(color(0, 255, 0))
          .setColorBackground(color(0, 100, 0))
          .setColorActive(color(0, 255, 0));     // Green when on
      } else if (valve_toggle_state == 0) {
        valve_toggle.setColorForeground(color(255, 0, 0))// Red when off
          .setColorActive(color(255, 0, 0))    // Red when off
          .setColorBackground(color(100, 0, 0));
      }
    }
  }
  public void draw() {
    // Nothing needed here if the UI is static
    background(100);
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
