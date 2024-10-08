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
int last_r_ping = 0, last_f_ping = 0;

byte targetID;

LinkedBlockingQueue<dataPacket> tx_queue = new LinkedBlockingQueue<dataPacket>();

DecimalFormat df = new DecimalFormat("0.00");

// packet structure : "SYNC", "CMD", "ID", "PLEN", "PAYLOAD", "CRC1", "CRC2"
byte CMD=0, PLEN=0, ID=0; /* CRC1, CRC2 */
byte[] rx_payload;
ParseState currentParseState = ParseState.START;
int rx_payload_index = 0;
byte[] empty_payload = {};

// maps
HashMap<String, boolean[]> prog_args = new HashMap<String, boolean[]>();
Map<Integer, String> state_map_rocket = new HashMap<>();
Map<Integer, String> state_map_filling = new HashMap<>();
HashMap<String, Byte> man_commands_map = new HashMap<String, Byte>();
HashMap<Byte, String> command_names = new HashMap<Byte, String>();

// controllers
Textfield[] textfields = new Textfield[3];
Textlabel log_display_rocket;
Textlabel log_display_filling;
Textlabel ack_display;
Textlabel log_stats;
Textlabel history;
ArrayDeque<String> history_deque;
Chart fillingChart, launchChart;
Textlabel pressureLabel, temperatureLabel, weightLabel;
Textlabel weight1Label, weight2Label, weight3Label, tankPressureLabel, chamberPressureLabel;
Textlabel ematch_label, chamber_temps_label, chamber_threshold_temp;
Textlabel he_label, n2o_label, line_label, tt_label, tb_label;
Toggle he_toggle, n2o_toggle, line_toggle, tt_toggle, tb_toggle, chamber_toggle;
Toggle status_toggle;

// status toggle
int status_toggle_state = 0;
int last_status_request = 0;
int last_status_id = 1;

float max_f = .01, max_l = .01;
int max_size = 100000;
int[] prog_inputs = new int[3];
int selected_index = -1;

boolean r_flash_log, f_flash_log;

Tab fillTab;
Tab launchTab;

// manual stuff
Toggle valve_toggle;
int valve_toggle_state = 0;
int last_open_valve = -1;

Textfield valve_ms;

List<Toggle> valve_toggles;
HashMap<Toggle, Byte> valve_toggle_map = new HashMap<Toggle, Byte>();

List<Textlabel> diagram_labels;
int valve_selected = -1;

void setup() {
  frameRate(60);
  fullScreen();
  background(bgColor);

  logDir = sketchPath() + "/logs/";
  String logFolder = "logs";
  String currentDirectory = sketchPath();
  String directoryPath = currentDirectory + File.separator + logFolder;
  File directory = new File(directoryPath);
  boolean directoryCreated = directory.mkdir();

  history_deque = new ArrayDeque<>(history_capacity);

  if (directoryCreated) {
    println("Directory created successfully at: " + directoryPath);
  } else {
    println("Failed to create directory. It may already exist at: " + directoryPath);
  }
  logDir = directoryPath + File.separator;
  file = new File(logDir+"log_"+day()+"_"+month()+"_"+year()+"_"+hour()+minute()+second()+".bin");

  font = createFont("arial", displayWidth*.013);
  cp5 = new ControlP5(this);
  
  init_data_objects(); // in data models tab
  setupColors(); // in global configs tab
  setupControllers(); // in setup controllers tab
  setupCharts(); // in chart functions tab
  init_log(); // log initialization
  setupDiagrams(); // in diagrams tab
  maps(); // in maps tab
}

