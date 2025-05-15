#include <Arduino.h>

#include <SharpIRArray.h>

#include <Adafruit_ADS1X15.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <controleMoteur.h>
#include <freertos/FreeRTOS.h>
#include <melodie.h>
#include <Encodeur.h>
#include <BluetoothSerial.h>

BluetoothSerial SerialBT;
QueueHandle_t queue;
QueueHandle_t queueEnvoie;

#define MAX_COMMANDE 100 // Valeur maximale de la commande moteur

// ----------------------- Déclaration des variables des moteurs ---------------------

ControleMoteur moteurs(19, 18, 5, 17);
int vitesseMoteurG = 0;
int vitesseMoteurD = 0;

// ------------------------- Déclaration des variables des pins ------------------------

const int pinLed = 23;
const int pinBuzzer = 26;
const int pinEncD_A = 16;
const int pinEncD_B = 4;
const int pinEncG_A = 38;
const int pinEncG_B = 35;
const int pinBatterie = 34;

// ------------------------- Déclaration des variables pour la mélodie ------------------------

Melodie music(pinBuzzer);

// ------------------------- Déclaration des variables des encodeurs ------------------------

Encodeur encodeur(pinEncD_B, pinEncD_A, pinEncG_A, pinEncG_B);
int countD = 0;
int countG = 0;

// ------------------------- Déclaration des variables des capteurs de distance ------------------------

Adafruit_ADS1115 ads; // Adresse I2C de l'ADS1115
bool ads_ok = true;
int16_t dist_Av_G, dist_Av_D, dist_Ar_G, dist_Ar_D;

SharpIRArray sharpArray(ads);

// ------------------------- Déclaration des variables du mpu ------------------------

Adafruit_MPU6050 mpu;
sensors_event_t a, g, temp;
bool mpu_ok = true;

bool tension_ok = true;

char FlagCalcul = 0;
float Te = 10;   // période d'échantillonage en ms
float Tau = 250; // constante de temps du filtre en ms
float Tau_v = 80;

// coefficient du filtre
float A, B, A_v, B_v;

// angle projeté et gyro
float thetaG, thetaOmegaR;

// angle filtre
float thetaGF, thetaOmegaRF, thetaFC;

float theta0 = 0; // angle d'équilibre

// ----------------------- Déclaration des variables PID -----------------------

// Constantes du régulateur PID
float kp = 850.0; // Gain proportionnel
float ki = 0.0;
float kd = 95.0; // Gain dérivé

// Variables globales pour le PID
float terme_prop = 0.0;
float terme_deriv = 0.0;
float erreur_cumulee = 0.0;
float erreur_precedente = 0.0;
float commande = 0.0;
float erreur = 0.0;

// ----------------------- Déclaration des variables PID pour l'angle -----------------------

// Constantes du régulateur PID
float kp_v = 0.0002;   // Gain proportionnel
float ki_v = 0.000002; // Gain intégral
float kd_v = 0.00000;  // Gain dérivé

// Variables globales pour le PID
float terme_prop_v = 0.0;
float terme_deriv_v = 0.0;
float erreur_cumulee_v = 0.0;
float erreur_precedente_v = 0.0;
float commande_v = 0.0;
float erreur_v = 0.0;
float max_commande_v = 0.5;

float consigne_v = 0.0;

float vitesse = 0.0;
float vitesse_F = 0.0;
float vitesse_prec = 0.0;
float pos_x = 0;
float pos_x_prec = 0.0;

float kp_d = 0.0001;
float ki_d = 0.0000001;
float terme_prop_d = 0.0;
float erreur_cumulee_d = 0.0;
float erreur_d = 0.0;
float rayon_estime = 0.0;
float rayon_consigne = 0.0;
float direction = 0.0;
int countD_prec = 0;
int countG_prec = 0;

// ----------------------- Déclaration des fonctions -----------------------

