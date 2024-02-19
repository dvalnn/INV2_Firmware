import processing.serial.*;

Serial Receiver;

String serialPort;
String prevPort;
String serialPorts[];
int nSerialPorts;
button portButtons[];

int baudRate = 115200;

void choosePort(){
    if (state != prevState){
        log("INITIALIZING SERIAL");
        prevState = state;
        setupSerial();
    }

    serialPort = chooseSerialPort();
    if (serialPort != "" && Receiver == null){
        log("SERIAL PORT SELECTED - " + serialPort);
        Receiver = new Serial(this, serialPort, baudRate);
        prevPort = serialPort;
        log("INITIALIZED SERIAL COMMUNICATION");
        state = 0;
    }
    else if (serialPort != ""){
        log("NEW SERIAL PORT SELECTED - " + serialPort);
        Receiver.stop();
        log("SERIAL PORT CLOSED - " + prevPort);
        Receiver = new Serial(this, serialPort, baudRate);
        prevPort = serialPort;
        log("INITIALIZED SERIAL COMMUNICATION AGAIN - " + serialPort);
        state = 0;
    }
}

String chooseSerialPort(){

    if (nSerialPorts == 0){
        text("NO SERIAL PORTS CONNECTED YOU MORON", width/2, height/2);
        return "";
    }

    for (int i = 0; i < nSerialPorts; ++i){
        if (portButtons[i].pushButton()){
            return serialPorts[i];
        }
    }
    return "";
}

void setupSerial(){
    serialPorts = Serial.list();
    nSerialPorts = serialPorts.length;

    portButtons = new button[nSerialPorts];

    for (int i = 0; i < nSerialPorts; ++i){
        portButtons[i] = new button(width/3, height/3 + i*50, width/3, 40, color(0), color(255), 10, serialPorts[i]);
    }
}