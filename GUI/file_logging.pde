import java.io.FileOutputStream;
import java.io.File;
import java.io.OutputStream;

boolean logRunning = false;
OutputStream logStream;
String logDir;
File file;

void init_log() {
  try {
    logStream = new FileOutputStream(file);
    println("file created: " + file.getAbsolutePath());
    logRunning = true;
  }
  catch (IOException e) {
    println("Error creating file: " + e);
  }
  println("log init called");
}

enum LogEvent {
  SENSOR_READING,
    MSG_RECEIVED,
    MSG_SENT,
    SYSTEM_ERROR,
    STATE_CHANGE,
    EVENT_REACTION,
};

void flash_log(dataPacket packet, LogEvent event) {
  if (!logRunning) {
    println("Logging not started, skipping log entry.");
    return;
  }
  try {
    byte[] buff = new byte[256];
    short index = 0;
    int time = millis();

    buff[index++] = (byte) event.ordinal();
    buff[index++] = (byte)((time >> 24) & 0xff);
    buff[index++] = (byte)((time >> 16) & 0xff);
    buff[index++] = (byte)((time >> 8)  & 0xff);
    buff[index++] = (byte)((time)       & 0xff);

    buff[index++] = packet.command;
    buff[index++] = packet.id;
    buff[index++] = packet.payloadLength;

    for (int i = 0; i < packet.payloadLength; i++) {
      buff[index++] = packet.payload[i];
    }

    logStream.write(buff, 0, index);
    logStream.flush();  // Ensure data is written immediately
    //println("Log entry added.");
  }
  catch (IOException e) {
    println("Error writing to log file: " + e);
  }
  catch (Exception e) {
    println("Unexpected error: " + e);
  }
}
