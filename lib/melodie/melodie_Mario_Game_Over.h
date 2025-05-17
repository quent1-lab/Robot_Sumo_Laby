#ifndef MELODIE_MARIO_GAMEOVER_H
#define MELODIE_MARIO_GAMEOVER_H

#include "pitches.h"

int melodie_Mario_GameOver[] = {
  NOTE_C5, NOTE_G4, NOTE_E4, NOTE_A4,
  NOTE_B4, NOTE_A4, NOTE_AS4, NOTE_A4,
  NOTE_G4, NOTE_E5, NOTE_G5, NOTE_A5,
  NOTE_F5, NOTE_G5, REST, NOTE_C5
};


int duree_Mario_GameOver[] = {
  4, 4, 4, 4,
  4, 8, 8, 4,
  4, 4, 4, 4,
  4, 4, 4, 4
};


int taille_Mario_GameOver = sizeof(melodie_Mario_GameOver) / sizeof(int);


#endif
