
/********************************************************************************************************************************************************************************************************
****                                                                                         AQUABOUNS v1.2                                                                                          ****
****                                                                         Automate pour la gestion d'aquarium recifal                                                                             ****
****                                                                                       Créé par Djbouns                                                                                          ****
****                                                                Avec l'aide du forum https://forum.arduino.cc pour le codage                                                                     ****
****                                                                                     Un grand merci a J.M.L                                                                                      ****
****                                                                                           05/01/2019                                                                                            ****
********************************************************************************************************************************************************************************************************/

/* AQUABOUNS, Automate pour la gestion d'aquarium recifal
   Copyright (C) 2018 par dj bouns

   Ce programme est un logiciel libre: vous pouvez le redistribuer
   et/ou le modifier selon les termes de la "GNU General Public
   License", tels que publiés par la "Free Software Foundation"; soit
   la version 2 de cette licence ou (à votre choix) toute version
   ultérieure.

   Ce programme est distribué dans l'espoir qu'il sera utile, mais
   SANS AUCUNE GARANTIE, ni explicite ni implicite; sans même les
   garanties de commercialisation ou d'adaptation dans un but spécifique.

   Se référer à la "GNU General Public License" pour plus de détails.
   Vous la trouverez dans l'onglet GNU_General_Public_License.h

   Vous devriez avoir reçu une copie de la "GNU General Public License"
   en même temps que ce programme; sinon, écrivez a la "Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA".*/

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ Librairies $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
#include <Wire.h>
#include <OneWire.h>
#include <avr/wdt.h>
#include "DallasTemperature.h"
#include "Nextion.h"

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ Fichiers $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
#include "global.h"
#include "affichage.h"
#include "autres.h"
#include "brassage.h"
#include "debug.h"
#include "eclairage.h"
#include "eeprom.h"
#include "flotteurs.h"
#include "gsm.h"
#include "horloge.h"
#include "oscillo.h"
#include "pin.h"
#include "carteSD.h"
#include "temperature_ph.h"
#include "wifi.h"

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ Déclarations $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
uint32_t roulementPrecedent1, roulementPrecedent2, roulementPrecedent3, compteurinitialisationPhDemarrage, compteurmesurePhDemarrage;
uint8_t roulement1 = 1;
uint8_t roulement2 = 1;
uint8_t nbr2MesurePhAuDemarage = 0;
float moyennePhBacDemmarage, moyennePhRacDemmarage;
const uint16_t refreshSwitch = 1000;
const uint16_t oneMinuteEnMillis = 60000u;
const uint32_t twoMinuteEnMillis = 120000ul;
const char texteNextion1[] PROGMEM = "Demarrage en cours, chargement des parametres";
const char texteNextion2[] PROGMEM = "Importation des parametres de l'eeprom, OK !";
const char texteNextion3[] PROGMEM = "Initialisation de l'Horloge... ";
const char texteNextion4[] PROGMEM = "Pas de reponse de l'Horloge !!!";
const char texteNextion5[] PROGMEM = "Initialisation de l'Horloge, OK !";
const char texteNextion6[] PROGMEM = "Initialisation des sondes de temperature ... ";
const char texteNextion7[] PROGMEM = "Aucune sonde de temperature detectee !!!";
const char texteNextion8[] PROGMEM = "UNE sonde de temperature detectee, OK !";
const char texteNextion9[] PROGMEM = "DEUX sondes de temperature detectees, OK !";
const char texteNextion10[] PROGMEM = "Initialisation de la carte SD... ";
const char texteNextion11[] PROGMEM = "Aucune reponse de la carte SD !!!";
const char texteNextion12[] PROGMEM = "Initialisation du lecteur de carte SD, OK.";
const char texteNextion13[] PROGMEM = "Initialisation du module Gsm ... ";
const char texteNextion14[] PROGMEM = "Aucune reponse du module Gsm !!!";
const char texteNextion15[] PROGMEM = "Erreur de passage en mode texte !!!";
const char texteNextion16[] PROGMEM = "Erreur de passage en mode routage !!!";
const char texteNextion17[] PROGMEM = "Initialisation du module Gsm, OK";
const char texteNextion18[] PROGMEM = "Initialisation du module Wifi ...";
const char texteNextion20[] PROGMEM = "BIENVENUE";
const char texteNextion21[] PROGMEM = "DANS";
const char texteNextion22[] PROGMEM = "L'AQUABOUN'S";
const char texteNextion23[] PROGMEM = "2019";
const char texteNextion24[] PROGMEM = "V1.2";
const char texteNextion25[] PROGMEM = "Importation des parametres par default, OK !";
const char texteNextion26[] PROGMEM = "Importation de ";
const char texteNextion27[] PROGMEM = "Pas de fichier ";
const char texteNextion29[] PROGMEM = "Initialisation du module Wifi, OK";
const char texteNextion30[] PROGMEM = "Aucune reponse du module Wifi !!!";
const char texteNextion31[] PROGMEM = "mesure PH en cours";
const char texteNextion32[] PROGMEM = "initialise    PH";
const char texteSurSD1[] PROGMEM = "demarage de l'aquabouns";

