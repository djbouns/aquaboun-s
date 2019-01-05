
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
#include "TimerFreeTone.h"

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ Fichiers $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
#include "autres.h"
#include "arduino.h"
#include "pin.h"
#include "global.h"
#include "affichage.h"
#include "debug.h"
#include "gsm.h"
#include "wifi.h"
#include "carteSD.h"

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ Déclarations $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
boolean texteNourrissage = true;
boolean alerteTemperature = true;
boolean alarmeOnOff = true;
uint8_t Hprecedent = 0;
boolean smsAlerteEdf = true;
const char smsCoupureEdf[] PROGMEM = " Coupure EDF";
const char smsEdfOk[] PROGMEM = " EDF OK";
const char texteSurSD2[] PROGMEM = "coupure batterie vide";
const char texteSurSD8[] PROGMEM = "arret pompe remonté nourissage";
const char texteSurSD9[] PROGMEM = "declenchement distributeur nouriture";
const char texteSurSD10[] PROGMEM = "pompe remonté ON";
const char texteSurSD11[] PROGMEM = "fin du nourissage";
const char texteSurSD12[] PROGMEM = "signal sonore";

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ Fonctions $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
/* nourrissage  automatique */
void nourrissage() {
  for (uint8_t numeroNourrissage = numeroUn; numeroNourrissage <= numeroDeux; numeroNourrissage++) { // lance la fonction pour chaque nourrissage
    if (sauvegarde.nourrissageON[numeroNourrissage] ) { // si ON
      if (Time == sauvegarde.heureNourrissage[numeroNourrissage]) { // si heure de nourrissage ateint
        if (sauvegarde.remonteDelay == Delay) { // si en mode delay
          digitalWrite(pinOutRelaisRemontee, HIGH); // remonté off pendant alimentation
          DPRINTF("pompe de remonté OFF");  DPRINTLN(); // debug
          horodatageEtEcritSurSD (texteSurSD8,"");
        }
        tempsPrecedentNourrissage = millis(); // lance le compteur
        nourissageEnCours = true; // nourrissage ON
        afficherNourrissageEnCours(); // affiche le message a l'ecran
        digitalWrite(pinOutRelaisDistributeurNouriture, HIGH); // declanche le distributeur
        delay (mille); // pause
        digitalWrite(pinOutRelaisDistributeurNouriture, LOW); // arrete le distributeur
         horodatageEtEcritSurSD (texteSurSD9,"");
        DPRINTF("distribution n°")DPRINT(numeroNourrissage); DPRINTF(" en cours");  DPRINTLN(); // debug
      }
    }
  }
  if (texteNourrissage ) { // Pour MAJ apres distribution
    if (millis() - tempsPrecedentNourrissage > sauvegarde.dureeNourrissageMillis) { // fin distribution
      afficherNourrissageEnCoursOff(); // affiche le message
      if (sauvegarde.remonteDelay == Delay) { // remet en ON la remonte apres alimentation si en mode delay
        digitalWrite(pinOutRelaisRemontee, LOW); // relais ouvert = remonté ON
         horodatageEtEcritSurSD (texteSurSD10,"");
        DPRINTF("pompe remonté ON");  DPRINTLN(); // debug
      }
      DPRINTF("fin Nourrissage");  DPRINTLN(); // debug
      texteNourrissage = false; // boolean pour ne pas avoir de MAJ a repetition
       horodatageEtEcritSurSD (texteSurSD11,"");
    }
  } // la fonction distribution manuel est avec le bouton dans l'affichage
}

/* alerte en cas de coupure EDF */
void coupureEdf() {
  const int16_t Val12VMin = 500; // quand 12V OK = ~573 sur l'echele de 1024 avec pont diviseur on prend une valeur inferieur pour laisser de la marge
  if (analogRead(pinInCoupureCourant) < Val12VMin) { // si coupure EDF
    if (smsAlerteEdf ) { // si premiere alerte
      completerMessageAlerte(alerte, smsCoupureEdf); // envoie un SMS
      smsAlerteEdf = !smsAlerteEdf; // boolean pour ne pas avoir de SMS a repetition
      alarme();
      DPRINTF("alarme coupure edf "); DPRINTLN() // debug
    }
  }
  else {
    if (smsAlerteEdf == false) { // Si juste apres coupure EDF
      completerMessageAlerte(finAlerte, smsEdfOk); // envoie un SMS
      smsAlerteEdf = !smsAlerteEdf; // boolean pour ne pas avoir de SMS a repetition²
      delay(deuxMille); // attente de 2 seconde le temps que l'ecran se rallume
      affichePageMenu(); // on affiche la page menu sur l'ecran
      DPRINTF("alarme edf ok "); DPRINTLN() // debug
    }
  }
}

