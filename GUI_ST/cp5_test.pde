import controlP5.*;
import processing.serial.*;
import java.util.*;
import java.util.concurrent.LinkedBlockingQueue;

ControlP5 cp5;
PFont font;

Serial myPort; // For serial communication
dataPacket tx_packet;
int baudRate = 115200;
String selectedPort;
dataPacket rx_packet;
boolean port_selected = false;
int last_read_time = 0;
int TIMEOUT = 250;

byte MyID = (byte) 0x00;
byte targetID;

LinkedBlockingQueue<byte[]> tx_queue = new LinkedBlockingQueue<byte[]>();

// GUI Positions and Sizes
float button_x = .78; // * displayWidth
float button_height = .04; // * displayHeight


// packet structure : "SYNC", "CMD", "ID", "PLEN", "PAYLOAD", "CRC1", "CRC2"
byte CMD=0, PLEN=0, ID=0; /* CRC1, CRC2 */
byte[] rx_payload = new byte[100];
ParseState currentParseState = ParseState.START;
int rx_payload_index = 0;
byte[] empty_payload = {};

HashMap<String, boolean[]> prog_args = new HashMap<String, boolean[]>();

Textfield[] textfields = new Textfield[5];
Textlabel log_display_rocket;
Textlabel log_display_filling;
Chart rocketChart;
Chart fillingChart;

float rp, rl, fp, fl;
int max_r, max_f;

int[] prog_inputs = new int[5];
int selected_index = -1;
byte[] prog_cmds = {(byte)0x01, (byte)0x02, (byte)0x03, (byte)0x01, (byte)0x02};

List<String> programs = Arrays.asList("fp1", "fp2", "fp3", "rp1", "rp2");
List<String> vars = Arrays.asList("tp1", "tp2", "tp3", "tl1", "tl2");
List<String> IDs = Arrays.asList( "1 : Rocket", "2 : Filling Station", "3 : Broadcast");
CColor gray = new CColor();
CColor blue = new CColor();

void setup() {
  //inits
  initializeWriters();
  frameRate(60);
  //windowRatio(1920, 1080);
  //size(displayWidth, displayHeight);
  fullScreen();
  background(0);
  font = createFont("arial", displayWidth*.016);
  cp5 = new ControlP5(this);
  gray.setForeground(color(0, 0, 0))
    .setBackground(color(50, 50, 50));
  blue.setForeground(color(50, 60, 150))
    .setBackground(color(0, 31, 84));

  // tp1, tp2, tp3, tl1, tl2
  boolean[] _bl1 = {true, false, false, false, false};
  prog_args.put("fp1", _bl1);
  boolean[] _bl2 = {false, true, false, true, true};
  prog_args.put("fp2", _bl2);
  boolean[] _bl3 = {false, false, true, false, false};
  prog_args.put("fp3", _bl3);
  boolean[] _bl4 = {true, true, false, false, false};
  prog_args.put("rp1", _bl4);
  boolean[] _bl5 = {false, false, true, false, false};
  prog_args.put("rp2", _bl5);

  setupControllers(); // in setup controllers tab
  setupCharts(); // in chart functions tab  
  
}

void draw() {
  background(0);
  updateCharts(rp, rl, fp, fl);
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
    for (int i = 0; i < 5; i++) {
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
        (byte) (prog_inputs[2] & 0xff),
        (byte)((prog_inputs[3] >> 8) & 0xff),
        (byte) (prog_inputs[3] & 0xff),
        (byte)((prog_inputs[4] >> 8) & 0xff),
        (byte) (prog_inputs[4] & 0xff)};
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
        cp5.getController(arg).setColor(blue);
      } else {
        cp5.getController(arg).setColor(gray);
      }
    }
    println(program);
  } else if (event.isFrom("Stop")) {
    send((byte)0x04, empty_payload);
  } else if (event.isFrom("Start Filling")) {
    send((byte)0x05, empty_payload);
  } else if (event.isFrom("Resume")) {
    send((byte)0x0a, empty_payload);
  } else if (event.isFrom("Status")) {
    send((byte)0x00, empty_payload);
  } else if (event.isFrom("Select ID")) {
    targetID = (byte) (event.getValue() + 1);
  } else if (event.isFrom("Abort")) {
    send((byte)0x02, empty_payload);
  } else if (event.isFrom("Arm")) {
    send((byte)0x08, empty_payload);
  }
}

void send(byte command, byte[] payload) {
  println(command, payload);
  tx_packet = new dataPacket(command, targetID, payload);
  tx_packet.logPacket();
  if (myPort != null) {
    if (targetID > 0) {
      byte[] packet = tx_packet.getPacket();
      println(packet);
      tx_queue.add(packet);
    } else {
      println("Target ID not selected");
    }
  } else {
    println("No serial port selected!");
  }
}

// This method ensures the serial port is closed properly when the program is exited
void stop() {
  flushWriters();
  if (myPort != null) {
    myPort.stop();
  }
  super.stop();
}
