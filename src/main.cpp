#include <Arduino.h>
#include <Wire.h>

#include "WebUI.h"

#include <SharpIRArray.h>
#include <controleMoteur.h>
#include <melodie.h>
#include <bouton.h>
#include <Encodeur.h>
#include "LineSensor.h"
#include "ecranOLED.h"

#include <Adafruit_ADS1X15.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include <freertos/FreeRTOS.h>

#define MAX_COMMANDE 100 // Valeur maximale de la commande moteur

// ----------------------- Déclaration des variables des moteurs ---------------------

ControleMoteur moteurs(19, 18, 5, 17);
int vitesseMoteurG = 0;
int vitesseMoteurD = 0;

// Durée (ms) de chaque phase de test
constexpr uint32_t PHASE_DURATION = 3000;
// Intervalle entre chaque phase (ms)
constexpr uint32_t PAUSE_DURATION = 1000;

// ------------------------- Déclaration des variables des pins ------------------------

const int pinLed = 13;
const int pinBuzzer = 12;
const int pinStartstop = 14;

//a vérifier
const int pinEncD_A = 16;
const int pinEncD_B = 4;
const int pinEncG_A = 38;
const int pinEncG_B = 35;
const int pinBatterie = 34;

// ------------------------- Déclaration écran OLED ------------------------

ecranOLED ecran;

/// ------------------------- Déclaration des variables du start and stop ------------------------
bool start = true; // Variable pour le démarrage

// ------------------------- Déclaration des variables pour la mélodie ------------------------

Melodie music(pinBuzzer);

// ------------------------- Déclaration des variables du bouton ------------------------
Bouton bt;  // Pin du bouton et temps d'appui pour le déclenchement
Bouton bt2; // Pin du bouton et temps d'appui pour le déclenchement

// ------------------------- Déclaration des variables des capteurs de distance ------------------------

int16_t dist_Av_G, dist_Av_D, dist_Ar_G, dist_Ar_D;

SharpIRArray sharpArray; // Capteur Sharp IR

// ------------------------- Déclaration des variables des capteurs de ligne ------------------------

// Déclaration des broches : 7 capteurs
const uint8_t pins[5] = {33, 36, 35, 32, 39};
float lineSensorsValue[5] = {0, 0, 0, 0, 0};
int backPin[2] = {25, 26};
int backLineSensorsValue[2] = {0, 0};
LineSensor lineSensors(pins, 5);
float consigne = 0;
unsigned long lastTime = millis();
float Kp = 0.3;
float Kd = 0.1;
float K = 1;
float commande;

// ------------------------- Déclaration des variables du mpu ------------------------

Adafruit_MPU6050 mpu;
sensors_event_t a, g, temp;
bool mpu_ok = true;

bool tension_ok = true;

char FlagCalcul = 0;

// ----------------------- Déclaration des variables PID -----------------------

// ----------------------- Déclaration des variables PID pour l'angle -----------------------

// ----------------------- Déclaration des fonctions -----------------------

void getInclinaison(float &angleX, float &angleY, float &angleZ);
float asservissementVitesse(float consigne, float mesure);

// --------------------- Fonction de calcul des angles ---------------------

void controle(void *parameters)
{
  // TickType_t xLastWakeTime;
  // xLastWakeTime = xTaskGetTickCount();
  while (0)
  {

    if (mpu_ok)
    {
      // Acquisition
      // mpu.getEvent(&a, &g, &temp);

      // Calcul des angles
      // thetaG = atan2(a.acceleration.y, a.acceleration.x); // Angle projeté
      // thetaOmegaR = -g.gyro.z * Tau / 1000;

      // // Appliquer le filtre passe-bas
      // thetaGF = A * (thetaG + B * thetaGF);
      // // Appliquer le filtre passe-haut sur l'angle gyro
      // thetaOmegaRF = A * (thetaOmegaR + B * thetaOmegaRF);

      // // Filtre complémentaire
      // thetaFC = thetaGF + thetaOmegaRF;
    }

    if (tension_ok == false)
    {
      // moteurs.setVitesses(0, 0);
    }
    else
    {
      // float theta_consigne = asservissementVitesse(consigne_v, vitesse_F);
      // float coefDirD = (rayon_consigne > 0) ? rayon_consigne : 0;
      // float coefDirG = (rayon_consigne > 0) ? 0 : rayon_consigne;
      // moteurs.setVitesses(100 * consigne_v + coefDirD, 100 * consigne_v + coefDirG);
    }

    // moteurs.updateMoteurs();
    FlagCalcul = 1;

    // vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(Te));
  }
}

