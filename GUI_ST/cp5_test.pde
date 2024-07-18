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

HashMap<String, boolean[]> prog_args = new HashMap<String, boolean[]>();
Textfield[] textfields = new Textfield[5];
Textlabel log_display_rocket;
Textlabel log_display_filling;

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


  cp5.addScrollableList("program")
    .setPosition(displayWidth*.05, displayHeight*.1)
    .setSize((int)(displayWidth*.15), (int)(displayHeight*.46))
    .setBarHeight((int)(displayHeight*.05))
    .setItemHeight((int)(displayHeight*.05))
    .addItems(programs)
    .setFont(font)
    .setColor(blue)
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER);

  // Textfields for parameters and limits
  for (int i = 1; i <= 3; i++) {
    textfields[i-1] = cp5.addTextfield("tp" + i)
      .setAutoClear(false)
      .setColor(blue)
      .setPosition(displayWidth*.15 + i * displayWidth*.09, displayHeight*.09)
      .setSize((int)(displayWidth*.08), (int)(displayHeight*.05))
      .setFont(font)
      .setInputFilter(ControlP5.FLOAT);
  }

  for (int i = 1; i <= 2; i++) {
    textfields[i+2] = cp5.addTextfield("tl" + i)
      .setAutoClear(false)
      .setColor(blue)
      .setPosition(displayWidth*.42 + i * displayWidth*.09, displayHeight*.09)
      .setSize((int)(displayWidth*.08), (int)(displayHeight*.05))
      .setFont(font)
      .setInputFilter(ControlP5.FLOAT);
  }

  // Send button
  cp5.addButton("Send")
    .setPosition(displayWidth*button_x, displayHeight*.05)
    .setSize((int)(displayWidth*.1), (int)(displayHeight*.04))
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);

  cp5.addButton("Start Filling")
    .setPosition(displayWidth*button_x, displayHeight*.1)
    .setSize((int)(displayWidth*.13), (int)(displayHeight*button_height))
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);

  cp5.addButton("Stop")
    .setPosition(displayWidth*button_x, displayHeight*.15)
    .setSize((int)(displayWidth*.05), (int)(displayHeight*button_height))
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);

  cp5.addButton("Resume")
    .setPosition(displayWidth*button_x, displayHeight*.20)
    .setSize((int)(displayWidth*.07), (int)(displayHeight*button_height))
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);

  cp5.addButton("Status")
    .setPosition(displayWidth*button_x, displayHeight*.25)
    .setSize((int)(displayWidth*.07), (int)(displayHeight*button_height))
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);

  cp5.addButton("Abort")
    .setPosition(displayWidth*button_x, displayHeight*.30)
    .setSize((int)(displayWidth*.07), (int)(displayHeight*button_height))
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);
 cp5.addButton("Arm")
    .setPosition(displayWidth*button_x, displayHeight*.35)
    .setSize((int)(displayWidth*.07), (int)(displayHeight*button_height))
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);

  // List available serial ports and add them to a new ScrollableList
  List<String> portNames = Arrays.asList(Serial.list());
  cp5.addScrollableList("serialPort")
    .setPosition(displayWidth*.05, displayHeight*.5)
    .setSize((int)(displayWidth*.2), (int)(displayHeight*.46))
    .setBarHeight((int)(displayHeight*.05))
    .setItemHeight((int)(displayHeight*.05))
    .addItems(portNames)
    .setFont(font)
    .setColorBackground(color(50, 50, 50))
    .setColorForeground(color(0, 144, 0))
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER);

  cp5.addScrollableList("Select ID")
    .setPosition(displayWidth*button_x, displayHeight*.5)
    .setSize((int)(displayWidth*.17), (int)(displayHeight*.5))
    .setBarHeight((int)(displayHeight*.05))
    .setItemHeight((int)(displayHeight*.05))
    .addItems(IDs)
    .setFont(font)
    .setColorBackground(color(50, 50, 50))
    .setColorForeground(color(0, 144, 0))
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER);

  log_display_rocket = cp5.addTextlabel("Rocket Log")
    .setText("Logging Packet goes here")
    .setPosition(displayWidth*.15, displayHeight*.8)
    .setFont(font);
  log_display_filling = cp5.addTextlabel("Filling Log")
    .setText("Logging Packet goes here")
    .setPosition(displayWidth*.6, displayHeight*.8)
    .setFont(font);
}

