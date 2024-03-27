button valves[];
button fillTypeBtn;
button confirmBtn;
String prompts[];
int promptIndex;
boolean[] initConditions = {false, false, false};
PImage img;
boolean valvesStates[];
byte[] valvesHex = {(byte)0x01, (byte)0x02, (byte)0x03, (byte)0x04};
int NVALVES = 4;
boolean manualFilling = false;
int fillStage = 0;

void initializeFillingScreen() {
    valves = new button[NVALVES];
    
    valves[0] = new button(width / 2 + 100, height - 200, 100, 75, color(0), color(255), 10, "Valve 1");
    valves[1] = new button(width / 2 + 250, height - 200, 100, 75, color(0), color(255), 10, "Valve 2");
    valves[2] = new button(width / 2 + 400, height - 200, 100, 75, color(0), color(255), 10, "Valve 3");
    valves[3] = new button(width / 2 + 550, height - 200, 100, 75, color(0), color(255), 10, "VPU");
    
    fillTypeBtn = new button(width - 200, 100, 100, 75, color(0), color(255), 10, "TOGGLE FILL"); 
    confirmBtn = new button(width / 2 + 400, height / 2, 100, 75, color(0), color(255), 10, "YES");
    
    prompts = new String[3];
    prompts[0] = "Are all valves closed? (v1, v2, v3, v4 and vpu)";
    prompts[1] = "Are both bottles open? (He & N20)";
    prompts[2] = "Ready to start filling ?";
    promptIndex = 0;  
      
    img = loadImage("FillProcess.png");
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
    manualFilling = fillTypeBtn.toggle(); // On : Manual Filling // Off : Auto Filling // Default : Off 
    if(manualFilling) {
      manualFilling();
    }
    else {
      autoFill();
    }
    colorCodingValves();
}

void autoFill() {
  switch(fillStage) {
    case 0:
      // ensure pre init and init conditions are met
      if(promptIndex == 3) {
        fillStage = 1;
      }
      else {
        textSize(32);
        text(prompts[promptIndex], width/2 + 400, height/2 - 50);
        if(initConditions[promptIndex] == false) {
          if(confirmBtn.pushButton()) {
            initConditions[promptIndex] = true;
            promptIndex++;
            delay(100);
          }
        }
      }
      break;
    case 1:
      // purge the system with He
      // open V1
      // check if tank pressure is above 5.5 bar
      // close V1 and open VPU
      // check if tank pressure is below or equal to 5 bar
      // close VPU
      break;
    case 2:
      // Fill the tank with X litters of N20
      // Check if V1 and VPU are closed and tank pressure is stabilized
      // Close VPU and open V2
      // Check if tank pressure is above 40 bar and volume of N20 is below X
      // Close V2 and open VPU
      // Check if tank pressure is below 35 bar
      // Close VPU and open V2
      // Check if volume of N20 is above or equal to X
      // Close V2
      // Open V3
      // Check if tank pressure is less or equal to 1 bar
      // Close V3
      break;
    case 3:
      // Get tank to Y bar of pressure
      // Check if V2 and VPU are closed and tank pressure is stabilized
      // Open V1
      // Check if tank pressure is above or equal to 50 bar
      // Close V1
      // Open V3
      break;
    case 4:
      // Display that filling is complete
      break;
  }
}

void manualFilling() {
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
}

void colorCodingValves() {
    PVector valvesCoordinates[] = new PVector[NVALVES];
    valvesCoordinates[0] = new PVector(252, 139);
    valvesCoordinates[1] = new PVector(252, 317);
    valvesCoordinates[2] = new PVector(514, 80);
    valvesCoordinates[3] = new PVector(691, 80);
    
    for (int i = 0; i < NVALVES; ++i) {
        fill(255, 0, 0);
        if (valvesStates[i]) {
            fill(0, 255, 0);
        }
        ellipse(valvesCoordinates[i].x, valvesCoordinates[i].y, 20, 20);
    }
}
