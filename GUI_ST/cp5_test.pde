import controlP5.*;
import processing.serial.*;
import java.util.*;

ControlP5 cp5;
PFont font;
Serial myPort; // For serial communication
dataPacket tx_packet;
HashMap<String, boolean[]> prog_args = new HashMap<String, boolean[]>();

Textfield[] textfields = new Textfield[5];
int[] prog_inputs = new int[5];
int selected_index = -1;
byte[] prog_cmds = {(byte)0x00, (byte)0x01, (byte)0x02, (byte)0x03, (byte)0x04};
List<String> programs = Arrays.asList("fp1", "fp2", "fp3", "rp1", "rp2");
List<String> vars = Arrays.asList("tp1", "tp2", "tp3", "tl1", "tl2");
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
  font = createFont("arial", 30);
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


  cp5.addScrollableList("program")
    .setPosition(100, 100)
    .setSize(300, 500)
    .setBarHeight(50)
    .setItemHeight(50)
    .addItems(programs)
    .setFont(font)
    .setColor(blue)
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER);

  // Textfields for parameters and limits
  for (int i = 1; i <= 3; i++) {
    textfields[i-1] = cp5.addTextfield("tp" + i)
      .setAutoClear(false)
      .setColor(blue)
      .setPosition(300 + i * 170, 100)
      .setSize(150, 50)
      .setFont(font)
      .setInputFilter(ControlP5.FLOAT);
  }

  for (int i = 1; i <= 2; i++) {
    textfields[i+2] = cp5.addTextfield("tl" + i)
      .setAutoClear(false)
      .setColor(blue)
      .setPosition(810 + i * 170, 100)
      .setSize(150, 50)
      .setFont(font)
      .setInputFilter(ControlP5.FLOAT);
  }

  // Send button
  cp5.addButton("Send")
    .setPosition(1500, 100)
    .setSize(100, 50)
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);

  cp5.addButton("Start Filling")
    .setPosition(1500, 160)
    .setSize(250, 50)
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);

  cp5.addButton("Stop")
    .setPosition(1500, 220)
    .setSize(100, 50)
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);

  cp5.addButton("Resume")
    .setPosition(1500, 280)
    .setSize(150, 50)
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);

  // List available serial ports and add them to a new ScrollableList
  List<String> portNames = Arrays.asList(Serial.list());
  cp5.addScrollableList("serialPort")
    .setPosition(50, 600) // Adjust position based on your UI layout
    .setSize(250, 100)
    .setBarHeight(50)
    .setItemHeight(50)
    .addItems(portNames)
    .setFont(font)
    .setColorBackground(color(50, 50, 50))
    .setColorForeground(color(0, 144, 0))
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER);
}

void draw() {
  background(0);
}

public void controlEvent(ControlEvent event) {
  if (event.isFrom("serialPort")) {
    int index = (int)event.getValue();
    String selectedPort = Serial.list()[index];
    println("Selected serial port: " + selectedPort);

    // Open the selected serial port here. Close any previously opened port first.
    if (myPort != null) {
      myPort.stop();
    }
    myPort = new Serial(this, selectedPort, 9600); // Adjust baud rate as needed
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
      send((byte) 0x02, payload); // placeholder command value -> replace with EXEC_PROG command value
    } else {
      print("No program selected");
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
  }
  else if (event.isFrom("Stop")) {
    byte[] placeholder = {};
    send((byte)0x03, placeholder);
  }
  else if (event.isFrom("Start Filling")) {
    byte[] placeholder = {};
    send((byte)0x04, placeholder);
  }
  else if (event.isFrom("Resume")) {
    byte[] placeholder = {};
    send((byte)0x0a, placeholder);
  }
}

void send(byte command, byte[] payload) {
  println(command, payload);
  tx_packet = new dataPacket(command, payload);
  tx_packet.logPacket();
  if (myPort != null) {
    println(tx_packet.getPacket());
    myPort.write(tx_packet.getPacket());
    
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
