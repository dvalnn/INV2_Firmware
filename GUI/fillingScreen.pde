button valves[];
PImage img;
boolean valvesStates[];

int NVALVES = 3;
// int NVALVES = 4;

void initializeFillingScreen() {
    valves = new button[NVALVES];
    
    valves[0] = new button(width / 2 + 100, height - 200, 100, 75, color(0), color(255), 10, "Valve 1");
    valves[1] = new button(width / 2 + 250, height - 200, 100, 75, color(0), color(255), 10, "Valve 2");
    valves[2] = new button(width / 2 + 400, height - 200, 100, 75, color(0), color(255), 10, "Valve 3");
    valves[3] = new button(width / 2 + 550, height - 200, 100, 75, color(0), color(255), 10, "VPU");
    
    // img = loadImage("FillProcess.png");
    img = loadImage("FuellingSystem.png");
    img.resize(1000, 0);
    
    valvesStates = new boolean[NVALVES];
    
    log("INITIALIZED FILLING SCREEN");
}

void fillingScreen() {
    if (state != prevState) {
        log("FILLING SCREEN");
        prevState = state;
    }
    
    image(img, 0, 0);
    byte[] valvesHex = {(byte)0x01, (byte)0x02, (byte)0x03, (byte)0x04};
    for (int i = 0; i < valves.length; ++i) {
        boolean anterior = valvesStates[i];
        valvesStates[i] = valves[i].toggle();
        if (anterior != valvesStates[i]) {
            log("CHANGED VALVE STATE - " + (i + 1) + "->" + valvesStates[i]);
            // serial write
            byte[] valvePayload = {valvesHex[i], valvesStates[i] ? (byte) 0x01 : (byte) 0x00};
            valves[i].setPacket((byte) 0x25, valvePayload);
            valves[i].packet.logPacket();
            Receiver.write(valves[i].packet.getPacket());
        }
    }
    colorCodingValves();
}

void colorCodingValves() {
    PVector valvesCoordinates[] = new PVector[NVALVES];
    valvesCoordinates[0] = new PVector(375, 242);
    valvesCoordinates[1] = new PVector(397, 711);
    valvesCoordinates[2] = new PVector(858, 94);
    
    for (int i = 0; i < NVALVES; ++i) {
        fill(255, 0, 0);
        if (valvesStates[i]) {
            fill(0, 255, 0);
        }
        ellipse(valvesCoordinates[i].x, valvesCoordinates[i].y, 20, 20);
    }
}
