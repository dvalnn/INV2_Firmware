import controlP5.*;
import processing.serial.*;
import java.util.*;
import java.util.concurrent.LinkedBlockingQueue;
import java.nio.*;
import java.text.DecimalFormat;

ControlP5 cp5;
PFont font;

Serial myPort; // For serial communication
dataPacket tx_packet;
String selectedPort;
dataPacket rx_packet;
boolean port_selected = false;

int last_read_time = 0;

int last_r_log_time = 0;
float r_log_rate = 0;
int last_f_log_time = 0;
float f_log_rate = 0;

int last_received_log_id = 0;
int log_packet_loss = 0;
int ack_packet_loss = 0;
byte last_cmd_sent = (byte) 0xff;
int last_cmd_sent_time = 0;
int last_chart_time = 0;
int last_status_time = 0;

byte targetID;

LinkedBlockingQueue<byte[]> tx_queue = new LinkedBlockingQueue<byte[]>();

DecimalFormat df = new DecimalFormat("#.00");

// packet structure : "SYNC", "CMD", "ID", "PLEN", "PAYLOAD", "CRC1", "CRC2"
byte CMD=0, PLEN=0, ID=0; /* CRC1, CRC2 */
byte[] rx_payload;
ParseState currentParseState = ParseState.START;
int rx_payload_index = 0;
byte[] empty_payload = {};

HashMap<String, boolean[]> prog_args = new HashMap<String, boolean[]>();
Map<Integer, String> state_map_rocket = new HashMap<>();
Map<Integer, String> state_map_filling = new HashMap<>();

Textfield[] textfields = new Textfield[3];
Textlabel log_display_rocket;
Textlabel log_display_filling;
Textlabel ack_display;
Textlabel log_stats;
Chart fillingChart, launchChart;

Toggle status_toggle;
int status_toggle_state = 0;
int last_status_request = 0;

short tank_top_temp, tank_bot_temp, chamber_temp1, chamber_temp2, chamber_temp3, tank_top_press, tank_bot_press, r_tank_press, r_tank_liquid, r_weight1, r_weight2, r_weight3, r_chamber_press;
byte r_bools;
short f_tank_press, f_tank_liquid, he_temp, n2o_temp, line_temp, he_press, n2o_press, line_press, ematch_v, f_bools;
int f_weight1 = 4;
float max_f = 1, max_l = 1;
int max_size = 100000;
int[] prog_inputs = new int[3];
int selected_index = -1;

float mock_value1, mock_value2, mock_value3, mock_value4;

boolean r_flash_log, f_flash_log;

Tab fillTab;
Tab launchTab;

// manual stuff
Toggle valve_toggle;
int valve_toggle_state = 0;
int last_open_valve = -1;

Textfield valve_ms;
Textlabel pressureLabel, liquidLabel, temperatureLabel, weightLabel;
Textlabel weight1Label, weight2Label, weight3Label, tankPressureLabel, chamberPressureLabel;
Textlabel ematch_label;
Textlabel he_label, n2o_label, line_label, tt_label, tb_label;

//Textfield

List<Textlabel> diagram_labels;
List<String> man_commands = Arrays.asList("Flash Log Start", "Flash Log Stop", "Flash IDs", "Loadcell Calibrate", "Loadcell Tare");
HashMap<String, Byte> man_commands_map = new HashMap<String, Byte>();
List<String> valves = Arrays.asList("VPU Valve", "Engine Valve", "He Valve", "N2O Valve", "Line Valve");
int valve_selected = -1;

