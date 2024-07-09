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
int TIMEOUT = 800;
byte MyID = (byte) 0x00;
byte targetID;
LinkedBlockingQueue<byte[]> tx_queue = new LinkedBlockingQueue<byte[]>(); 
            
// packet structure : "SYNC", "CMD", "ID", "PLEN", "PAYLOAD", "CRC1", "CRC2"
byte CMD=0, PLEN=0, ID=0; /* CRC1, CRC2 */
byte[] rx_payload = new byte[100];
ParseState currentParseState = ParseState.START;
int rx_payload_index = 0;

HashMap<String, boolean[]> prog_args = new HashMap<String, boolean[]>();
Textfield[] textfields = new Textfield[5];
Textlabel log_display;
int[] prog_inputs = new int[5];
int selected_index = -1;
byte[] prog_cmds = {(byte)0x01, (byte)0x02, (byte)0x03, (byte)0x01, (byte)0x02};

List<String> programs = Arrays.asList("fp1", "fp2", "fp3", "rp1", "rp2");
List<String> vars = Arrays.asList("tp1", "tp2", "tp3", "tl1", "tl2");
List<String> IDs = Arrays.asList( "1 : Filling Station", "2 : Rocket");
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

  cp5.addButton("Status")
    .setPosition(1500, 340)
    .setSize(150, 50)
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
    .setFont(font);
  // List available serial ports and add them to a new ScrollableList
  List<String> portNames = Arrays.asList(Serial.list());
  cp5.addScrollableList("serialPort")
    .setPosition(100, 600) 
    .setSize(250, 500)
    .setBarHeight(50)
    .setItemHeight(50)
    .addItems(portNames)
    .setFont(font)
    .setColorBackground(color(50, 50, 50))
    .setColorForeground(color(0, 144, 0))
    .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER);
  
  cp5.addScrollableList("Select ID")
     .setPosition(1500, 400)
     .setSize(320, 500)
     .setBarHeight(50)
     .setItemHeight(50)
     .addItems(IDs)
     .setFont(font)
     .setColorBackground(color(50, 50, 50))
     .setColorForeground(color(0, 144, 0))
     .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER);
  
  log_display = cp5.addTextlabel("Log")
      .setText("Logging Packet goes here")
      .setPosition(width/2-200, height/2-200)
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
      while(myPort.available() > 0) {
        byte rx_byte = (byte) myPort.read();
        // log_display.setText(str(char(rx_byte)));
        parseIncomingByte(rx_byte);
      }
      
      // Send packets in queue
      byte[] head = tx_queue.peek();
      while(head != null) {
        myPort.write(head);
        tx_queue.remove();
        head = tx_queue.peek();
      }
    }
    // log_display.setText(str(selected_index));
}

void parseIncomingByte(byte rx_byte) {
  
  if(last_read_time == 0 || millis() - last_read_time > TIMEOUT) {
    currentParseState = ParseState.START;
  }
  last_read_time = millis();
  switch(currentParseState) {
  case START:
    if (rx_byte == (byte) 0x55) {
      currentParseState = ParseState.CMD;
    } else {
      println("Start byte not received");
    }
    break;
  case CMD:
    CMD = rx_byte;
    if (CMD == (byte) 0x00) {
      println("Status cmd received " + (int) CMD);
      currentParseState = ParseState.ID;
    }
    else {
      println("Command Received: " + (int) CMD);
    }
    break;
  case ID:
    println("Reading ID");
    ID = rx_byte;
    if(ID != MyID) {
      currentParseState = ParseState.START;
      println("Wrong ID");
    }
    else {
      currentParseState = ParseState.PAYLOAD_LENGTH;
    }
    break;
  case PAYLOAD_LENGTH:
    PLEN = rx_byte;
    println("Payload length " + (int) PLEN);
    if ((int) PLEN > 0) {
      currentParseState = ParseState.PAYLOAD;
      rx_payload_index = 0;
    } else {
      currentParseState = ParseState.CRC1;
    }
    break;
  case PAYLOAD:
    println("Reading Payload");
    if(rx_payload_index < (int) PLEN) {
      rx_payload[rx_payload_index] = rx_byte;
      rx_payload_index++;
      currentParseState = ParseState.PAYLOAD;
    }
    else {
      currentParseState = ParseState.CRC1;
      rx_payload_index = 0;
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

void processPacket() {
  dataPacket new_packet = new dataPacket(CMD, targetID, rx_payload);
  new_packet.logPacket();
  rx_packet = new_packet;
  String state = str(Byte.toUnsignedInt(rx_packet.payload[0]));
  String imu_ax = str((Byte.toUnsignedInt(rx_packet.payload[1]) << 8) | Byte.toUnsignedInt(rx_packet.payload[2]));
  String imu_ay = str((Byte.toUnsignedInt(rx_packet.payload[3]) << 8) | Byte.toUnsignedInt(rx_packet.payload[4]));
  String imu_az = str((Byte.toUnsignedInt(rx_packet.payload[5]) << 8) | Byte.toUnsignedInt(rx_packet.payload[6]));
  String imu_gx = str((Byte.toUnsignedInt(rx_packet.payload[7]) << 8) | Byte.toUnsignedInt(rx_packet.payload[8]));
  String imu_gy = str((Byte.toUnsignedInt(rx_packet.payload[9]) << 8) | Byte.toUnsignedInt(rx_packet.payload[10]));
  String imu_gz = str((Byte.toUnsignedInt(rx_packet.payload[11]) << 8) | Byte.toUnsignedInt(rx_packet.payload[12]));
  
  log_display.setText(state + " " + imu_ax + "," + imu_ay+ "," + imu_az + " " + imu_gx + "," + imu_gy+ "," + imu_gz);
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
  } else if (event.isFrom("Stop")) {
    byte[] placeholder = {};
    send((byte)0x03, placeholder);
  } else if (event.isFrom("Start Filling")) {
    byte[] placeholder = {};
    send((byte)0x04, placeholder);
  } else if (event.isFrom("Resume")) {
    byte[] placeholder = {};
    send((byte)0x0a, placeholder);
  } else if (event.isFrom("Status")) {
    byte[] placeholder = {};
    send((byte)0x00, placeholder);
  } else if (event.isFrom("Select ID")) {
    targetID = (byte) (event.getValue() + 1);
  }
}

void send(byte command, byte[] payload) {
  println(command, payload);
  tx_packet = new dataPacket(command, targetID, payload);
  tx_packet.logPacket();
  if (myPort != null) {
    byte[] packet = tx_packet.getPacket();
    println(packet);
    tx_queue.add(packet);
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