void getInclinaison(float &angleX, float &angleY, float &angleZ);
float asservissementVitesse(float consigne, float mesure);
void reception(char ch);
void readDistance();

// --------------------- Fonction de calcul des angles ---------------------

void controle(void *parameters)
{
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();
  while (1)
  {

    if (mpu_ok)
    {
      // Acquisition
      mpu.getEvent(&a, &g, &temp);

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

    countD = encodeur.get_countD();
    countG = encodeur.get_countG();
    // encodeur.odometrie();
    // pos_x = encodeur.get_x();

    pos_x = 1.0 * (countD + countG) / 2;

    vitesse = (-1.0) * (pos_x - pos_x_prec) * 1000.0 / Te;

    // Ajout d'un filtre passe bas sur la vitesse
    vitesse_F = A_v * (vitesse + B_v * vitesse_prec);
    vitesse_prec = vitesse_F;
    pos_x_prec = pos_x;

    countD_prec = countD;
    countG_prec = countG;

    if (tension_ok == false)
    {
      moteurs.setVitesses(0, 0);
    }
    else
    {
      // float theta_consigne = asservissementVitesse(consigne_v, vitesse_F);
      float coefDirD = (rayon_consigne > 0) ? rayon_consigne : 0;
      float coefDirG = (rayon_consigne > 0) ? 0 : rayon_consigne;
      moteurs.setVitesses(100 * consigne_v + coefDirD, 100 * consigne_v + coefDirG);
    }

    moteurs.updateMoteurs();
    FlagCalcul = 1;

    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(Te));
  }
}

// --------------------- Fonction de bluethoot ---------------------

void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
  uint16_t RxTaille, i;
  if (event == ESP_SPP_SRV_OPEN_EVT)
  {
    Serial.println("Client connecté à l'adresse : ");
    for (i = 0; i < 6; i++)
    {
      Serial.printf("%02X", param->srv_open.rem_bda[i]);
      if (i < 5)
      {
        Serial.print(":");
      }
    }
    Serial.println("");
    // Créer la file d'attente lors de la connexion du client
    queue = xQueueCreate(10, sizeof(uint16_t));
  }

  if (event == ESP_SPP_DATA_IND_EVT)
  {
    RxTaille = param->data_ind.len;
    // Envoyer la taille des données à la file d'attente
    if (queue != NULL)
    {
      xQueueSend(queue, &RxTaille, portMAX_DELAY);
    }
    else
    {
      Serial.println("Erreur : la file d'attente n'a pas été créée.");
    }
  }
}

