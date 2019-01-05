
/* AQUABOUNS REEF, Automate pour la gestion d'aquarium recifal
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
#include "DallasTemperature.h"
#include <OneWire.h>

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ Fichiers $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
#include "temperature_ph.h"
#include "pin.h"
#include "global.h"
#include "debug.h"
#include "affichage.h"
#include "autres.h"
#include "gsm.h"
#include "wifi.h"
#include "carteSD.h"

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ Déclarations $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
// temperature
OneWire oneWire(pinInSondeDs18b20);
DallasTemperature sensors(&oneWire);
DeviceAddress sondeBac, sondeRampe; // adresse des sondes
const float temperatureDeFlotement = 0.3;// fourchette de temperature haute et basse ou il ne se passe rien // evite les ON/OFF lorsque la mesure est a cheval avec la valeur de declenchement
boolean etatVentilationBac = false, etatVentilationRampe = false; // etat de la ventilation pour afficher l'icone adapté
const uint8_t limiteTemperatureMinimum = 10; // temperature minimum, on en deduit que la sonde n'est pas presente // plus d'alerte en dessous de cette temperature
const uint8_t limiteTemperatureMaximum = 85; // temperature maximum, on en deduit que la sonde n'est pas presente // plus d'alerte en dessous de cette temperature

//ph
float moyennePhBac, moyennePhRac; // pour les mesure ph moyenne
boolean alertePhHaut = true, alertePhBas = true; // on off alerte ph
boolean switchPhRacBac = true; // mesure le ph bac / rac a tour de role
const uint8_t phDeFlotement = 5; // fourchette ph haut et bas ou il ne se passe rien // evite les ON/OFF lorsque la mesure est a cheval avec la valeur de declenchement

//const uint16_t etalonnagePhBas = 40100; // solution etalon ph 4.01
//const uint32_t etalonnagePhHaut = 100100; // solution etalon ph 10.01
//const uint16_t valeurAnalogiquePh401 = 765; //  ph 4.01 = 827 dans l'entre analogique sur une echelle de 1023 /// 925
//const uint16_t valeurAnalogiquePh1001 = 500; // ph 10.01 = 610 dans l'entre analogique sur une echelle de 1023 /// 666

// message d'alerte
const char alerte[] PROGMEM = "!!! ALERTE !!!";
const char finAlerte[] PROGMEM = "Fin d'alerte,";
const char texteSurSD32[] PROGMEM = "ventilation bac ON";
const char texteSurSD33[] PROGMEM = "ventilation bac OFF";
const char texteSurSD34[] PROGMEM = "ventilo rampe ON";
const char texteSurSD35[] PROGMEM = "ventilo rampe OFF";
const uint8_t compteurErreurMax = 5; // nombre max d'erreur de mesure
uint8_t compteurErreurTempBac = 0, compteurErreurTempRampe = 0, compteurErreurPhBac = 0, compteurErreurPhRac = 0;


//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ Fonctions $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
/* recuperation temperature des ds18b20 */
void temperature() {
  float stockMesure;
  stockMesure = sensors.getTempCByIndex(sauvegarde.adresseSondeRampe); // affecte la temperature recu a la variable
  DPRINTF("temp rampe : "); DPRINT(stockMesure); DPRINTLN();
  if (stockMesure <= limiteTemperatureMinimum || stockMesure >= limiteTemperatureMaximum) { // si relevé hors limite
    compteurErreurTempRampe++; // on incremente le compteur
    if (compteurErreurTempRampe >= compteurErreurMax) { // si le compteur est egal ou supperieur au nombre max d'erreur
      variable.temperatureRampe = zero; // = zero car trop de mesure erroné
    }
  }
  else {
    variable.temperatureRampe = stockMesure; // affecte la mesure
    compteurErreurTempRampe = zero; // met compteur d'erreur a zero
  }
  stockMesure = sensors.getTempCByIndex(sauvegarde.adresseSondeBac); // affecte la mesure
  DPRINTF("temp bac : "); DPRINT(stockMesure); DPRINTLN(); // debug
  if (stockMesure <= limiteTemperatureMinimum || stockMesure >= limiteTemperatureMaximum) { // si la mesure est hors limite
    compteurErreurTempBac++; // incremente le compteur d'erreur
    if (compteurErreurTempBac >= compteurErreurMax) { // si nombre d'erreur MAX atteind
      variable.temperatureBac = zero; // = zero car trop de mesure erroné
    }
  }
  else {
    variable.temperatureBac = stockMesure; // affecte la mesure
    compteurErreurTempBac = zero; // met compteur d'erreur a zero
  }
  ventilation(); // lance la fonction
  sensors.requestTemperatures(); // on lance une demande aux sondes pour prochain relevé de temperature
}

