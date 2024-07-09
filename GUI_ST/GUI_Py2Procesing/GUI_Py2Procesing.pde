import processing.serial.*;
import controlP5.*;

Serial ser;
ControlP5 cp5;

String portName = "COM7"; // Change this to the appropriate port name
int baudRate = 115200;
float serTimeout = 0.001;
int logDelay = 50;
float messageTimeout = 250;
int missedPackets = 0;
int logSpeed = 0;
int logFileSize = 4096;

int cmdId = 1;

HashMap<String, Integer> commandMap = new HashMap<String, Integer>() {{
  put("STATUS", 0);
  put("ABORT", 1);
  put("EXEC_PROG", 2); 
  put("STOP_PROG", 3);
  put("FUELING", 4);
  put("READY", 5);
  put("ARM", 6);
  put("LED_ON", 7);
  put("LED_OFF", 8);
  put("IMU_CALIB", 9);
  put("RESUME_PROG", 10);
  put("FIRE_PYRO", 11);
  put("FLASH_LOG_START", 12);
  put("FLASH_LOG_STOP", 13);
  put("FLASH_IDS", 14);
  put("FLASH_DUMP", 15);
  put("cmd_size", 16);
  put("STATUS_ACK", 17);
  put("ABORT_ACK", 18); 
  put("EXEC_PROG_ACK", 19);
  put("STOP_PROG_ACK", 20);
  put("FUELING_ACK", 21);
  put("READY_ACK", 22);
  put("ARM_ACK", 23);
  put("LED_ON_ACK", 24);
  put("LED_OFF_ACK", 25);
  put("IMU_CALIB_ACK", 26);
  put("RESUME_PROG_ACK", 27);
  put("FIRE_PYRO_ACK", 28); 
  put("FLASH_LOG_START_ACK", 29);
  put("FLASH_LOG_STOP_ACK", 30);
  put("FLASH_IDS_ACK", 31);
  put("FLASH_DUMP_ACK", 32);
}};

HashMap<Integer, String> stateMapToStringRocket = new HashMap<Integer, String>() {{
  put(0, "IDLE");
  put(1, "FUELING");
  put(2, "PROG1");
  put(3, "PROG2");
  put(4, "SAFETY");
  put(5, "READY");
  put(6, "ARMED");
  put(7, "LAUNCH");
  put(8, "ABORT");
  put(9, "IMU_CALIB");
}};

public static final int syncState = 1;
public static final int cmdState = 2;
public static final int idState = 3;
public static final int sizeState = 4;
public static final int dataState = 5;
public static final int crc1State = 6;
public static final int crc2State = 7;
public static final int endState = 8;

int commState = syncState;
int dataTotal = 0;
int dataRecv = 0;
long begin = 0;
long end = 0;
byte[] buff = new byte[1024];

boolean logStatus = false;
long logBegin = 0;
long logEnd = 0;

void setup() {
  size(800, 600);
  surface.setResizable(true);
  ser = new Serial(this, portName, baudRate);
  ser.clear();
  
  cp5 = new ControlP5(this);

  cp5.addButton("_LED_ON_").setLabel("LED ON").setPosition(20, 20).setSize(100, 50);
  cp5.addButton("_LED_OFF_").setLabel("LED OFF").setPosition(140, 20).setSize(100, 50);
  cp5.addButton("_FUELING_").setLabel("Start Fueling").setPosition(20, 90).setSize(100, 50);
  cp5.addButton("_STOP_").setLabel("Stop").setPosition(140, 90).setSize(100, 50);
  cp5.addButton("_EXEC_").setLabel("Exec Prog").setPosition(260, 90).setSize(100, 50);
  cp5.addTextfield("_PROG_").setLabel("Prog").setPosition(380, 90).setSize(200, 50);
  cp5.addButton("_READY_").setLabel("Ready").setPosition(20, 160).setSize(100, 50);
  cp5.addButton("_ARM_").setLabel("Arm").setPosition(140, 160).setSize(100, 50);
  cp5.addButton("_ABORT_").setLabel("Abort").setPosition(260, 160).setSize(100, 50);
  cp5.addButton("_STATUS_").setLabel("Status").setPosition(380, 160).setSize(100, 50);
  cp5.addButton("_IMU_CALIB_").setLabel("IMU Calib").setPosition(500, 160).setSize(100, 50);
  cp5.addButton("_LOG_TOGGLE_").setLabel("Toggle Log").setPosition(620, 160).setSize(100, 50);
  cp5.addButton("Exit").setLabel("Exit").setPosition(740, 160).setSize(100, 50);
}

void draw() {
  background(200);

  if (logStatus) {
    long currentTime = millis();
    if (currentTime - logBegin > logDelay) {
      logBegin = currentTime;
      // Send status command
      sendCommand("STATUS");
    }
  }
}