// --------------------- Variable ---------------------
// Variables exposées sur l’UI
int dflAvG = 0, dflAvD = 0, dflArG = 0, dflArD = 0;
float tension = 0;
float consG = 0, consD = 0;


WebUI ui("POCO_QT1", "Oul0uCoupTer4321", "robot-sumo");

// --------------------- Wifi debug --------------------


const char *ssid = "ESP32_AP";
const char *password = "12345678";

WiFiUDP udp;
const int udpPort = 4210;

void sendUDP() {
  udp.beginPacket(udp.remoteIP(), udp.remotePort());
  const char* data = "test";
  udp.write((const uint8_t*)data, strlen(data));
  udp.endPacket();
}

void readUDP() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char buffer[255];
    int len = udp.read(buffer, 255);
    if (len > 0) buffer[len] = '\0';

    Serial.print("Reçu : ");
    Serial.println(buffer);
  }
}

// --------------------- Serial debug ---------------------

void sendSerial(){

  Serial.print("SUMO-P ");   //Header

  //Line sensors
  for (uint8_t i = 0; i < 5; i++) {
    Serial.print(lineSensorsValue[i], 3);
    Serial.print(" ");
  }
  for (uint8_t i = 0; i < 2; i++) {
    Serial.print(backLineSensorsValue[i], 3);
    Serial.print(" ");
  }

  //Distance sensors & motors
  Serial.printf("%d %d %d %d ", dflAvG, dflAvD, dflArG, dflArD);

  //Tension batterie
  int pourcentage = static_cast<int>((tension - 6.5) / (8.5 - 6.5) * 100);
  Serial.printf("%d", pourcentage);
  
}

void readSerial()
{
  if (Serial.available())
  {
    String input = Serial.readStringUntil('\n');
    if (input.startsWith("SUMO-M ")) {
      //Commande moteur
      // Lecture brute des données (ex: "SUMO-M \x00\x80")
      // On récupère les deux octets après "SUMO-M "
      if (input.length() >= 9) {
        // Les caractères après "SUMO-M " sont les octets bruts (en ASCII)
        uint8_t index = (uint8_t)input[7];
        uint8_t vitesse = (uint8_t)input[8];

        if (index == 0) {
          vitesseMoteurG = (int8_t)vitesse; // conversion signed
        } else if (index == 1) {
          vitesseMoteurD = (int8_t)vitesse; // conversion signed
        }

        //  Appliquer les consignes aux moteurs
        consG = constrain(vitesseMoteurG, -MAX_COMMANDE, MAX_COMMANDE);
        consD = constrain(vitesseMoteurD, -MAX_COMMANDE, MAX_COMMANDE);
        moteurs.setSpeed(vitesseMoteurG, vitesseMoteurD);
        moteurs.update();

      }

    } else if (input.startsWith("SUMO-B ")) {
      //Commande buzzer
      // Lecture brute des données (ex: "SUMO-B \x02")
      // On récupère les deux octets après "SUMO-B "
      if (input.length() >= 8) {
        // Les caractères après "SUMO-B " sont les octets bruts (en ASCII)
        uint8_t musique = (uint8_t)input[7];
        music.choisirMelodie(musique);
      }
    } else if (input.startsWith("SUMO-L ")) {
      //Commande LED

      digitalWrite(pinLed, !digitalRead(pinLed));

    }

    //Mode debug
    ecran.afficherTexte("MODE DEBUG");

  }
}




// --------------------- Main ---------------------