void setup() {
  frameRate(60);
  fullScreen();
  background(bgColor);

  logDir = sketchPath() + "/logs/";
  file = new File(logDir+"log_"+day()+"_"+month()+"_"+year()+"_"+hour()+minute()+second()+".bin");

  font = createFont("arial", displayWidth*.013);
  cp5 = new ControlP5(this);

  setupColors();


  boolean[] _bl1 = {true, true, false};
  prog_args.put("Safety Pressure", _bl1);
  boolean[] _bl2 = {true, false, false};
  prog_args.put("Purge Pressure", _bl2);
  boolean[] _bl3 = {false, false, true};
  prog_args.put("Purge Liquid", _bl3);
  boolean[] _bl4 = {true, false, false};
  prog_args.put("Fill He", _bl4);
  boolean[] _bl5 = {false, true, true};
  prog_args.put("Fill N2O", _bl5);
  boolean[] _bl6 = {true, false, false};
  prog_args.put("Purge Line", _bl6);

  state_map_rocket.put(0, "IDLE");
  state_map_rocket.put(1, "FUELING");
  state_map_rocket.put(2, "MANUAL");
  state_map_rocket.put(3, "SAFETY_PRESSURE");
  state_map_rocket.put(4, "PURGE_PRESSURE");
  state_map_rocket.put(5, "PURGE_LIQUID");
  state_map_rocket.put(6, "SAFETY_PRESSURE_ACTIVE");
  state_map_rocket.put(7, "READY");
  state_map_rocket.put(8, "ARMED");
  state_map_rocket.put(9, "LAUNCH");
  state_map_rocket.put(10, "ABORT");
  state_map_rocket.put(11, "IMU_CALIB");

  state_map_filling.put(0, "IDLE");
  state_map_filling.put(1, "FUELING");
  state_map_filling.put(2, "MANUEL");
  state_map_filling.put(3, "FILL_He");
  state_map_filling.put(4, "FILL_N2O");
  state_map_filling.put(5, "PURGE_LINE");
  state_map_filling.put(6, "SAFETY");
  state_map_filling.put(7, "ABORT");
  state_map_filling.put(8, "READY");
  state_map_filling.put(9, "ARMED");
  state_map_filling.put(10, "FIRE");
  state_map_filling.put(11, "LAUNCH");

  man_commands_map.put("Flash Log Start", (byte) 0);
  man_commands_map.put("Flash Log Stop", (byte) 1);
  man_commands_map.put("Flash IDs", (byte) 2);
  man_commands_map.put("Loadcell Calibrate", (byte) 7);
  man_commands_map.put("Loadcell Tare", (byte) 8);

  setupControllers(); // in setup controllers tab
  setupCharts(); // in chart functions tab
  init_log(); // log initialization
  setupDiagrams();
}

void draw() {
  updateDiagrams();
  if (millis() - last_chart_time > chart_interval) {
    updateCharts(r_tank_press, r_tank_liquid, tank_top_temp, f_weight1, r_weight1, r_weight2, r_weight3, r_tank_press, r_chamber_press);
    last_chart_time = millis();
  }
  if (millis() - last_status_request > status_interval && status_toggle_state == 1) {
    send((byte)0x00, empty_payload);
    last_status_request = millis();
  }
  if (last_cmd_sent != (byte)0xff) {
    if (millis() - last_cmd_sent_time > packet_loss_timeout) {
      ack_packet_loss++;
      last_cmd_sent = (byte)0xff;
      updateLogStats();
      ack_display.setText("Last Ack Received:\nFAIL");
    }
  }
  
  if (r_flash_log == true) {
      fill(0, 255, 0);
    } else {
      fill(255, 0, 0);
    }
    circle(width*.79, height*.72, height*.018);
    
    if (f_flash_log == true) {
      fill(0, 255, 0);
    } else {
      fill(255, 0, 0);
    }
    circle(width*.79, height*.81, height*.018);
}

