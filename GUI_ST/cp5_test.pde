import controlP5.*;
import processing.serial.*;
import java.util.*;

ControlP5 cp5;
PFont font;
Serial myPort; // For serial communication
dataPacket tx_packet;

byte tp11, tp12, tp21, tp22, tp31, tp32, tl11, tl12, tl21, tl22; // program inputs (2 bytes per arg)
byte[] prog_cmds = {(byte)0x00, (byte)0x01, (byte)0x02, (byte)0x03, (byte)0x04};

void setup() {
  //inits
  initializeWriters();
  frameRate(120);
  fullScreen();
  background(0);
  font = createFont("arial", 30);
  cp5 = new ControlP5(this);

  // List for programs
  List<String> programs = Arrays.asList("fp1", "fp2", "fp3", "rp1", "rp2");
  cp5.addScrollableList("program")
     .setPosition(100, 100)
     .setSize(300, 500)
     .setBarHeight(50)
     .setItemHeight(50)
     .addItems(programs)
     .setFont(font)
     .setColorBackground(color(50, 50, 50))
     .setColorForeground(color(144, 0, 0))
     .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER);

  // Textfields for parameters and limits
  for (int i = 1; i <= 3; i++) {
    cp5.addTextfield("tp" + i)
       .setPosition(300 + i * 170, 100)
       .setSize(150, 50)
       .setFont(font)
       .setInputFilter(ControlP5.FLOAT);
  }
  
  for (int i = 1; i <= 2; i++) {
    cp5.addTextfield("tl" + i)
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
  }
  
  if(event.isFrom("Send")) {
    byte[] payload = {prog_cmds[3], tp11, tp12, tp21, tp22, tp31, tp32, tl11, tl12, tl21, tl22}; // program to exec and the args
    send((byte) 0x53, payload); // placeholder command value -> replace with EXEC_PROG command value
  }
}

void send(byte command, byte[] payload) {
  println("Send button clicked.");
  tx_packet = new dataPacket(command, payload); 
  tx_packet.logPacket();
  if (myPort != null) {
    myPort.write(tx_packet.getPacket());
  }
  else {
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
