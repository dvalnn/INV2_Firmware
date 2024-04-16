void initializeLaunchScreen() {
    
    log("INITIALIZED LAUNCH SCREEN");
}

void launchScreen() {
    if (state != prevState) {
        log("LAUNCH SCREEN");
        prevState = state;
    }
}