void coupureBatterie() { // coupe la batterie afin d'eviter le dechargement total et le scitillement de l'ecran lorsque la tension devient trop faible
  mesureNiveauBatterie = analogRead(pinInBatterie); // prend la mesure
  if (mesureNiveauBatterie < batterie0) { // si mesure est inferieur a la tension minimum necessaire.
    horodatageEtEcritSurSD (texteSurSD2,"");
    digitalWrite(pinOutBatterie , LOW); //  coupe la batterie
  }
  else {
    digitalWrite(pinOutBatterie , HIGH); // batterie active
    //DPRINTF("Batterie On "); DPRINTLN() // debug
  }
}

/* relance les alerte toute les heure entre l'intervale configurer */
void rebootAlarme() {
  if (((Time >= sauvegarde.heureDebutAlerte) && (M == 0) && (variable.seconde == 0)) && ((Time <= sauvegarde.heureFinAlerte) && (M == 0) && (variable.seconde == 0))) { // reboot toute les heures
    if (alerteTemperature == false) {
      alerteTemperature = true;
      DPRINTF("reboot alarme temperature");  DPRINTLN(); // debug
    }
    if (smsAlerteOsmolation == false) {
      smsAlerteOsmolation = true;
      DPRINTF("reboot alarme osmolateur");  DPRINTLN(); // debug
    }
    if (alertePhBas == false) {
      alertePhBas = true;
      DPRINTF("reboot alarme ph bas");  DPRINTLN(); // debug
    }
    if (alertePhHaut == false) {
      alertePhHaut = true;
      DPRINTF("reboot alarme ph haut");  DPRINTLN(); // debug
    }
    if (smsAlerteGodet == false) {
      smsAlerteGodet = true;
      DPRINTF("reboot alarme godet ecumeur");  DPRINTLN(); // debug
    }
    if (smsAlerteEdf == false) {
      smsAlerteEdf = true;
      DPRINTF("reboot alarme EDF");  DPRINTLN(); // debug
    }
    if (smsAlerteSecurite == false) {
      smsAlerteSecurite = true;
      DPRINTF("reboot alarme securité");  DPRINTLN(); // debug
    }
    if (smsAlerteReserve == false) {
      smsAlerteReserve = true;
      DPRINTF("reboot alarme reserve");  DPRINTLN(); // debug
    }
    if (alarmeOnOff == false) {
      alarmeOnOff = true;
      DPRINTF("reboot alarme sonore");  DPRINTLN(); // debug
    }
    delay(mille); // pour ne pas avoir de repetition
  }
}

/* alarme sonnore */
void alarme() {
  if ((Time >= sauvegarde.heureDebutAlerte) && (Time <= sauvegarde.heureFinAlerte)) {
    uint8_t i = 0;
    uint8_t i2 = 0;
    uint16_t note = 500u;
    const uint8_t repetition = 3;
    if (alarmeOnOff ) {
      DPRINTF("debut BIP sonnore");  DPRINTLN(); // debug
      horodatageEtEcritSurSD (texteSurSD12,"");
      while (i2 < repetition) { // joue 3 fois la "melodie"
        while (i < 200) {
          TimerFreeTone(pinOutbuzzer, note, 1); // envoie les notes
          note = note + 40; // change la note
          i++;
        }
        delay (500) ; // pause
        while (i > 1) {
          if (i2 < 2) {
            TimerFreeTone(pinOutbuzzer, note, 1); // envoie les notes
            note = note - 40; // change la note
          }
          i--;
        }
        i2++;
      }
      DPRINTF("fin BIP sonore");  DPRINTLN(); // debug
      alarmeOnOff = !alarmeOnOff;
    }
    else {
      DPRINTF("BIP sonore OFF");  DPRINTLN(); // debug
    }
  }
}

void reboot(const uint8_t pinHardReset) { // pour faire hard reboot
  digitalWrite(pinHardReset, LOW); // met pin en OFF
  delay (100); // attente
  digitalWrite(pinHardReset, HIGH); // met pin en On
}
