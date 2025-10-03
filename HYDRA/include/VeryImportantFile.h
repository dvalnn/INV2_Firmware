#ifndef VERY_IMPORTANT_FILE_H
#define VERY_IMPORTANT_FILE_H

#include <Arduino.h>

#define ARRAY_LEN(array) (sizeof(array) / sizeof(array[0]))
#define E5 659
#define A5 880
#define Cb6 1109
#define B5 988
#define Gb5 831
#define Fb5 740
#define Db5 622
#define D6 1175
#define E6 1319

const int midi1[81][3] = {
 {E5, 136, 136},
 {A5, 136, 136},
 {A5, 136, 136},
 {Cb6, 136, 136},
 {Cb6, 136, 136},
 {A5, 136, 409},
 {E5, 136, 136},
 {E5, 136, 136},
 {E5, 136, 273},
 {E5, 136, 0},
 {B5, 136, 0},
 {A5, 136, 0},
 {Gb5, 136, 0},
 {Fb5, 136, 0},
 {E5, 136, 682},
 {E5, 136, 136},
 {A5, 136, 136},
 {A5, 136, 136},
 {Cb6, 136, 136},
 {Cb6, 136, 136},
 {A5, 136, 409},
 {E5, 136, 136},
 {A5, 136, 136},
 {Gb5, 136, 136},
 {Fb5, 136, 0},
 {Gb5, 136, 0},
 {A5, 136, 136},
 {Db5, 136, 136},
 {E5, 136, 682},
 {E5, 136, 136},
 {Gb5, 136, 136},
 {Gb5, 136, 136},
 {A5, 136, 0},
 {Gb5, 136, 0},
 {Fb5, 136, 0},
 {Gb5, 136, 0},
 {A5, 136, 409},
 {E5, 136, 136},
 {A5, 136, 136},
 {Gb5, 136, 136},
 {Gb5, 136, 136},
 {Gb5, 136, 0},
 {D6, 136, 0},
 {B5, 136, 0},
 {Gb5, 136, 0},
 {A5, 136, 682},
 {A5, 136, 136},
 {Fb5, 136, 136},
 {Fb5, 136, 136},
 {Fb5, 136, 136},
 {A5, 136, 136},
 {A5, 136, 409},
 {E5, 136, 136},
 {E5, 136, 136},
 {E5, 136, 273},
 {E5, 136, 0},
 {B5, 136, 136},
 {Gb5, 136, 136},
 {A5, 136, 682},
 {A5, 136, 136},
 {Gb5, 136, 0},
 {Fb5, 136, 0},
 {Fb5, 136, 136},
 {Fb5, 136, 0},
 {A5, 136, 0},
 {Gb5, 136, 0},
 {B5, 136, 0},
 {A5, 136, 409},
 {E5, 136, 136},
 {E5, 136, 136},
 {E5, 136, 273},
 {E5, 136, 0},
 {B5, 136, 136},
 {Gb5, 136, 136},
 {A5, 136, 682},
 {Cb6, 136, 136},
 {E6, 136, 0},
 {B5, 136, 0},
 {Cb6, 136, 0},
 {A5, 136, 0},
 {Fb5, 136, 0},
};

void playMidi(int pin, const int notes[][3], size_t len){
 for (int i = 0; i < len; i++) {
    tone(pin, notes[i][0]);
    delay(notes[i][1]);
    noTone(pin);
    delay(notes[i][2]);
  }
}
// Generated using https://github.com/ShivamJoker/MIDI-to-Arduino

void success_song(int pin) {
  playMidi(pin, midi1, ARRAY_LEN(midi1));
}


#endif // VERY_IMPORTANT_FILE_H