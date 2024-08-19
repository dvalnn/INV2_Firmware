import processing.core.PApplet;
import controlP5.*;

public class Window extends PApplet {
  private ControlP5 m_cp5;

  Toggle valve_toggle;
  int valve_toggle_state = 0;
  int last_open_valve = -1;
  Textlabel man_log_display_rocket, man_log_display_filling;

  List<String> man_commands = Arrays.asList("Flash Log Start", "Flash Log Stop", "Flash IDs", "Loadcell Calibrate", "Loadcell Tare");
  HashMap<String, Byte> man_commands_map = new HashMap<String, Byte>();
  List<String> valves = Arrays.asList("VPU Valve", "Engine Valve", "He Valve", "N2O Valve", "Line Valve");
  boolean isWindowVisible = true;
  int valve_selected = -1;

  public void settings() {
    size((int)(displayWidth*.8), (int)(displayHeight*.8));
  }

  public void setup() {
    man_setup();
    man_window_open = true;
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
    } else if (event.isFrom("Manual Stop")) {
      send((byte)0x04, empty_payload);
    } else if (event.isFrom("Manual Abort")) {
      send((byte)0x02, empty_payload);
    } else if (event.isFrom("Manual Select ID")) {
      targetID = (byte) (event.getValue() + 1);
    }
  }

  public void man_setup() {
    background(100);
    windowResizable(true);
    font = createFont("arial", displayWidth*.013);
    m_cp5 = new ControlP5(this);

    man_commands_map.put("Flash Log Start", (byte) 0);
    man_commands_map.put("Flash Log Stop", (byte) 1);
    man_commands_map.put("Flash IDs", (byte) 2);
    man_commands_map.put("Loadcell Calibrate", (byte) 6);
    man_commands_map.put("Loadcell Tare", (byte) 7);

    m_cp5.addButton("Start Manual")
      .setPosition(width*.75, height*.1)
      .setSize((int)(width*.22), (int)(height*.05))
      .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
      .setFont(font);

    m_cp5.addScrollableList("Select Valve")
      .setPosition(width*.02, height*.02)
      .setSize((int)(width*.17), (int)(height*.5))
      .setBarHeight((int)(height*.05))
      .setItemHeight((int)(height*.05))
      .addItems(valves)
      .setFont(font)
      .setColorBackground(color(10, 10, 10))
      .setColorForeground(color(0, 144, 0))
      .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER);

    m_cp5.addButton("Change Valve State")
      .setPosition(width*.02, height*.5)
      .setSize((int)(width*.22), (int)(height*.05))
      .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
      .setFont(font);

    valve_toggle = m_cp5.addToggle("Valve Toggle")
      .setPosition(width*.02, height*.4)
      .setSize((int)(width*.05), (int)(width*.02))
      .setValue(false)
      .setMode(ControlP5.SWITCH)
      .setLabel("Valve State")
      .setFont(font)
      .setColorForeground(color(255, 0, 0))  // Red when off
      .setColorBackground(color(100, 0, 0))
      .setColorActive(color(255, 0, 0));

    m_cp5.addButton("Manual Stop")
      .setPosition(width*.75, height*.17)
      .setSize((int)(width*.22), (int)(height*.05))
      .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
      .setFont(font);

    m_cp5.addButton("Manual Abort")
      .setPosition(width*.75, height*.24)
      .setSize((int)(width*.22), (int)(height*.05))
      .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
      .setFont(font);

    for (int i = 0; i < man_commands.size(); i++) {
      m_cp5.addButton(man_commands.get(i))
        .setPosition(width*.75, height*.45 + height*.06*i)
        .setSize((int)(width*.22), (int)(height*.05))
        .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
        .setFont(font);
    }

    m_cp5.addScrollableList("Manual Select ID")
      .setPosition(displayWidth*.02, displayHeight*.5)
      .setSize((int)(displayWidth*.17), (int)(displayHeight*.5))
      .setBarHeight((int)(displayHeight*.05))
      .setItemHeight((int)(displayHeight*.05))
      .addItems(IDs)
      .setFont(font)
      .setColorBackground(color(50, 50, 50))
      .setColorForeground(color(0, 144, 0))
      .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER);

    man_log_display_rocket = m_cp5.addTextlabel("Manual Rocket Log")
      .setText("Logging Packet goes here")
      .setPosition(displayWidth*.17, displayHeight*.02)
      .setFont(font);

    man_log_display_filling = m_cp5.addTextlabel("Manual Filling Log")
      .setText("Logging Packet goes here")
      .setPosition(displayWidth*.37, displayHeight*.02)
      .setFont(font);
  }

  public void man_displayLogRocket() {
    String state = "\n" + "State: " + state_map_rocket.get(Byte.toUnsignedInt(rx_packet.payload[0]));
    String ttt = "\n" + "Tank Top Temperature: " + str(tank_top_temp);
    String tbt = "\n" + "Tank Bottom Temperature: " + str(tank_bot_temp);
    String ct1 = "\n" + "Chamber Temperature 1: " + str(chamber_temp1);
    String ct2 = "\n" + "Chamber Temperature 2: " + str(chamber_temp2);
    String ct3 = "\n" + "Chamber Temperature 3: " + str(chamber_temp3);
    String ttp = "\n" + "Tank Top Pressure: " + str(tank_top_press);
    String tbp = "\n" + "Tank Bottom Pressure: " + str(tank_bot_press);
    String rtp = "\n" + "Tank Pressure: " + str(r_tank_press);
    String rtl = "\n" + "Tank Liquid: " + str(r_tank_liquid);
    String w1 = "\n" + "Weight 1: " + str(r_weight1);
    String w2 = "\n" + "Weight 2: " + str(r_weight2);
    String w3 = "\n" + "Weight 3: " + str(r_weight3);

    String bools = String.format("%8s", Integer.toBinaryString(r_bools & 0xFF)).replace(' ', '0');
    String log_running = "\nLog Running: " + bools.substring(0, 1);
    String tt_valve = "\nTank Top Valve: " + bools.substring(1, 2);
    String tb_valve = "\nTank Bottom Valve: " + bools.substring(2, 3);
    String tactiles = "\nTactiles: " + bools.substring(3);

    man_log_display_rocket.setText("Rocket" + state + ttt + tbt + ct1 + ct2 + ct3 + ttp + tbp + rtp + rtl + log_running + tt_valve + tb_valve + tactiles + w1 + w2 + w3);
  }

  public void man_displayLogFilling() {
    String state = "\n" + "State: " + state_map_filling.get(Byte.toUnsignedInt(rx_packet.payload[0]));
    String ftp = "\n" + "Tank Temperature: " + str(f_tank_press);
    String ftl = "\n" + "Tank Liquid: " + str(f_tank_liquid);
    String ht = "\n" + "He Temperature: " + str(he_temp);
    String nt = "\n" + "N2O Temperature: " + str(n2o_temp);
    String lt = "\n" + "Line Temperature: " + str(line_temp);
    String hp = "\n" + "He Pressure: " + str(he_press);
    String np = "\n" + "N2O Pressure: " + str(n2o_press);
    String lp = "\n" + "Line Pressure: " + str(line_press);
    String ev = "\n" + "eMatch reading: " + str(ematch_v);
    String w1 = "\n" + "Weight 1: " + str(f_weight1);

    String bools = String.format("%8s", Integer.toBinaryString(f_bools & 0xFF)).replace(' ', '0');
    String log_running = "\nLog Running: " + bools.substring(0, 1);
    String he_valve = "\nHelium Valve: " + bools.substring(1, 2);
    String n2o_valve = "\nN2O Valve: " + bools.substring(2, 3);
    String line_valve = "\nLine Valve: " + bools.substring(3, 4);

    log_display_filling.setText("Filling Station\n" + state + ftp + ftl + ht + nt + lt + hp + np + lp + ev + w1 + log_running + he_valve + n2o_valve + line_valve);
  }
  public void draw() {
    background(100);
  }

  //@Override
  //  public void exit() {
  //  man_window_open = false;
  //  this.dispose();
  //}

  @Override
    public void exit() {
    // Hide the window instead of closing it
    isWindowVisible = false;
    this.getSurface().setVisible(false);
  }

  public void showWindow() {
    isWindowVisible = true;
    this.getSurface().setVisible(true);
  }
}
