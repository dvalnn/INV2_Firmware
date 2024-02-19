import processing.serial.*;

Serial Receiver;

String serialPort;
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
    if (serialPort != ""){
        log("SERIAL PORT SELECTED - " + serialPort);
        Receiver = new Serial(this, serialPort, baudRate);
        log("INITIALIZED SERIAL COMMUNICATION");
        state = 0;
    }
}

String chooseSerialPort(){

    if (nSerialPorts == 0){
        text("NO SERIAL PORTS CONNECTED YOU MORON", width/2, height/2);
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