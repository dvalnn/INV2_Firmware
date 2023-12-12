import processing.serial.*;

Serial Receiver;

String serialPort = "dev/cu.usbmodem1101";
int baudRate = 115200;

void initializeSerial(){
    Receiver = new Serial(this, serialPort, baudRate);

    log("INITIALIZED SERIAL COMMUNICATION");
}