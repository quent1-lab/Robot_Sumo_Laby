// melodie.cpp
#include "melodie.h"
#include "melodie_Mario_Bros.h"
#include "melodie_StarWars.h"
#include "melodie_Zelda.h"
#include "melodie_Minecraft.h"

Melodie::Melodie(int pinBuzzer)
    : _pin(pinBuzzer),
      _melody(nullptr), _durations(nullptr), _len(0), _idx(0), _playingMelody(false), _nextChange(0),
      _beepCount(0), _beepFreq(0), _beepDur(0), _beepGap(0), _beepIdx(0), _playingBeep(false), _beepNext(0)
{
    pinMode(_pin, OUTPUT);
}

void Melodie::choisirMelodie(int melodie)
{
    switch (melodie)
    {
    case 1:
        playMelody(melodie_Mario_Bros, duree_Mario_Bros, taille_Mario_Bros);
        break;
    case 2:
        playMelody(melodie_StarWars, duree_StarWars, taille_StarWars);
        break;
    case 3:
        playMelody(melodie_Zelda, duree_Zelda, taille_Zelda);
        break;
    case 4:
        playMelody(melodie_Minecraft, duree_Minecraft, taille_Minecraft);
        break;
    default:
        break;
    }
}

void Melodie::playMelody(int melodie[], int duree[], int taille)
{
    _melody = melodie;
    _durations = duree;
    _len = taille;
    _idx = 0;
    _playingMelody = true;
    _nextChange = millis();
}

void Melodie::stop()
{
    noTone(_pin);
    _playingMelody = false;
    _playingBeep = false;
}

void Melodie::bib(int count, int freq, uint32_t dur, uint32_t gap)
{
    _beepCount = count;
    _beepFreq = freq;
    _beepDur = dur;
    _beepGap = gap;
    _beepIdx = 0;
    _playingBeep = true;
    _beepNext = millis();
}

void Melodie::bib(int count, int freq, uint32_t dur)
{
    _beepCount = count;
    _beepFreq = freq;
    _beepDur = dur;
    _beepGap = dur;
    _beepIdx = 0;
    _playingBeep = true;
    _beepNext = millis();
}

void Melodie::update()
{
    uint32_t now = millis();

    // Gestion des bips en priorité
    if (_playingBeep)
    {
        if (now >= _beepNext)
        {
            if (_beepIdx < _beepCount * 2)
            {
                // paire idx pair = tone, impair = silence
                if (_beepIdx % 2 == 0)
                {
                    tone(_pin, _beepFreq);
                    _beepNext = now + _beepDur;
                }
                else
                {
                    noTone(_pin);
                    _beepNext = now + _beepGap;
                }
                _beepIdx++;
            }
            else
            {
                // finis les bips
                noTone(_pin);
                _playingBeep = false;
            }
        }
        return;
    }

    // Sinon gestion de la mélodie
    if (_playingMelody)
    {
        if (now >= _nextChange)
        {
            if (_idx < _len)
            {
                int note = _melody[_idx];
                int d = _durations[_idx];
                // calcul ms: d divisions de noire => 1000/dur durée (noire=1000ms)
                uint32_t noteDur = 1000UL / d;
                if (note > 0)
                    tone(_pin, note);
                else
                    noTone(_pin);
                _nextChange = now + noteDur;
                _idx++;
            }
            else
            {
                // fin de mélodie
                noTone(_pin);
                _playingMelody = false;
            }
        }
    }
}