void controlEvent(ControlEvent theEvent) {
  if (theEvent.isController()) {
    String name = theEvent.getName();
    println(name);

    if (name.equals("_LED_ON_")) {
      sendCommand("LED_ON");
    } else if (name.equals("_LED_OFF_")) {
      sendCommand("LED_OFF");
    } else if (name.equals("_FUELING_")) {
      sendCommand("FUELING");
    } else if (name.equals("_STOP_")) {
      sendCommand("STOP_PROG");
    } else if (name.equals("_EXEC_")) {
      sendCommand("EXEC_PROG");
    } else if (name.equals("_READY_")) {
      sendCommand("READY");
    } else if (name.equals("_ARM_")) {
      arm();
    } else if (name.equals("_ABORT_")) {
      sendCommand("ABORT");
    } else if (name.equals("_STATUS_")) {
      sendCommand("STATUS");
    } else if (name.equals("_IMU_CALIB_")) {
      sendCommand("IMU_CALIB");
    } else if (name.equals("_LOG_TOGGLE_")) {
      toggleStatus();
    } else if (name.equals("Exit")) {
      exit();
    }
  }
}

void sendCommand(String command) {
  if (commandMap.containsKey(command)) {
    int cmd = commandMap.get(command);
    byte[] cmdBytes = { (byte) 0x55, (byte) cmd, (byte) cmdId, 0, 0x20, 0x21 };
    ser.write(cmdBytes);
    readCmd();
  }
}

void arm() {
  int[] ids = { 1, 2, 3 };
  for (int id : ids) {
    byte[] cmd = { (byte) 0x55, (byte) (int) commandMap.get("ARM"), (byte) cmdId, (byte) 1, (byte) id, (byte) 2, (byte) 3 };
    ser.write(cmd);
    readCmd();
  }
}

void readCmd() {
  commState = syncState;
  dataRecv = 0;
  begin = millis();
  
  while (true) {
    while (ser.available() > 0) {
      int ch = ser.read();
      processByte(ch);
    }
    
    end = millis();
    float msec = end - begin;
    
    if (commState == endState) {
      commState = syncState;
      println("ACK received:", buff.length);
      println("Time", msec);
      if (buff[1] == commandMap.get("STATUS_ACK")) {
        printStatus();
      } else {
        println("Not status ACK");
      }
      StringBuilder sb = new StringBuilder("Cmd_ACK: [");
      for (byte b : buff) {
        sb.append(hex(b)).append(" ");
      }
      sb.append("]");
      println(sb.toString());
      delay(5);
      return;
    } else if (commState != syncState && msec > messageTimeout) {
      missedPackets++;
      println("Command timeout", msec, buff.length, dataRecv, dataTotal);
      println("Cmd state:", commState);
      StringBuilder sb = new StringBuilder("Cmd fail: [");
      for (byte b : buff) {
        sb.append(hex(b)).append(" ");
      }
      sb.append("]");
      println(sb.toString());
      commState = syncState;
      delay(15);
      return;
    } else if (msec > messageTimeout) {
      println("Command timeout", msec);
      println("No cmd state:", commState);
      StringBuilder sb = new StringBuilder("Cmd fail: [");
      for (byte b : buff) {
        sb.append(hex(b)).append(" ");
      }
      sb.append("]");
      println(sb.toString());
      missedPackets++;
      delay(15);
      return;
    }
  }
}

void processByte(int ch) {
  switch (commState) {
    case syncState:
      if (ch == 0x55) {
        commState = cmdState;
        buff[dataRecv++] = (byte) ch;
      }
      break;
    case cmdState:
      buff[dataRecv++] = (byte) ch;
      commState = idState;
      break;
    case idState:
      buff[dataRecv++] = (byte) ch;
      commState = sizeState;
      break;
    case sizeState:
      buff[dataRecv++] = (byte) ch;
      dataTotal = ch + 5;
      commState = dataState;
      break;
    case dataState:
      buff[dataRecv++] = (byte) ch;
      if (dataRecv >= dataTotal) {
        commState = crc1State;
      }
      break;
    case crc1State:
      buff[dataRecv++] = (byte) ch;
      commState = crc2State;
      break;
    case crc2State:
      buff[dataRecv++] = (byte) ch;
      commState = endState;
      break;
    case endState:
      break;
  }
}

void printStatus() {
  int packetSize = buff.length;
  int state = buff[5];
  int altitude = (buff[6] << 8) | buff[7];
  int accelX = (buff[8] << 8) | buff[9];
  int accelY = (buff[10] << 8) | buff[11];
  int accelZ = (buff[12] << 8) | buff[13];
  int gyroX = (buff[14] << 8) | buff[15];
  int gyroY = (buff[16] << 8) | buff[17];
  int gyroZ = (buff[18] << 8) | buff[19];
  int burnState = buff[20];
  String flightState = stateMapToStringRocket.get(state);
  
  println("Status packet received");
  println("Packet size:", packetSize);
  println("Flight state:", flightState);
  println("Altitude:", altitude, "cm");
  println("Acceleration - X:", accelX, "m/s^2, Y:", accelY, "m/s^2, Z:", accelZ, "m/s^2");
  println("Gyro - X:", gyroX, "deg/s, Y:", gyroY, "deg/s, Z:", gyroZ, "deg/s");
  println("Burn state:", burnState);
}

void toggleStatus() {
  logStatus = !logStatus;
}
