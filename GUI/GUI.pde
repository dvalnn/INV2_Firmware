int state; // state 0 is startScreen, state 1 is fillingScreen, state 2 is launchScreen
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

    //initializeSerial();

    log("INITIALIZATION SUCCESSFUL");
}