void draw() {
  updateDiagrams(); // diagrams tab
  updateControllers(); // update controllers tab
  if (millis() - last_chart_time > chart_interval) {
    updateCharts();
    last_chart_time = millis();
  }
  request_status(); // comms tab
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
        cp5.getController(arg).setColor(defaultColor);
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
    AskData[] asks = {AskData.rocket_flags_state, AskData.kalman_data};
    short askShort = createAskDataMask(asks);
    byte[] asksBytes = ByteBuffer.allocate(2).putShort(askShort).array();
    send((byte)0x00, asksBytes);
  } else if (event.isFrom("Select ID")) {
    targetID = (byte) (event.getValue() + 1);
  } else if (event.isFrom("Abort")) {
    byte lastID = targetID;
    targetID = 4;
    send((byte)0x02, empty_payload);
    targetID = lastID;
  } else if (event.isFrom("Arm")) {
    send((byte)0x09, empty_payload);
  } else if (event.isFrom("Ready")) {
    send((byte)0x08, empty_payload);
    status_toggle.setState(true);
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
  } else if (event.isFrom("Start Manual")) {
    byte[] payload = {};
    send((byte)0x06, payload);
  } else if (event.isFrom("Change Valve State")) {
    if (valve_selected > -1) {
      byte lastID = targetID;
      targetID = 4;
      byte[] man_payload = {(byte) 0x04, (byte) valve_selected, (byte) valve_toggle_state};
      send((byte)0x07, man_payload);
      targetID = lastID;
    }
  }

  for (int i = 0; i < man_commands.size(); i++) {
    if (event.isFrom(man_commands.get(i))) {
      byte[] man_payload = {man_commands_map.get(man_commands.get(i))};
      send((byte)0x07, man_payload);
    }
  }
  if (valve_toggles != null) {
    for (Toggle toggle : valve_toggles) {
      if (event.isFrom(toggle.getName())) {
        int state = (int) event.getController().getValue();
        if (state == 1) {
          toggle.setColorForeground(color(0, 255, 0))
            .setColorBackground(color(0, 100, 0))
            .setColorActive(color(0, 255, 0));     // Green when on
        } else if (state == 0) {
          toggle.setColorForeground(color(255, 0, 0)) // Red when off
            .setColorActive(color(255, 0, 0))    // Red when off
            .setColorBackground(color(100, 0, 0));
        }
        byte[] payload = {(byte) 0x04, valve_toggle_map.get(toggle), (byte) state};
        byte lastID = targetID;
        targetID = 4;
        send((byte)0x07, payload);
        targetID = lastID;
      }
    }
  }
  if (event.isFrom("Select Valve")) {
    valve_selected = (int)event.getValue();
    print(valve_selected);
  } else if (event.isFrom("Reset Chart")) {
    fillingChart.setData("Pressure", new float[0]);
    fillingChart.setData("Temperature", new float[0]);
    fillingChart.setData("Weight", new float[0]);
  } else if (event.isTab()) {
    multi_tab_controllers(event.getTab().getName());
  } else if (event.isFrom("Reset")) {
    launchChart.setData("Altitude", new float[0]);
    launchChart.setData("Velocity", new float[0]);
    launchChart.setData("Acceleration", new float[0]);
  } else if (event.isFrom("Open valve")) {
    try {
      int valve_time = (int) Float.parseFloat(valve_ms.getText());
      print(valve_time);
      byte[] payload = {(byte) 0x05, (byte) valve_selected, (byte) valve_time};
      send((byte)0x07, payload);
    }
    catch (Exception e) {
      print("Valve open time empty\n");
    }
  } else if (event.isFrom("Mode Toggle")) {
    int mode = (int) event.getValue();
    mode = 1; // force dark : delete when light is done
    if (mode == 0) { // light mode
      abortColor = abortColorLight;
      stopColor = stopColorLight;
      unactiveColor = unactiveColorLight;
      defaultColor = defaultColorLight;
      bgColor = bgColorLight;
      fill_img = fill_img_light;
      labelColor = labelColorLight;
      labelColor2 = labelColor2Light;
    } else { // dark mode
      abortColor = abortColorDark;
      stopColor = stopColorDark;
      unactiveColor = unactiveColorDark;
      defaultColor = defaultColorDark;
      bgColor = bgColorDark;
      fill_img = fill_img_dark;
      labelColor = labelColorDark;
      labelColor2 = labelColor2Dark;
    }
    fill_diagram = loadImage(fill_img);
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
