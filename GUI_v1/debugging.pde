boolean debugging = false;

void debugging() {
    //VALVES STATES CHECKING
    /* for (int i = 0; i < 3; ++i) {
        print("Valve ");
        print(i);
        print(": ");
        if (valvesStates[i]) {
            println("OPEN");
        }
        else {
            println("CLOSED");
        }
    } */
    //VALVES STATES CHECKING

    /* println("X: " + mouseX, "Y: " + mouseY);

    print("\n"); */
    textSize(32);
    textAlign(TOP, LEFT);
    fill(0, 255, 0);
    text("X: " + mouseX + "\nY: " + mouseY, width-150, 50);
    text(frameRate, width-155, 130);
}