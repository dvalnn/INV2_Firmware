#include "Buzzer.h"
#include "VeryImportantFile.h"  // For success_song function

void setup_buzzer() {
    pinMode(BUZZER_PWM, OUTPUT);
}

void play_buzzer(int frequency, int duration) {
    tone(BUZZER_PWM, frequency, duration);
}

void stop_buzzer() {
    noTone(BUZZER_PWM);
}

void play_buzzer_error() {
    for(int i = 0; i < 4; i++) {
        play_buzzer(200, 500); 
        delay(1000);
    }
}

void play_buzzer_success() {
    /*
    for(int i = 0; i < 3; i++) {
        play_buzzer(2000, 100); 
        delay(200);
    }
    */
    success_song(BUZZER_PWM);
}