public void controlEvent(ControlEvent event) {
  if (event.isFrom("serialPort")) {
    port_selected = false;
    int index = (int)event.getValue();
    selectedPort = Serial.list()[index];
    println("Selected serial port: " + selectedPort);
    // Open the selected serial port here. Close any previously opened port first.
    if (myPort != null) {
      myPort.stop();
    }
    thread("serialThread");
  } else if (event.isFrom("Send")) {
    for (int i = 0; i < 3; i++) {
      String input = textfields[i].getText();
      try {
        prog_inputs[i] = Integer.parseInt(input);
        println(input);
      }
      catch (NumberFormatException e) {
        prog_inputs[i] = -1;
        println("invalid input");
      }
    }
    if (selected_index >= 0) {
      byte[] payload = {prog_cmds[selected_index],
        (byte) ((prog_inputs[0] >> 8) & 0xff),
        (byte)(prog_inputs[0] & 0xff),
        (byte)((prog_inputs[1] >> 8) & 0xff),
        (byte) (prog_inputs[1] & 0xff),
        (byte)((prog_inputs[2] >> 8) & 0xff),
        (byte) (prog_inputs[2] & 0xff)};
      println(payload);
      send((byte) 0x03, payload);
    } else {
      println("No program selected");
    }
  } else if (event.isFrom("program")) {
    selected_index = (int) event.getValue();
    String program = programs.get(selected_index);
    for (int i = 0; i < 5; i++) {
      String arg = vars.get(i);
      if (prog_args.get(program)[i]) {
        cp5.getController(arg).setColor(colors2);
      } else {
        cp5.getController(arg).setColor(unactiveColor);
      }
    }
    println(program);
  } else if (event.isFrom("Stop")) {
    send((byte)0x04, empty_payload);
  } else if (event.isFrom("Start Filling")) {
    send((byte)0x05, empty_payload);
  } else if (event.isFrom("Resume")) {
    send((byte)0x0b, empty_payload);
  } else if (event.isFrom("Status")) {
    send((byte)0x00, empty_payload);
  } else if (event.isFrom("Select ID")) {
    targetID = (byte) (event.getValue() + 1);
  } else if (event.isFrom("Abort")) {
    send((byte)0x02, empty_payload);
  } else if (event.isFrom("Arm")) {
    send((byte)0x09, empty_payload);
  } else if (event.isFrom("Ready")) {
    send((byte)0x08, empty_payload);
  } else if (event.isFrom("Fire")) {
    send((byte)0x0c, empty_payload);
  } else if (event.isFrom("Allow Launch")) {
    send((byte)0x0a, empty_payload);
  } else if (event.isFrom(status_toggle)) {
    status_toggle_state = (int) event.getController().getValue();
    if (status_toggle_state == 1) {
      status_toggle.setColorForeground(color(0, 255, 0))
        .setColorBackground(color(0, 100, 0))
        .setColorActive(color(0, 255, 0));     // Green when on
    } else if (status_toggle_state == 0) {
      status_toggle.setColorForeground(color(255, 0, 0))// Red when off
        .setColorActive(color(255, 0, 0))    // Red when off
        .setColorBackground(color(100, 0, 0));
    }
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
  } else if (event.isFrom("Reset Chart")) {
    fillingChart.setData("Pressure", new float[0]);
    fillingChart.setData("Liquid", new float[0]);
    fillingChart.setData("Temperature", new float[0]);
    fillingChart.setData("Weight", new float[0]);
  } else if (event.isTab()) {
    multi_tab_controllers(event.getTab().getName());
  } else if (event.isFrom("Reset")) {
    launchChart.setData("Weight 1", new float[0]);
    launchChart.setData("Weight 2", new float[0]);
    launchChart.setData("Weight 3", new float[0]);
    launchChart.setData("Tank Pressure", new float[0]);
    launchChart.setData("Chamber Pressure", new float[0]);
  } else if (event.isFrom("Open valve")) {
    try {
    float valve_time = Float.parseFloat(valve_ms.getText());
    print(valve_time);
    byte[] payload = {(byte) 0x05, (byte) valve_time};
    send((byte)0x07, payload);
    } catch (Exception e) {
      print("Valve open time empty\n");
    }
  }
}


// This method ensures the serial port is closed properly when the program is exited
void stop() {
  try {
    logStream.close();
  }
  catch (IOException e) {
    println(e);
  }
  if (myPort != null) {
    myPort.stop();
  }
  super.stop();
}