/* declanchement des ventilateurs si besoin */
void ventilation() {
  // ventilation bac
  if (variable.temperatureBac >= sauvegarde.ventilationaquarium) { // si temperaure est superieur a la consigne
    digitalWrite(pinOutRelaisVentilateurBac, HIGH); // relais fermé = ventilation ON
    digitalWrite(pinOutRelaisChauffage, HIGH); // relais fermé = chauffage OFF // par securité car doit deja etre OFF
    if (etatVentilationBac == false) {
      etatVentilationBac = !etatVentilationBac; // icone ventilation
      DPRINTF("  ON ventilo bac : "); DPRINT(variable.temperatureBac); DPRINTLN();
      horodatageEtEcritSurSD (texteSurSD32, "");
    }
  }
  else if (((float)variable.temperatureBac + temperatureDeFlotement) >= sauvegarde.ventilationaquarium) { // temperature de flotement ou on ne fait rien
    DPRINTF("flottement ventilo bac : "); DPRINT(variable.temperatureBac); DPRINTLN();
  }
  else { // si temperature inferieur a la consigne
    digitalWrite(pinOutRelaisVentilateurBac, LOW); // relais ouvert = ventilateur OFF
    digitalWrite(pinOutRelaisChauffage, LOW); // relais ouvert = chauffage ON (temperature géré par thermostat independant de l'arduino)
    if (etatVentilationBac) {
      horodatageEtEcritSurSD (texteSurSD33, "");
      etatVentilationBac = !etatVentilationBac; // icone ventilation
      DPRINTF("OFF ventilo bac : "); DPRINT(variable.temperatureBac); DPRINTLN();
    }
  }
  // ventilation rampe
  if (variable.temperatureRampe >= sauvegarde.ventilationrampe) { // si temperature superieur a la consigne
    digitalWrite(pinOutRelaisVentilateurRampe, HIGH); // relais fermé = ventilation ON
    if (etatVentilationRampe == false) {
      horodatageEtEcritSurSD (texteSurSD34, "");
      etatVentilationRampe = !etatVentilationRampe; // icone ventilation
      DPRINTF("ON ventilo rampe : "); DPRINT(variable.temperatureRampe); DPRINTLN();
    }
  }
  else if (variable.temperatureRampe + temperatureDeFlotement >= (sauvegarde.ventilationrampe)) { // temperature de flotement ou on ne fait rien
    DPRINTF("flottement ventilo rampe : "); DPRINT(variable.temperatureRampe); DPRINTLN();
  }
  else { // si temperature inferieur a la consigne
    digitalWrite(pinOutRelaisVentilateurRampe, LOW); // relais ouvert = ventilateur OFF
    if (etatVentilationRampe) {
      horodatageEtEcritSurSD (texteSurSD35, "");
      etatVentilationRampe = false; // icone ventilation
      DPRINTF("OFF ventilo rampe : "); DPRINT(variable.temperatureRampe); DPRINTLN();
    }
  }
}

