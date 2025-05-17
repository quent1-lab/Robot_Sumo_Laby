#ifndef ZELDA_MELODY_H
#define ZELDA_MELODY_H

#include "pitches.h"

int melodie_Zelda[] = {
  NOTE_B5, NOTE_D6, NOTE_F6, NOTE_B6,
  NOTE_A6, NOTE_F6, NOTE_G6, NOTE_E6,
  NOTE_C6, NOTE_E6, NOTE_A6, NOTE_B6,
  REST, NOTE_A6, NOTE_B6, NOTE_E6,
  NOTE_F6, NOTE_D6, NOTE_B5
};

int duree_Zelda[] = {
  8, 8, 8, 8,
  8, 8, 8, 8,
  8, 8, 8, 8,
  4, 8, 8, 8,
  8, 8, 4
};

int taille_Zelda = sizeof(melodie_Zelda) / sizeof(int);


#endif // ZELDA_MELODY_H