void setup()
{
  Serial.begin(115200);
  Wire.begin();          // SDA, SCL
  Wire.setClock(400000); // ← I2C Fast Mode 400 kHz

  // ---------- WIFI UDP debug --------------
  /*
  // Créer le point d'accès
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);  // Ex : 192.168.4.1

  udp.begin(udpPort);
  Serial.println("UDP serveur démarré");
  */
  // ----------------------------------

  pinMode(pinLed, OUTPUT);
  pinMode(pinBatterie, INPUT);
  pinMode(pinStartstop, INPUT);

  sharpArray.begin();
  lineSensors.begin();
  pinMode(backPin[0], INPUT);
  pinMode(backPin[1], INPUT);

  //bt.begin(bouton1, false); // Pin du bouton et temps d'appui pour le déclenchement

  // — Initialisation capteurs —
  if (!mpu.begin())
  {
    Serial.println("MPU6050 absent");
    mpu_ok = false;
  }

  moteurs.begin();
  moteurs.attachMPU(mpu);
  moteurs.setFriction(49.0, 49.0);
  moteurs.setRampRate(200); // 200%/s
  moteurs.enableHeadingControl(false);
  moteurs.setHeadingTarget(0);
  moteurs.setSpeed(0, 0);

  ecran.begin();



  // — Instanciation des widgets (tuiles) —
  ui.exposeVariable("tension", VarType::FLOAT, []()
                    { return String(tension); }, [](const String &v)
                    { tension = v.toFloat(); });
  ui.exposeVariable("start", VarType::BOOL, []()
                    { return String(start); }, [](const String &v)
                    { start = v.toInt(); });
  ui.exposeVariable("Moteur Droit", VarType::FLOAT, []()
                    { return String(consD); }, [](const String &v)
                    { consD = v.toFloat(); });
  ui.exposeVariable("Moteur Gauche", VarType::FLOAT, []()
                    { return String(consG); }, [](const String &v)
                    { consG = v.toFloat(); });
  ui.exposeVariable("AvG", VarType::FLOAT, []()
                    { return String(dflAvG); }, [](const String &v)
                    { dflAvG = v.toInt(); });
  ui.exposeVariable("AvD", VarType::FLOAT, []()
                    { return String(dflAvD); }, [](const String &v)
                    { dflAvD = v.toInt(); });
  /*
  ui.exposeVariable("ArG", VarType::FLOAT, []()
                    { return String(dflArG); }, [](const String &v)
                    { dflArG = v.toInt(); });
  ui.exposeVariable("ArD", VarType::FLOAT, []()
                    { return String(dflArD); }, [](const String &v)
                    { dflArD = v.toInt(); });
  */
  ui.exposeVariable("Commande", VarType::FLOAT, []()
                    { return String(commande); }, [](const String &v)
                    { commande = v.toFloat(); });
  ui.exposeVariable("Kp", VarType::FLOAT, []()
                    { return String(Kp); }, [](const String &v)
                    { Kp = v.toFloat(); });
  ui.exposeVariable("Kd", VarType::FLOAT, []()
                    { return String(Kd); }, [](const String &v)
                    { Kd = v.toFloat(); });

  auto *btn = new ButtonWidget("Bib", 1500, 500);

  btn->onClick([]()
               { music.choisirMelodie(1); });

  // Widgets correspondants
  ui.addWidget(new LabelWidget("Tension", "tension", "V"));
  ui.addWidget(new LabelWidget("Start", "start"));
  ui.addWidget(btn);
  ui.addWidget(new SliderWidget("Kp", "kp", 0, 1, 0.05));
  ui.addWidget(new SliderWidget("Moteur Droit", "Moteur Droit", -100, 100, 1));
  ui.addWidget(new SliderWidget("Moteur Gauche", "Moteur Gauche", -100, 100, 1));
  ui.addWidget(new SliderWidget("AvG", "AvG", 0, 5000, 1));
  ui.addWidget(new SliderWidget("AvD", "AvD", 0, 5000, 1));
  //ui.addWidget(new SliderWidget("ArG", "ArG", 0, 5000, 1));
  //ui.addWidget(new SliderWidget("ArD", "ArD", 0, 5000, 1));
  ui.addWidget(new SliderWidget("Kp","Kp", 0, 10, 0.1));
  ui.addWidget(new SliderWidget("Kd", "Kd", 0, 10, 0.1));
  ui.addWidget(new LabelWidget("Commande", "Commande"));

  // Démarrage complet du serveur web
  ui.begin();

  // xTaskCreate(controle, "controle", 10000, NULL, 5, NULL);

  music.bib(1, 1000, 100, 100);
}