void draw() {
  background(0);
}

void serialThread() {
  myPort = new Serial(this, selectedPort, baudRate);
  port_selected = true;
  myPort.clear();
  while (port_selected) {
    while (myPort.available() > 0) {
      byte rx_byte = (byte) myPort.read();
      parseIncomingByte(rx_byte);
    }

    // Send packets in queue
    byte[] head = tx_queue.peek();
    while (head != null) {
      myPort.write(head);
      tx_queue.remove();
      head = tx_queue.peek();
    }
  }
  // log_display.setText(str(selected_index));
}

void parseIncomingByte(byte rx_byte) {

  if (last_read_time == 0 || millis() - last_read_time > TIMEOUT) {
    currentParseState = ParseState.START;
  }
  last_read_time = millis();
  switch(currentParseState) {
  case START:
    if (rx_byte == (byte) 0x55) {
      currentParseState = ParseState.CMD;
    } else {
      // println("Start byte not received");
    }
    break;
  case CMD:
    CMD = rx_byte;
    currentParseState = ParseState.ID;
    //println("Command Received: " + (int) CMD);
    break;
  case ID:
    ID = rx_byte;
    currentParseState = ParseState.PAYLOAD_LENGTH;
    println("Reading ID : " + ID);
    break;
  case PAYLOAD_LENGTH:
    PLEN = rx_byte;
    //println("Payload length " + (int) PLEN);
    if ((int) PLEN > 0) {
      currentParseState = ParseState.PAYLOAD;
      rx_payload_index = 0;
    } else {
      currentParseState = ParseState.CRC1;
    }
    break;
  case PAYLOAD:
    rx_payload[rx_payload_index] = rx_byte;
    rx_payload_index++;
    if (rx_payload_index >= PLEN) {
      currentParseState = ParseState.CRC1;
    }
    break;
  case CRC1:
    println("Reading CRC1");
    // CRC1 = rx_byte;
    currentParseState = ParseState.CRC2;
    break;
  case CRC2:
    println("Reading CRC2");
    // CRC2 = rx_byte;
    processPacket();
    currentParseState = ParseState.START;
  }
}

