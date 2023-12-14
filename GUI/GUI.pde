int state; // 0-> startScreen | 1-> fillingScreen | 2-> launchScreen | 3-> chooseSerialPort
int prevState;

void setup() {
    frameRate(120);
    fullScreen();
    
    initialize();
    
    state = 0;
} 

void draw() {
    background(0);
    
    switch(state) {
        case 0:
            startScreen();
            break;
        case 1:
            fillingScreen();
            break;
        case 2:
            launchScreen();
            break;
        case 3:
            choosePort();
    }

    if (debugging){
        debugging();
    }
}

void keyPressed() {
    log("PRESSED KEY - " + key);
    if (key == 27) {
        if (state == 0) {
            log("EXITING");
            flushWriters();
            exit();
        }
        state = 0;
        key = 0;
        log("CHANGED STATE - " + state);
    }
    else if(key == 'd' || key == 'D'){
        log("TOGGLED DEBUGGING - " + !debugging);
        debugging = !debugging;
    }
}

void initialize() {
    initializeWriters();

    initializeStartScreen();
    initializeFillingScreen();
    initializeLaunchScreen();

    log("INITIALIZATION SUCCESSFUL");
}