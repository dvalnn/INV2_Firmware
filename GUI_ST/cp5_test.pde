import controlP5.*;
import processing.serial.*;
import java.util.*;

ControlP5 cp5;
PFont font;
Serial myPort; // For serial communication
int baudRate = 115200;
dataPacket tx_packet;

void setup() {
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
  
  // Handle other events, like selections from your "program" ScrollableList
}

// Implement actions for the "Send" button and other interactions as needed
void send() {
  // Example function to be called when the "Send" button is pressed
  println("Send button clicked.");
  // Add code here for what should happen when "Send" is clicked.
  byte[] placeholder_payload = {(byte) 0x05, (byte) 0x06, (byte) 0x04};
  tx_packet = new dataPacket((byte) 0x01, placeholder_payload); 
  myPort.write(tx_packet.getPacket());
}

// This method ensures the serial port is closed properly when the program is exited
void stop() {
  if (myPort != null) {
    myPort.stop();
  }
  super.stop();
}