void loop()
{

  unsigned long startTime = micros(); // Start measuring time
  // 1) Mise à jour WebUI (broadcast périodique)
  ui.loop(500);
  music.update();
  sharpArray.update();
  //bt.update();
  lineSensors.update();

  lineSensors.getAllValues(lineSensorsValue);
  start = digitalRead(pinStartstop);
  backLineSensorsValue[0] = digitalRead(backPin[0]);
  backLineSensorsValue[1] = digitalRead(backPin[1]);

  if (bt.wasClicked())
  {
    start = !start;
  }
  
  // 2) Lecture capteurs distance
  if (sharpArray.getADSOk())
  {
    sharpArray.update();
    dflAvG = sharpArray.getDistanceMM(0);
    dflAvD = sharpArray.getDistanceMM(1);
    dflArG = sharpArray.getDistanceMM(2);
    dflArD = sharpArray.getDistanceMM(3);
    // Serial.printf("AvG: %d mm, AvD: %d mm, ArG: %d mm, ArD: %d mm\n", dflAvG, dflAvD, dflArG, dflArD);
  }
  else
  {
    dflAvG = dflAvD = dflArG = dflArD = 0;
  }

  // 4) Lecture tension batterie

  int raw = analogRead(pinBatterie);
  float vmes = raw * (3.3f / 4095.0f);
  tension = 3.472f * vmes + 0.6f; // Ajusté pour compenser l'offset

  /*
  // 5) Calcul de la consigne des moteurs pour suivre un objet à 10 cm
  float distanceCons = 100.0; // Distance cible en mm (10 cm)

  float consigneAvG = (dflAvD - distanceCons) * kp;
  float consigneAvD = (dflAvG - distanceCons) * kp;

  // Calcul de la consigne finale
  consG = constrain(consigneAvG, -MAX_COMMANDE, MAX_COMMANDE);
  consD = constrain(consigneAvD, -MAX_COMMANDE, MAX_COMMANDE);

  int consMotG = int(consG);
  int consMotD = int(consD);

  //  Appliquer les consignes aux moteurs
  if (start)
  {
    moteurs.setSpeed(consMotG, consMotD);
  }
  else
  {
    moteurs.stop();
  }
  moteurs.update();
  */



  //Envoi des données sur le port série
  sendSerial();
  //Lecture des données sur le port série
  readSerial();


  //sendUDP();
  //readUDP();

  unsigned long endTime = micros(); // End measuring time
  // Serial.printf("Execution time: %lu microseconds\n", endTime - startTime);

  delay(50);
}

void asservissementLigne() {

  float diff = lineSensorsValue[1] - lineSensorsValue[3];
  float error = consigne - diff;
  unsigned long dt = millis() - lastTime;
  commande = ((Kp + Kd/dt) * error) * K;

  /*
  //  Appliquer les consignes aux moteurs
  if (start)
  {
    moteurs.setSpeed(consMotG, consMotD);
  }
  else
  {
    moteurs.stop();
  }
  moteurs.update();
  */

  lastTime = millis();
}

void getInclinaison(float &angleX, float &angleY, float &angleZ)
{
  if (mpu_ok)
  {
    // Lecture des données du capteur
    mpu.getEvent(&a, &g, &temp);

    // Calcul des angles d'inclinaison
    angleX = atan2(a.acceleration.y, a.acceleration.z) * 180 / PI; // Inclinaison sur l'axe X
    angleY = atan2(a.acceleration.x, a.acceleration.z) * 180 / PI; // Inclinaison sur l'axe Y
    angleZ = atan2(a.acceleration.x, a.acceleration.y) * 180 / PI; // Inclinaison sur l'axe Z
  }
  else
  {
    // Si le capteur n'est pas initialisé, renvoyer des valeurs par défaut
    angleX = 0.0;
    angleY = 0.0;
    angleZ = 0.0;
  }
}