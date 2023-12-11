int state; // state 0 is startScreen, state 1 is fillingScreen, state 2 is launchScreen

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
    if (key == 27) {
        if (state == 0) {
            exit();
        }
        state = 0;
        key = 0;
    }
    else if(key == 'd' || key == 'D'){
        debugging = !debugging;
    }
}

void initialize() {
    initializeStartScreen();
    initializeFillingScreen();
    initializeLaunchScreen();

    //initializeSerial();
}