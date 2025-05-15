// Melodie.h
#ifndef MELODIE_H
#define MELODIE_H

#include <Arduino.h>

class Melodie {
public:
  Melodie(int pinBuzzer);

  // Choix d'une mélodie
  void choisirMelodie(int melodie);
  
  // Arrête toute émission
  void stop();

  // Démarre une série de "bips"
  // count: nombre de bips
  // freq: fréquence du bip en Hz
  // dur: durée du bip en ms
  // gap: intervalle entre deux bips en ms
  void bib(int count, int freq, uint32_t dur, uint32_t gap);

  // À appeler fréquemment dans loop(), gère la progression
  void update();

private:
  int _pin;

  // Mélodie
  int* _melody;
  int* _durations;
  int  _len;
  int  _idx;
  bool _playingMelody;
  uint32_t _nextChange;
  void playMelody(int melodie[], int duree[], int taille);

  // Bips
  int  _beepCount;
  int  _beepFreq;
  uint32_t _beepDur;
  uint32_t _beepGap;
  int  _beepIdx;
  bool _playingBeep;
  uint32_t _beepNext;
};

#endif // MELODIE_H