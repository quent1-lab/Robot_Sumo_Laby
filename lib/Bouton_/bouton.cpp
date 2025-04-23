
#include <Arduino.h>
#include <stdio.h>
#include <inttypes.h>

#include "bouton.h"



bouton::bouton()
{
    
}

void bouton::begin(int pin, bool type_bt, int delay_click, int delay_press, int delay_rebond)
{
    PIN = pin;
    pinMode(pin,INPUT);
    DELAY_CLICK = delay_click;
    DELAY_PRESS = delay_press;
    DELAY_REBOND = delay_rebond;
    DELAY_RESET = 2500;
    TYPE = type_bt;
    TIME_BT = millis();
}

void bouton::read_Bt()
{
    timer_reset();
    if (d_read() == !TYPE && ETAT == 0)
    {
        ETAT = 1;
        reset();
    }
    if (d_read() == !TYPE && ETAT == 1 && timer(DELAY_PRESS))
    {
        ETAT = 3;
        return;
    }
    else if (d_read() == TYPE && ETAT == 1 && timer(DELAY_CLICK))
    {
        ETAT = 2;
        return;
    }
}

bool bouton::click()
{
    if (ETAT == 2)
    {
        ETAT = 0;
        return true;
    } else return false;
}

bool bouton::press()
{
    if (ETAT == 3)
    {
        ETAT = 0;
        reset();
        return true;
    } else return false;
}

int bouton::d_read()
{
    return digitalRead(PIN);
}

void bouton::reset()
{
    TIME_BT = millis();
}

bool bouton::timer(int delay)
{
    if (millis() > TIME_BT + delay){
        return true;
    } else return false;
}

int bouton::etat(){
    return ETAT;
}

void bouton::timer_reset(){
    if(ETAT != 0 && timer(DELAY_RESET)){
        ETAT = 0;
    }
}