void processPacket() { // TODO passar para funcoes individuais
  dataPacket new_packet = new dataPacket(CMD, ID, rx_payload);
  new_packet.logPacket();
  rx_packet = new_packet;
  String stringID = str(Byte.toUnsignedInt(rx_packet.id));
  if (CMD == (byte) 0x00) { // LOG COMANDO PLACEHOLDER 0x01 se tudo correr bem
    if (ID == (byte) 0x01) {
      String state = "State: " + str(Byte.toUnsignedInt(rx_packet.payload[0])) + "\n";
      String tank_temp1 = "Tank Temperature 1: " + str((Byte.toUnsignedInt(rx_packet.payload[1]) << 8) | Byte.toUnsignedInt(rx_packet.payload[2])) + "\n";
      String tank_temp2 = "Tank Temperature 2: " + str((Byte.toUnsignedInt(rx_packet.payload[3]) << 8) | Byte.toUnsignedInt(rx_packet.payload[4])) + "\n";
      String tank_press1 = "Tank Pressure 1; " + str((Byte.toUnsignedInt(rx_packet.payload[5]) << 8) | Byte.toUnsignedInt(rx_packet.payload[6])) + "\n";
      String tank_press2 = "Tank Pressure 2: " + str((Byte.toUnsignedInt(rx_packet.payload[7]) << 8) | Byte.toUnsignedInt(rx_packet.payload[8])) + "\n";

      String t1 = "Tank Pressure: " + str((Byte.toUnsignedInt(rx_packet.payload[1]) << 8) | Byte.toUnsignedInt(rx_packet.payload[2])) + "\n";
      String t2 = "Tank L: " + str((Byte.toUnsignedInt(rx_packet.payload[3]) << 8) | Byte.toUnsignedInt(rx_packet.payload[4])) + "\n";

      //log_display.setText("Status ACK rocket: " + state + tank_temp1 + tank_temp2 + tank_press1 + tank_press2);
      log_display_rocket.setText("Status ACK rocket: " + state + t1 + t2);
      println("Status ACK rocket");
    } else if (ID == (byte) 0x02) {
      // log_display.setText("Status ACK filling");
      String state = "Filling State: " + str(Byte.toUnsignedInt(rx_packet.payload[0])) + "\n";
      String t1 = "Tank Pressure: " + str((Byte.toUnsignedInt(rx_packet.payload[1]) << 8) | Byte.toUnsignedInt(rx_packet.payload[2])) + "\n";
      String t2 = "Tank L: " + str((Byte.toUnsignedInt(rx_packet.payload[3]) << 8) | Byte.toUnsignedInt(rx_packet.payload[4])) + "\n";
      log_display_filling.setText("Status ACK filling: " + state + t1 + t2);
      println("Status ACK filling");
    }
  } else if (CMD == (byte) 0x13) {
    if (targetID == 1) { // rocket
      String state = "State: " + str(Byte.toUnsignedInt(rx_packet.payload[0])) + "\n";
      String tank_temp1 = "Tank Temperature 1: " + str((Byte.toUnsignedInt(rx_packet.payload[1]) << 8) | Byte.toUnsignedInt(rx_packet.payload[2])) + "\n";
      String tank_temp2 = "Tank Temperature 2: " + str((Byte.toUnsignedInt(rx_packet.payload[3]) << 8) | Byte.toUnsignedInt(rx_packet.payload[4])) + "\n";
      String tank_press1 = "Tank Pressure 1; " + str((Byte.toUnsignedInt(rx_packet.payload[5]) << 8) | Byte.toUnsignedInt(rx_packet.payload[6])) + "\n";
      String tank_press2 = "Tank Pressure 2: " + str((Byte.toUnsignedInt(rx_packet.payload[7]) << 8) | Byte.toUnsignedInt(rx_packet.payload[8])) + "\n";

      String t1 = "Tank Pressure: " + str((Byte.toUnsignedInt(rx_packet.payload[1]) << 8) | Byte.toUnsignedInt(rx_packet.payload[2])) + "\n";
      String t2 = "Tank L: " + str((Byte.toUnsignedInt(rx_packet.payload[3]) << 8) | Byte.toUnsignedInt(rx_packet.payload[4])) + "\n";

      //log_display.setText("Status ACK rocket: " + state + tank_temp1 + tank_temp2 + tank_press1 + tank_press2);
      log_display_rocket.setText("Status ACK rocket: " + state + t1 + t2);
    } else if (targetID == 2) { // filling
      // log_display.setText("Log filling");
      String state = "Filling State: " + str(Byte.toUnsignedInt(rx_packet.payload[0])) + "\n";
      String t1 = "Tank Pressure: " + str((Byte.toUnsignedInt(rx_packet.payload[1]) << 8) | Byte.toUnsignedInt(rx_packet.payload[2])) + "\n";
      String t2 = "Tank L: " + str((Byte.toUnsignedInt(rx_packet.payload[3]) << 8) | Byte.toUnsignedInt(rx_packet.payload[4])) + "\n";
      log_display_filling.setText("Status ACK filling: " + state + t1 + t2);
    }
  }
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
    byte[] placeholder = {};
    send((byte)0x04, placeholder);
  } else if (event.isFrom("Start Filling")) {
    byte[] placeholder = {};
    send((byte)0x05, placeholder);
  } else if (event.isFrom("Resume")) {
    byte[] placeholder = {};
    send((byte)0x0a, placeholder);
  } else if (event.isFrom("Status")) {
    byte[] placeholder = {};
    send((byte)0x00, placeholder);
  } else if (event.isFrom("Select ID")) {
    targetID = (byte) (event.getValue() + 1);
  } else if (event.isFrom("Abort")) {
    byte[] placeholder = {};
    send((byte)0x02, placeholder);
  } else if (event.isFrom("Arm")) {
    byte[] placeholder = {};
    send((byte)0x08, placeholder);
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