/* affiche sur l'ecran et envoie SMS si temperature hors consigne */
void messageAlerteTemperature() {
  if (variable.temperatureBac > limiteTemperatureMinimum) {
    if (variable.temperatureBac >= sauvegarde.alertetemperaturehaute) { // si la temperature du bac est egal ou depasse la temperature d'alerte
      if (alerteTemperature) { // pour ne pas avoir de message a repetition
        completerMessageAlerteTemperature(alerte); // prepare texte du SMS a envoyer
        alarme(); // lance la fonction
        DPRINTF("alarme temp haut "); DPRINTLN() // debug
      }
    }
    else if (variable.temperatureBac <= sauvegarde.alertetemperaturebasse) { // si la temperature du bac est egal ou depasse la temperature d'alerte
      if (alerteTemperature ) { // pour ne pas avoir de message a repetition
        completerMessageAlerteTemperature(alerte); // prepare texte du SMS a envoyer
        alarme(); // lance la fonction
        DPRINTF("alarme temp bas "); DPRINTLN() // debug
      }
    }
    else if (((variable.temperatureBac + temperatureDeFlotement) <= sauvegarde.alertetemperaturehaute) && ((variable.temperatureBac - temperatureDeFlotement) >= sauvegarde.alertetemperaturebasse )) {
      if (alerteTemperature == false) { // pour ne pas avoir de message a repetition
        completerMessageAlerteTemperature(finAlerte); // prepare texte du SMS a envoyer
        alarmeOnOff = true; // remet l'alarme sonore On
        DPRINTF("alarme temp ok "); DPRINTLN() // debug
      }
    }
  }
}

/* recuperation ph, gere electrovanne co2 Rac et envoie les alertes */
void ph() {
  if (switchPhRacBac) { // boolean pour switch entre les deux sondes
    recupererPh(pinInPhBac, moyennePhBac); // lance la fonction
    DPRINTF("moyene ph Bac : "); DPRINT(variable.phBac);  DPRINTLN(); // debug
  }
  else if (switchPhRacBac == false) { // boolean pour switch entre les deux sondes
    recupererPh(pinInPhRac, moyennePhRac); // lance la fonction
    DPRINTF("moyene ph Rac : "); DPRINT(variable.phRac);  DPRINTLN(); // debug
  }
  switchPhRacBac = !switchPhRacBac; // boolean switch entre les deux sonde ph pour prendre les mesures a tour de role
}

void alertePhBac() { // alerte ph bac
  if (etalonnageOnOFF == false) { // si l'etalonage n'est pas en cours
    if (variable.phBac <= sauvegarde.alertephbacbas && variable.phBac != zero) { // alerte ph bas et si pas d'erreur de mesure
      if (sauvegarde.alertephbacbas != sauvegarde.alertephbachaut) { // quand les 2 alertes sont identique = alerte OFF
        digitalWrite(pinOutRelaisElectovanneRac, HIGH); // relais fermé = electrovanne On
        if (alertePhBas) { // pour ne pas avoir de message a repetition
          alertePhBas = !alertePhBas;
          afficherAlertepPhBacBas(); // lance la fonction
          completerMessageAlertePh(alerte); // prepare texte du SMS a envoyer
          alarme(); // lance la fonction
          DPRINTF("alarme ph bas "); DPRINTLN() // debug
        }
      }
      else {
        DPRINTF("alarme ph bac OFF "); DPRINTLN() // debug
      }
    }
    else if (variable.phBac >= sauvegarde.alertephbachaut && variable.phBac != zero) { // alerte ph haut et si pas d'erreur de mesure
      if (sauvegarde.alertephbacbas != sauvegarde.alertephbachaut) { // si l'alerte PH est activé (si alerte bas = haut, alerte desactivé)
        if (alertePhHaut) { // pour ne pas avoir de message a repetition
          alertePhHaut = !alertePhHaut;
          afficherAlertepPhBacHaut();// lance la fonction
          completerMessageAlertePh(alerte); // prepare texte du SMS a envoyer
          alarme();// lance la fonction
          DPRINTF("alarme ph haut "); DPRINTLN() // debug
        }
      }
      else {
        DPRINTF("alarme ph bac OFF "); DPRINTLN() // debug
      }
    }
    else { // si pas de probleme PH dans le bac, gestion CO2 du RAC
      rac();
      if (alertePhBas == false || alertePhHaut == false) {
        afficherAlertepPhBacOff(); // fin / pas d'alerte
        completerMessageAlertePh(finAlerte); // prepare texte du SMS a envoyer
        alarmeOnOff = true; // remet l'alarme sonore On
        DPRINTF("alarme ph ok "); DPRINTLN() // debug
      }
    }
  }
}