void loopDesFonctions() {
  if (millis() - roulementPrecedent1 > refreshSwitch) { // si compteur ateind
    roulementPrecedent1 = millis(); // reinitialise le compteur
    rafraichirHeureSiSurMenu(); // rafraichi l'heure
    flotteurs();
    oscillateur();
    coupureEdf();
  }
  if ( etalonnageOnOFF ) { /* si etalonnage en cours */
    const uint16_t refreshSwitchEtalonnagePh = 500;
    if (millis() - roulementPrecedent2 > refreshSwitchEtalonnagePh) { // rafraichi toute les 250 millis
      roulementPrecedent2 = millis(); // reinitialise le compteur
      if ( initialisationPhDemarrage ) { // si initialisation du demarrage
        if (millis() - compteurinitialisationPhDemarrage > twoMinuteEnMillis) { // si le compteut ateind
          initialisationPhDemarrage = !initialisationPhDemarrage; // fin des mesure du demarage
          mesurePhDemarrage = true; // met en ON
          texteProgmemAuNextion(messageetalonnage, texteNextion31 , pas2Changement2Couleur); // champ, texte, couleur
          compteurmesurePhDemarrage = millis();
        }
      }
      else {
        ph(); // lance la prise de mesure PH pour une sonde puis switch
        lectureBoutons(); // lit le bouton pour ne pas avoir le bouton "etalonnage" sans reponse
        rafraichirPhBacNextion(); // MAJ du PH bac l'ecran
        lectureBoutons(); // lit le bouton pour ne pas avoir le bouton "etalonnage" sans reponse
        ph(); // lance la prise de mesure PH pour une sonde puis switch
        lectureBoutons(); // lit le bouton pour ne pas avoir le bouton "etalonnage" sans reponse
        rafraichirPhRacNextion(); // MAJ du PH rac l'ecran
        lectureBoutons(); // lit le bouton pour ne pas avoir le bouton "etalonnage" sans reponse
        if ( mesurePhDemarrage ) { // si pendent les mesure Ph du demarrage
          if (millis() - compteurmesurePhDemarrage > oneMinuteEnMillis) { // si le compteut ateind
            etalonnageOFF(); // lance la fonction
            mesurePhDemarrage = !mesurePhDemarrage;
          }
        }
      }
    }
  }
  else { /* si pas d'etalonnage en cours lance les fonction a tour de role */
    if (millis() - roulementPrecedent2 > (refreshSwitch * dix)) { // quand compteur ateint
      roulementPrecedent2 = millis(); // reinitialise le compteur
      switch (roulement1) {
        case 1:
          ph();
          roulement1++;
          break;
        case 2:
          eclairage();
          roulement1++;
          break;
        case 3:
          ph();
          roulement1++;
          break;
        case 4:
          temperature();
          roulement1++;
          break;
        case 5:
          brassage();
          roulement1++;
          break;
        case 6:
          if (variable.minut == zero) { // toute les heures a 00 minute
            etatGSM();
          }
          else if (variable.minut == dix) { // toute les heure a 10 minutes
            etatWifi();
          }
          coupureBatterie();
          roulement1 = 1;
          break;
      }
    }
    /* rafraichi les variable du menu */
    if (millis() - roulementPrecedent3 > (refreshSwitch * trois)) {
      roulementPrecedent3 = millis(); // reinitialise le compteur
      switch (roulement2) {
        case 1 :
          rafraichirPhBacNextion();
          alertePhBac();
          roulement2++;
          break;
        case  2:
          rafraichirPhRacNextion();
          roulement2++;
          break;
        case 3 :
          rafraichirTemperatureBacNextion();
          roulement2++;
          break;
        case  4:
          rafraichirTemperatureRampeNextion();
          roulement2++;
          break;
        case 5 :
          rafraichirEclairageBlancNextion();
          roulement2++;
          break;
        case  6:
          rafraichirEclairageBleuNextion();
          roulement2++;
          break;
        case 7:
          rafraichirBrassage1Nextion();
          roulement2++;
          break;
        case  8:
          rafraichirBrassage2Nextion();
          roulement2++;
          break;
        case 9:
          rafraichirBrassage3Nextion();
          roulement2 = 1;
          break;
      }
    }
  }
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ SETUP $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
void setup() {

  Serial.begin(74880); //configure la vitesse de communication du moniteur serie (74880 baud)

  analogReference(EXTERNAL);

  compteurinitialisationPhDemarrage = millis(); // lance le compteur d'atente avant la prise des mesure PH (la carte ph met 2 min a se stabiliser)

  // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< CONFIGURATION DES BROCHES DE L'ARDUINO
  /* pinmode */
  pinMode(pinOutEclairage[bleu1], OUTPUT); // Définit la broche comme sortie
  pinMode(pinOutEclairage[bleu2], OUTPUT); // Définit la broche comme sortie
  pinMode(pinOutEclairage[blanc1], OUTPUT); // Définit la broche comme sortie
  pinMode(pinOutEclairage[blanc2], OUTPUT); // Définit la broche comme sortie
  pinMode(pinOutBrassage[numeroUn], OUTPUT); // Définit la broche comme sortie
  pinMode(pinOutBrassage[numeroDeux], OUTPUT); // Définit la broche comme sortie
  pinMode(pinOutBrassage[numeroTrois], OUTPUT); // Définit la broche comme sortie
  oscillo[numeroUn].attach(pinOUTOscillo1); // Définit la broche comme sortie
  oscillo[numeroDeux].attach(pinOUTOscillo2); // Définit la broche comme sortie
  oscillo[numeroTrois].attach(pinOUTOscillo3); // Définit la broche comme sortie
  pinMode(pinInFloteurOsmolation, INPUT_PULLUP); // Définit la broche comme entrée et active la resistance pull-up
  pinMode(pinInFloteurNiveauEcumeur, INPUT_PULLUP); // Définit la broche comme entrée et active la resistance pull-up
  pinMode(pinInFloteurGodetEcumeur, INPUT_PULLUP); // Définit la broche comme entrée et active la resistance pull-up
  pinMode(pinInFloteurSecurite, INPUT_PULLUP); // Définit la broche comme entrée et active la resistance pull-up
  pinMode(pinInFloteurReserve, INPUT_PULLUP); // Définit la broche comme entrée et active la resistance pull-up
  pinMode(pinOutBatterie, OUTPUT); // Définit la broche comme sortie
  pinMode(pinOutRelaisRemontee, OUTPUT); // Définit la broche comme sortie
  pinMode(pinOutRelaisChauffage, OUTPUT); // Définit la broche comme sortie
  pinMode(pinOutRelaisVentilateurBac, OUTPUT); // Définit la broche comme sortie
  pinMode(pinOutRelaisVentilateurRampe, OUTPUT); // Définit la broche comme sortie
  pinMode(pinOutRelaisOsmolation, OUTPUT); // Définit la broche comme sortie
  pinMode(pinOutRelaisDistributeurNouriture, OUTPUT); // Définit la broche comme sortie
  pinMode(pinOutRelaisElectovanneRac, OUTPUT); // Définit la broche comme sortie
  pinMode(pinOutRelaisEcumeur, OUTPUT); // Définit la broche comme sortie
  pinMode(pinOutbuzzer, OUTPUT); // Définit la broche comme sortie
  pinMode(hardResetGSM, OUTPUT); // Définit la broche comme sortie
  pinMode(hardResetWIFI, OUTPUT); // Définit la broche comme sortie

  /* initialise etat des pin */
  digitalWrite(pinOutEclairage[bleu1], LOW); // Off
  digitalWrite(pinOutEclairage[bleu2], LOW); // Off
  digitalWrite(pinOutEclairage[blanc1], LOW); // Off
  digitalWrite(pinOutEclairage[blanc2], LOW); // Off
  digitalWrite(pinOutBrassage[numeroUn], LOW); // Off
  digitalWrite(pinOutBrassage[numeroDeux], LOW); // Off
  digitalWrite(pinOutBrassage[numeroTrois], LOW); // Off
  digitalWrite(pinOutBatterie, HIGH); // on
  digitalWrite(pinOutRelaisRemontee, LOW); // On
  digitalWrite(pinOutRelaisChauffage, LOW); // On
  digitalWrite(pinOutRelaisVentilateurBac, LOW); // Off
  digitalWrite(pinOutRelaisOsmolation, LOW); // Off
  digitalWrite(pinOutRelaisDistributeurNouriture, LOW); // Off
  digitalWrite(pinOutRelaisElectovanneRac, LOW); // Off
  digitalWrite(pinOutRelaisEcumeur, LOW); // On
  digitalWrite(pinOutbuzzer, LOW); // Off

  /* reboot gsm  avant initialisation */
  reboot(hardResetGSM);

  /* Initialisation de l'ecran nextion
       configure la vitesse de communication (115200 baud)
       Communication sur RX2/TX2
       !!! IMPORTANT !!! >>> Si vous utiliser la biblioteque officiel telecharger a partir de https://github.com/itead/ITEADLIB_Arduino_Nextion vous devez effectuer des modification :
       >>> Dans NexConfig.h, ligne 27, vous devez modifier #define DEBUG_SERIAL_ENABLE par:
       //#define DEBUG_SERIAL_ENABLE
       Cela désactive le debug dans le moniteur serie.
       >>> Toujours dans NexConfig.h, ligne 37, vous devez modifier #define nexSerial Serial1 par:
       #define nexSerial Serial2
       Serial2 correspond a RX2/TX2
       >>> Dans NexHardware.cpp, ligne 226, vous devez modifier nexSerial.begin(9600); par:
       nexSerial.begin(115200);
       Cela Correspond a la vitesse de communication (115200 baud) entre l'ecran et l'arduino <<< !!! IMPORTANT !!! */
  nexInit();
  pageActuelNextion = demarrage; // indique que l'ecran est sur la page de demarrage

  // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Debut de l'initialisation de l'aquaboun's

  /* affiche les messages de demarage sur l'ecran */
  texteProgmemAuNextion(texte1, texteNextion1, pas2Changement2Couleur); // champ, texte, couleur
  delay(deuxMille);

  /* Verifie si il y a des parametres dans l'eeprom
    Si il n'y a rien, chargement des parametre par default
    Sinon, chargement des parametre de l'eeprom */
  etablirValeursParDefaut();

  /* Initialise les oscillateurs */
  for (uint8_t numeroOscillo = numeroUn; numeroOscillo <= numeroTrois; numeroOscillo++) { // effectue la fonction une fois par oscillateurs
    oscillo[numeroOscillo].write(((sauvegarde.angle2Oscillo[numeroOscillo] - sauvegarde.angle1Oscillo[numeroOscillo]) / deux) + sauvegarde.angle1Oscillo[numeroOscillo]); // envoie l'angle a l'oscillateur
  }

  /* Initialise l'horloge */
  texteProgmemAuNextion(texte2, texteNextion3, pas2Changement2Couleur); // champ, texte, couleur
  delay(deuxMille);

  if (! rtc.begin()) { // si l'horloge n'est pas initialisé
    texteProgmemAuNextion(texte2, texteNextion4, rouge); // champ, texte, couleur
  }
  else { // si horloge bien initialisé
    texteProgmemAuNextion(texte2, texteNextion5, vert); // champ, texte, couleur

    /* pour mettre a jour votre horloge lors de la premiere utilisation il faut retirer les // au debut de la ligne ci dessous et televerser le programme sur l'arduino */
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // met a jour l'horloge en utilisant l'heure de televersement
  }
  delay(deuxMille);

  /* Initialise les sondes de temperature ds18b20 */
  sensors.begin();
  texteProgmemAuNextion(texte3, texteNextion6, pas2Changement2Couleur); // champ, texte, couleur
  delay(deuxMille);

  if (!sensors.getDeviceCount()) { // si aucune sonde de temperature detectée
    texteProgmemAuNextion(texte3, texteNextion7, rouge); // champ, texte, couleur
  }
  else {
    if (sensors.getDeviceCount() == un) { // Si 1 sonde detectée
      texteProgmemAuNextion(texte3, texteNextion8, orange); // champ, texte, couleur
    }
    else if (sensors.getDeviceCount() == deux) { // si 2 sondes detectées
      texteProgmemAuNextion(texte3, texteNextion9, vert); // champ, texte, couleur
    }
    sensors.getAddress(sondeBac, sauvegarde.adresseSondeBac) ; // indique l'adresse de la sonde
    sensors.getAddress(sondeRampe, sauvegarde.adresseSondeRampe); // indique l'adresse de la sonde
    sensors.setResolution(sondeBac, 12);//resolution 9 = 0.5°= 93.75ms pour mesurer;, 10 = 0.25° = 187.5ms pour mesurer , 11 = 0.125° = 375ms pour mesurer, 12 = 0.0625° = 750ms pour mesurer
    sensors.setResolution(sondeRampe, 12);//resolution 9 = 0.5°= 93.75ms pour mesurer;, 10 = 0.25° = 187.5ms pour mesurer , 11 = 0.125° = 375ms pour mesurer, 12 = 0.0625° = 750ms pour mesurer
    sensors.setWaitForConversion(false); // on travaillera en asynchrone, on n'attend pas les lectures
    sensors.requestTemperatures(); // on lance une demande de lecture qui sera prête plus tard
  }
  delay(deuxMille);

  /* Initialisation de la carte SD */
  texteProgmemAuNextion(texte4, texteNextion10, pas2Changement2Couleur); // champ, texte, couleur
  delay(deuxMille);

  if (!SD.begin(SD_CS_PIN)) { // Si carte SD non détéctée
    texteProgmemAuNextion(texte4, texteNextion11, rouge); // champ, texte, couleur
  }
  else { // Si carte SD bien initialisée
    texteProgmemAuNextion(texte4, texteNextion12, vert); // champ, texte, couleur
    delay(mille);

    lisSurSd(ssidSurSD, identifiantWifi.ssid, maxSsid); // lance la lecture/copie du fichier
    lisSurSd(mdpSurSD, identifiantWifi.mdp, maxMdp); // lance la lecture/copie du fichier
    lisSurSd(numero2TelSurSD, numero2tel, maxnumero2tel); // lance la lecture/copie du fichier
  }

  /* initialise le module wifi */
  gsm.begin(115200); // configure la vitesse de communication avec l'arduino (115200 baud)
  texteProgmemAuNextion(texte5, texteNextion13, pas2Changement2Couleur); // champ, texte, couleur
  delay(deuxMille);

  if (gsmPrintlnAndWaitATCommand(ATString, OKLongString, mille, true)) {
    if (!gsmPrintlnAndWaitATCommand("AT+CMGF=1", OKLongString, mille, true)) { // on passe les SMS en mode texte
      texteProgmemAuNextion(texte5, texteNextion15, rouge); // champ, texte, couleur
      gsmOn = false; // Met le GSM en OFF
    }
    else {
      if (!gsmPrintlnAndWaitATCommand("AT+CNMI=1,0,0,0,0", OKLongString, mille, true)) {  // on passe en mode routage des SMS vers le terminal
        texteProgmemAuNextion(texte5, texteNextion16, rouge); // champ, texte, couleur
        gsmOn = false; // Met le GSM en OFF
      }
      else {
        texteProgmemAuNextion(texte5, texteNextion17, vert); // champ, texte, couleur
      }
    }
  }
  else {
    texteProgmemAuNextion(texte5, texteNextion14, rouge); // champ, texte, couleur
    gsmOn = false; // Met le GSM en OFF
  }

  /* Initialise le module wifi */
  reboot(hardResetWIFI); // reboot le module wifi avant initialisation
  d1mini.begin(115200); // configure la vitesse de communication du module Wifi avec l'arduino (115200 baud)
  d1mini.setTimeout(500); // temps max de réceptoin d'une structure en ms
  texteProgmemAuNextion(texte6, texteNextion18, pas2Changement2Couleur); // champ, texte, couleur
  horaire(); // lance la fonction
  executer(RECEVOIRconnectionWifi, true); // demande le statut de connection // pour voir si le module est connecté et repond
  delay(deuxMille);
  if (connectionWifi.presenceModule ) {
    texteProgmemAuNextion(texte6, texteNextion29, vert); // champ, texte, couleur
  }
  else {
    texteProgmemAuNextion(texte6, texteNextion30, rouge); // champ, texte, couleur
  }
  delay(deuxMille);

  // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<  Initialisation de l'aquaboun's terminé
  texteProgmemAuNextion(texte1, vide, vert); // champ, texte, couleur
  texteProgmemAuNextion(texte2, vide, vert); // champ, texte, couleur
  texteProgmemAuNextion(texte3, vide, vert); // champ, texte, couleur
  texteProgmemAuNextion(texte4, vide, vert); // champ, texte, couleur
  texteProgmemAuNextion(texte5, vide, vert); // champ, texte, couleur
  texteProgmemAuNextion(texte6, vide, vert); // champ, texte, couleur
  delay(mille);
  texte1.setFont(8); // change de police
  texte2.setFont(8); // change de police
  texte3.setFont(8); // change de police
  texte4.setFont(8); // change de police
  texte5.setFont(8); // change de police
  texteProgmemAuNextion(texte1, texteNextion20, vert); // champ, texte, couleur
  texteProgmemAuNextion(texte2, texteNextion21, vert); // champ, texte, couleur
  texteProgmemAuNextion(texte3, texteNextion22, vert); // champ, texte, couleur
  texteProgmemAuNextion(texte4, texteNextion23, vert); // champ, texte, couleur
  texteProgmemAuNextion(texte5, texteNextion24, vert); // champ, texte, couleur
  delay(deuxMille);

  // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<  Attache chaque bouton de l'ecran a une fonction
  onoffpomperemonte.attachPush(boutonOnOffRemonte, &onoffpomperemonte);
  onoffalimentation1.attachPush(boutonOnOffAlim1, &onoffalimentation1);
  onoffalimentation2.attachPush(boutonOnOffAlim2, &onoffalimentation2);
  onoffpwmblanc.attachPush(boutonBlanc, &onoffpwmblanc);
  onoffpwmbleu.attachPush(boutonBleu, &onoffpwmbleu);
  onoffpwmbrassage1.attachPush(boutonBrassage1, &onoffpwmbrassage1);
  onoffpwmbrassage2.attachPush(boutonBrassage2, &onoffpwmbrassage2);
  onoffpwmbrassage3.attachPush(boutonBrassage3, &onoffpwmbrassage3);
  boutonetalonnage.attachPush(etalonnage, &boutonetalonnage);
  onofftempete.attachPush(boutonTempeteManuel, &onofftempete);
  onofftempetealeatoire.attachPush(boutonTempeteAleatoire, &onofftempetealeatoire);
  boutonmenuversconfig.attachPush(versConfig, &boutonmenuversconfig);
  boutonalimentationmanuel.attachPush(boutonNourrissageManuel, &boutonalimentationmanuel);
  boutonconfigverstel.attachPush(versTel, &boutonconfigverstel);
  boutonconfigverswifi.attachPush(versWifi, &boutonconfigverswifi);
  pageMenu.attachPop(versMenu, &pageMenu);
  boutonconfigversmenu.attachPush(versMenu, &boutonconfigversmenu);
  boutonconfigversinfo.attachPush(versInfo, &boutonconfigversinfo);
  boutonconfigvershorloge.attachPush(versHorloge, &boutonconfigvershorloge);
  boutonwifiversconfig.attachPush(versConfig, &boutonwifiversconfig);
  boutontelversconfig.attachPush(versConfig, &boutontelversconfig);
  boutoninfoversconfig.attachPush(versConfig, &boutoninfoversconfig);
  boutonbrassageversconfig.attachPush(versConfig, &boutonbrassageversconfig);
  boutonenregistrerbrassageversconfig.attachPop(enregistrerBrassage, &boutonenregistrerbrassageversconfig);
  boutonconfigversbrassage.attachPush(versBrassage, &boutonconfigversbrassage);
  boutonenregistrertelversconfig.attachPop(enregistrerTel, &boutonenregistrertelversconfig);
  boutonenregistrerwifiversconfig.attachPop(enregistrerWifi, &boutonenregistrerwifiversconfig);
  boutonenregistrerconfigversmenu.attachPop(enregistrerConfig, &boutonenregistrerconfigversmenu);
  boutonhorlogeversconfig.attachPush(versConfig, &boutonhorlogeversconfig);
  boutonenregistrerhorlogeversconfig.attachPop(enregistrerHorloge, &boutonenregistrerhorlogeversconfig);
  boutonomoinsoscillo1min.attachPop(boutonoscillo1min, &boutonomoinsoscillo1min);
  boutonplusoscillo1min.attachPop(boutonoscillo1min, &boutonplusoscillo1min);
  boutonomoinsoscillo1max.attachPop(boutonoscillo1max, &boutonomoinsoscillo1max);
  boutonplusoscillo1max.attachPop(boutonoscillo1max, &boutonplusoscillo1max);
  boutonomoinsoscillo2min.attachPop(boutonoscillo2min, &boutonomoinsoscillo2min);
  boutonplusoscillo2min.attachPop(boutonoscillo2min, &boutonplusoscillo2min);
  boutonomoinsoscillo2max.attachPop(boutonoscillo2max, &boutonomoinsoscillo2max);
  boutonplusoscillo2max.attachPop(boutonoscillo2max, &boutonplusoscillo2max);
  boutonomoinsoscillo3min.attachPop(boutonoscillo3min, &boutonomoinsoscillo3min);
  boutonplusoscillo3min.attachPop(boutonoscillo3min, &boutonplusoscillo3min);
  boutonomoinsoscillo3max.attachPop(boutonoscillo3max, &boutonomoinsoscillo3max);
  boutonplusoscillo3max.attachPop(boutonoscillo3max, &boutonplusoscillo3max);
  pagestandby.attachPush(versMenu, &pagestandby);
  boutonversstandby.attachPush(versStandby, &boutonversstandby);

  // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< MAJ des valeurs avant d'afficher la page menu sur l'ecran
  horaire(); // lance la fonction
  temperature(); // lance la fonction
  eclairage(); // lance la fonction
  brassage(); // lance la fonction
  definiTempeteAleatoire(); // lance la fonction
  etalonnageOnOFF = true; // met en ON
  delay(deuxMille);

  // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Affichage de le page menu
  affichePageMenu();// lance la fonction

  // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ecrit sur la carte sd la date et heure de demarage
  horodatageEtEcritSurSD (texteSurSD1, "");

  // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Active le watchdog de l'arduino
  /* Si plus de remise a zero du compteur "reboot" (fait dans le loop) au bout de 8 secondes = bug de l'arduino = reboot automatique ! */
  wdt_enable(WDTO_8S);
  
} // fin du setup

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ LOOP $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
void loop() {
  lectureBoutons(); /* lis les bontons du nextion >>> !!! IMPORTANT !!! >>> loop minimum 450/seconde pour ne pas avoir de bouton sans reponse <<< !!! IMPORTANT !!! <<< */
  ecouterModuleWifi(); /* ecoute les demande du module wifi */
  horaire(); /* recupere h'aure de l'horloge */
  rebootAlarme(); /* reboot les alarme toutes les heures durant la periode d'alerte determiné */
  tempeteAleatoireAutomatique(); /* declanche la tempete aleatoire si activé */
  nourrissage(); /* declanche le nourissage si activé */
  loopDesFonctions(); // lance des fonctions secondaire a tour de role
  wdt_reset(); // indique que le loop est OK, pas de bug, remise a zero du compteur "reboot" du watchdog
}