void vReceptionBT(void *pvParameters)
{
  char buffer[101]; // +1 pour le caractère nul de fin de chaîne
  uint16_t data;
  while (1)
  {
    if (queue == NULL)
    {
      vTaskDelay(10 / portTICK_PERIOD_MS);
      continue;
    }
    if (xQueueReceive(queue, &data, portMAX_DELAY))
    {
      int bytesRead = SerialBT.readBytes(buffer, data);
      buffer[bytesRead] = '\0'; // Assurez-vous que la chaîne est terminée par un caractère nul
      // Serial.printf("Données reçues : %s", buffer);
      for (int i = 0; i < bytesRead; i++)
      {
        reception(buffer[i]);
      }
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void vEnvoieBT(void *pvParameters)
{
  char *data;
  while (1)
  {
    if (queueEnvoie == NULL)
    {
      vTaskDelay(10 / portTICK_PERIOD_MS);
      continue;
    }
    if (xQueueReceive(queueEnvoie, &data, portMAX_DELAY))
    {
      SerialBT.printf("%s", data);
    }
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}

void setup()
{
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(400000); // ← I2C Fast Mode 400 kHz

  pinMode(pinLed, OUTPUT);///
  pinMode(pinBuzzer, OUTPUT);
  pinMode(pinBatterie, INPUT);
  pinMode(pinEncD_A, INPUT);
  pinMode(pinEncD_B, INPUT);
  pinMode(pinEncG_A, INPUT);
  pinMode(pinEncG_B, INPUT);

  // Initialisation de la communication avec le MPU6050
  if (!mpu.begin())
  {
    Serial.println("Failed to find MPU6050 chip");
    mpu_ok = false;
  }
  else
  {
    Serial.println("MPU6050 Found!");
  }

  sharpArray.begin();

  xTaskCreate(controle, "controle", 10000, NULL, 5, NULL);
  xTaskCreate(vReceptionBT, "vReceptionBT", 10000, NULL, 8, NULL);
  xTaskCreate(vEnvoieBT, "vEnvoieBT", 10000, NULL, 10, NULL);

  queueEnvoie = xQueueCreate(100, sizeof(char *));

  // calcul coeff filtre
  A = 1 / (1 + Tau / Te);
  B = Tau / Te;
  A_v = 1 / (1 + Tau_v / Te);
  B_v = Tau_v / Te;

  encodeur.init(0, 0, 0, 34, 255, 22, 34);

  SerialBT.begin("Robot_SUMO"); // Nom du module bluetooth
  SerialBT.register_callback(callback);

  moteurs.setAlphaFrottement(0.15);
  delay(100);
  music.bib(2, 2000, 100, 100);
}

void reception(char ch)
{

  static int i = 0;
  static String chaine = "";
  String commande;
  String valeur;
  int index, length;

  if ((ch == 13) or (ch == 10) or (ch == 42))
  {
    index = chaine.indexOf(' ');
    length = chaine.length();
    if (index == -1)
    {
      commande = chaine;
      valeur = "";
    }
    else
    {
      commande = chaine.substring(0, index);
      valeur = chaine.substring(index + 1, length);
    }
    Serial.printf("Commande : %s | Valeur : %s\n", commande.c_str(), valeur.c_str());
    if (commande == "kp_v")
    {
      kp_v = valeur.toFloat();
      kp_v /= 1000.0;
    }
    if (commande == "kd_v")
    {
      kd_v = valeur.toFloat();
      kd_v /= 1000.0;
    }
    if (commande == "ki_v")
    {
      ki_v = valeur.toFloat();
    }
    if (commande == "kp")
    {
      kp = valeur.toFloat();
    }
    if (commande == "kd")
    {
      kd = valeur.toFloat();
    }
    if (commande == "ki")
    {
      ki = valeur.toFloat();
    }
    if (commande == "af")
    {
      moteurs.setAlphaFrottement(valeur.toFloat());
    }
    if (commande == "th")
    {
      theta0 = valeur.toFloat();
    }
    if (commande == "to_v")
    {
      Tau_v = valeur.toInt();
      A_v = 1 / (1 + Tau_v / Te);
      B_v = Tau_v / Te;
    }
    if (commande == "max_c")
    {
      max_commande_v = valeur.toFloat();
    }
    if (commande == "kp_d")
    {
      kp_d = valeur.toFloat();
    }
    if (commande == "led")
    {
      digitalWrite(pinLed, HIGH);
    }
    if (commande == "led_off")
    {
      digitalWrite(pinLed, LOW);
    }
    if (commande == "mel")
    {
      music.choisirMelodie(valeur.toInt());
    }
    if (commande == "co")
    {
      valeur += "*";
      int indexX = valeur.indexOf('X');
      int indexY = valeur.indexOf('Y');
      int indexEnd = valeur.indexOf('*');

      if (indexX != -1 && indexY != -1 && indexEnd != -1)
      {
        String valeurX = valeur.substring(indexX + 1, indexY);
        String valeurY = valeur.substring(indexY + 1, indexEnd);

        // Convertir les valeurs en entiers
        int valX = valeurX.toInt();
        int valY = valeurY.toInt();

        consigne_v = valY / 300.0;
        rayon_consigne = valX / 300.0;
        // Serial.println(consigne_v);
      }
    }

    chaine = "";
  }
  else
  {
    chaine += ch;
  }
}

void loop()
{
  music.update();
  sharpArray.update();
  readDistance();

  if (FlagCalcul == 1)
  {
    /*// Allouer de la mémoire pour la chaîne à envoyer
    char *bufferSend = (char *)malloc(100 * sizeof(char));
    if (bufferSend == NULL)
    {
      // Gérer l'erreur d'allocation de mémoire ici
    }

    sprintf(bufferSend, "%3.2lf %3.2lf %3.2lf \n", thetaFC, vitesse_F, consigne_v);
    xQueueSend(queueEnvoie, &bufferSend, portMAX_DELAY);

    FlagCalcul = 0;

    // N'oubliez pas de libérer la mémoire une fois que vous avez fini de l'utiliser
    free(bufferSend);*/

    // printf("%3.5lf %3.5lf %5.6lf %1.6lf \n", vitesse_F, terme_prop_v, direction, commande_v);
  }

  // Obtenir les angles d'inclinaison
  // float angleX, angleY, angleZ;
  // getInclinaison(angleX, angleY, angleZ);

  // Afficher les angles sur le moniteur série
  // Serial.printf("Inclinaison - X: %3.2f°, Y: %3.2f°, Z: %3.2f°\n", angleX, angleY, angleZ);

  // Calcul de la tension de la batterie
  float tensionMesuree = analogRead(pinBatterie) * (3.3 / 4095.0) + 0.31; // Conversion en tension mesurée
  float tension = 3.472 * tensionMesuree + 0.028;            // Conversion en tension réelle
  //Serial.printf("Tension: %3.2fV | Tension mesuré: %3.2fV | Valeur: %d\n", tension, tensionMesuree, readAnalog);

  if (tension < 6.9)
  {
    digitalWrite(pinLed, HIGH);
    // tension_ok = false;
    music.bib(3, 2000, 200, 100);
  }
  else
  {
    digitalWrite(pinLed, LOW);
  }

  delay(50); // Attendre un peu avant la prochaine lecture
}

void readDistance()
{
  if (ads_ok)
  {
    sharpArray.update();

    for (int i = 0; i < 4; ++i)
    {
      Serial.print("Capteur ");
      Serial.print(i);
      Serial.print(": ");
      Serial.print(sharpArray.getDistanceMM(i));
      Serial.println(" mm");
    }
  }
  else
  {
    dist_Av_G = 0;
    dist_Av_D = 0;
    dist_Ar_G = 0;
    dist_Ar_D = 0;
  }
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

float asservissementVitesse(float consigne, float mesure)
{
  // Boucle d'asservissement en pas à pas pour le calcul de la consigne de la boucle en position
  // Entrée : consigne en milimètres
  // Entrée : mesure en milimètres
  // Sortie : consigne theta en radian

  // Calcul de l'erreur
  erreur_v = consigne - mesure;

  // Calcul des termes PID
  terme_prop_v = kp_v * erreur_v;
  terme_deriv_v = kd_v * (erreur_v - erreur_precedente_v) * 1000.0 / Te;
  erreur_cumulee_v += ki_v * erreur_v;

  erreur_cumulee_v = constrain(erreur_cumulee_v, -0.1, 0.1);

  // Calcul de la commande finale
  commande_v = terme_prop_v + terme_deriv_v + erreur_cumulee_v;

  // Limiter la commande pour éviter des valeurs excessives
  commande_v = constrain(commande_v, -max_commande_v, max_commande_v);

  // Mettre à jour l'erreur précédente pour le terme dérivé
  erreur_precedente_v = erreur_v;

  // Retourner la consigne theta
  return commande_v;
}

void serialEvent()
{
  while (Serial.available() > 0) // tant qu'il y a des caractères à lire
  {
    reception(Serial.read());
  }
}