void rac() { // electrovanne RAC
  const uint16_t consigneMiniPhRac = 400; // 400 = Off
  if (sauvegarde.consignePhRac > consigneMiniPhRac) { // si pas en Off
    if (variable.phRac >= sauvegarde.consignePhRac + phDeFlotement) { // si mesure superieur ou egale a la consigne + flotement
      digitalWrite(pinOutRelaisElectovanneRac, HIGH); // relais fermé = electrovanne On
    }
    else if (variable.phRac > sauvegarde.consignePhRac) { // ph de flotement, on ne fait rien
    }
    else { // si mesure inferieur ou egal a la consigne
      digitalWrite(pinOutRelaisElectovanneRac, LOW); // relais ouvert = electrovanne OFF
    }
  }
  else {
    DPRINTF("RAC OFF"); DPRINTLN() // debug
  }
}

/* etablie la moyenne des mesures ph */
void recupererPh(const uint8_t pin, float moyennePrecedente) {
  float mesurePh = 0; // stock calcul du ph
  uint8_t nbr2loop = 4; // divise le nombre de mesure pour que les valeur max et man soit déduite a X reprises
  const uint8_t nombreDeMesure = 20; // !!! forcement un multiple de nbr2loop !!! nombre de prise de mesure pour etablir la moyene
  const uint16_t mesurePhMin = 400u; //  = ph 4.00 limite mesure ph
  const uint16_t mesurePhMax = 1000u; // = ph 10.00 limite mesure ph
  uint16_t min, max; // stock valeur min et max
  uint16_t stockMesure[nombreDeMesure]; // stock les mesure ph
  const float AREF = 4.096; // 4.096V appliquer sur le pin AREF de l'arduino
  const uint16_t echelleAnalogique = 1024u; // echelle analogique de 0 a 1023 soit 1024 possibilité
  const float ajustementV = 0.683; // ajuste le voltage calculer pour ne pas etre en fin de course du potentiometre d'etalonage de la carte ph
  const float ajustementEchellePH = 0.173; // 0.173 a remplacer par 0,176595052 pour avoir la meme courbe que les mesure faite durant les test
  const uint8_t pointZeroPH = 7; // la sonde PH emet 0V a un PH 7
  const float pointZeroV = 2.5; // la carte PH emet 2.5V a un PH 7

  for (uint8_t i = zero; i < nbr2loop; i++) { // boucle le nombre de loop
    for (uint8_t i2 = zero; i2 < (nombreDeMesure / nbr2loop); i2++) { // boucle le nombre de mesure divisé par le nombre de boucle
      stockMesure[i2] = analogRead(pin); // prend la mesure
      delay(dix); // un peu d'attente entre chaque mesure
    }
    if (stockMesure[zero] < stockMesure[un]) { // si mesure 0 < mesure 1
      min = stockMesure[zero]; max = stockMesure[un]; // mesure 0 = mini et mesure 1 egal maxi
    }
    else { // sinon mesure 0 > mesure 1
      min = stockMesure[un]; max = stockMesure[zero]; // mesure 1 = mini et mesure 0 egal maxi
    }
    for (uint8_t i3 = deux; i3 < (nombreDeMesure / nbr2loop); i3++) { // on commence a la mesure 2
      if (stockMesure[i3] < min) { //si la mesure dans l'index est inferieur au mini
        mesurePh += min;      // on cumul la mini a la somme
        min = stockMesure[i3]; // on affecte a mini la nouvelle valeur mini
      }
      else {
        if (stockMesure[i3] > max) { //si la mesure dans l'index est suprieur au maxi
          mesurePh += max;  // on cumul la mini a la somme
          max = stockMesure[i3]; // on affecte a maxi la nouvelle valeur maxi
        }
        else { // sinon
          mesurePh += stockMesure[i3]; // on cumul la mini a la somme
        }
      }
      DPRINTF("Ph mesuré - min et max: "); DPRINT(stockMesure[i3]); DPRINTLN() // debug
    }
  }
  DPRINTF("Ph mesuré cumul : "); DPRINT(mesurePh); DPRINTLN(); // debug
  mesurePh = mesurePh / (nombreDeMesure - (deux * nbr2loop) ); // on divise la somme par le nombre de mesure -  (MIN et MAX)* nombre de loop
  DPRINTF("Ph mesuré moyenne : "); DPRINT(mesurePh); DPRINTLN(); // debug

  // if (mesurePh < valeurAnalogiquePh401 + (valeurAnalogiquePh401 / marge) && mesurePh > valeurAnalogiquePh1001 - (valeurAnalogiquePh1001 / marge)) {
  // mesurePh = map(mesurePh, valeurAnalogiquePh401, valeurAnalogiquePh1001, etalonnagePhBas, etalonnagePhHaut);
  // mesurePh = mesurePh / cent;

  float voltage = (float)((mesurePh / echelleAnalogique) * AREF) - ajustementV; // calcule le Voltage sur le PIN en entree
  DPRINTF("voltage : "); DPRINT(voltage); DPRINTLN(); // debug
  mesurePh = (pointZeroPH + ((pointZeroV - voltage) / ajustementEchellePH)) * cent; // calcule le PH par rapport au V mesuré
  DPRINTF("mesurePh : "); DPRINT(mesurePh); DPRINTLN(); // debug

  if (mesurePh > (mesurePhMin - (mesurePhMin / dix)) && mesurePh < (mesurePhMax + (mesurePhMax / dix))) { // si la mesure est dans l'echelle
    if (moyennePrecedente != zero) {
      mesurePh = ((moyennePrecedente * ((nombreDeMesure) - un)) + mesurePh) / (nombreDeMesure); // etablie la moyenne des mesures
    }
    if (pin == pinInPhBac) { // si PIN PH BAC en cours de mesure
      variable.phBac = mesurePh; // affecte la mesure
      moyennePhBac = mesurePh; // affecte la mesure
      compteurErreurPhBac = zero; // met le compteur d'erreur a zero
    }
    else if (pin == pinInPhRac) { // si PIN PH RAC en cours de mesure
      variable.phRac = mesurePh; // affecte la mesure
      moyennePhRac = mesurePh; // affecte la mesure
      compteurErreurPhRac = zero; // met le compteur d'erreur a zero
    }
  }
  else { // si mesure hors echelle
    if (pin == pinInPhBac) { // si PIN PH BAC en cours de mesure
      compteurErreurPhBac++; // incremente le compteur
      if (compteurErreurPhBac > compteurErreurMax) { // si nombre d'erreur max atteint
        variable.phBac = zero; // mesure = zero
      }
    }
    else if (pin == pinInPhRac) { // si PIN PH RAC en cours de mesure
      compteurErreurPhRac++; // incremente le compteur
      if (compteurErreurPhRac > compteurErreurMax) { // si nombre d'erreur max atteint
        variable.phRac = zero; // mesure = zero
      }
    }
  }
}
