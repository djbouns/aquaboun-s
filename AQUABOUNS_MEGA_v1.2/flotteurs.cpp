
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


//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ Fichiers $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
#include "flotteurs.h"
#include "pin.h"
#include "global.h"
#include "debug.h"
#include "affichage.h"
#include "gsm.h"
#include "autres.h"
#include "wifi.h"
#include "carteSD.h"

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ Déclarations $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
boolean osmolationOn = true;
uint8_t compteurOsmolation = 0;

uint32_t tempsPrecedentOsmolation;
uint32_t tempsAttenteOsmolation;
uint32_t compteurPinInFloteurReserveBas;
uint32_t compteurPinInFloteurReserveHaut;
boolean smsAlerteOsmolation = true;
boolean smsAlerteGodet = true;
boolean smsAlerteSecurite = true;
boolean smsAlerteReserve = true;
// message d'alerte
const char smsAlerteSecuriteDecante[] PROGMEM = " Niveau securite decante, ecumeur + osmolateur OFF";
const char smsSecuriteDecanteOk[] PROGMEM = " Niveau securite decante OK";
const char smsReserveVide[] PROGMEM = " Reserve vide";
const char smsTropOsmolation[] PROGMEM = " Trop de declenchement de l'osmolateur, osmolateur OFF";
const char smsOsmolationOk[] PROGMEM = " Osmolation ON";
const char smsGodetPlein[] PROGMEM = " Godet plein, ecumeur OFF";
const char texteSurSD13[] PROGMEM = "osmolation compteur MAX";
const char texteSurSD14[] PROGMEM = "osmolation compteur +1";
const char texteSurSD15[] PROGMEM = "niveau constant OK";

/* lit l'etat du flotteurs securite decante*/
void flotteurs() {
  if (digitalRead (pinInFloteurSecurite) == HIGH) { // floteur securite HAUT = tout couper
    digitalWrite(pinOutRelaisOsmolation, LOW); // relais OFF = osmolateur OFF
    digitalWrite(pinOutRelaisEcumeur, HIGH);// relais ecumeur ON = ecumeur OFF
    if (smsAlerteSecurite ) { // si boolean alerte true envoie alerte + alarme
      completerMessageAlerte(alerte, smsAlerteSecuriteDecante); // lance la fonction
      smsAlerteSecurite = !smsAlerteSecurite; // boolean pour ne pas avoir de message a repetition
      alarme(); // lance la fonction
      DPRINTF("alarme flotteur securite haut "); DPRINTLN() // debug
    }
  }
  else {// floteur securite BAS = voir autres flotteurs
    if (smsAlerteSecurite == false) { // si fin d'alerte
      completerMessageAlerte(finAlerte, smsSecuriteDecanteOk); // lance la fonction
      smsAlerteSecurite = !smsAlerteSecurite; // boolean pour ne pas avoir de message a repetition
      alarmeOnOff = true; // remet l'alarme sonore On
      DPRINTF("flotteur securite bas "); DPRINTLN() // debug
    }
    // DPRINTF("securite decante OK");  DPRINTLN(); // debug
    flotteurReserve(); // lance la fonction qui lit l'etat d'autres flotteurs
  }
}

/* lit l'etat du flotteur niveau reserve d'eau osmosé */
void flotteurReserve() {
  if (digitalRead (pinInFloteurReserve) == HIGH) { // reserve d'eau osmose OK
    const uint32_t delayFlottement = 900000ul; // 15 minutes en millis pour ne pas avoir d'alerte/redemarage a repetition lors du remplissage de la reserve
    compteurPinInFloteurReserveHaut = millis();

    if ( millis() - compteurPinInFloteurReserveBas > delayFlottement) {
      flotteurNiveauConstant(); // lance la fonction qui lit l'etat d'autres flotteurs
      flotteurNiveauEcumeur(); // lance la fonction qui lit l'etat d'autres flotteurs
      if (smsAlerteReserve == false) { // boolean pour ne pas avoir de message a repetition
        smsAlerteReserve = true; // remet SMS en On
        alarmeOnOff = true; // remet l'alarme sonore On
        compteurOsmolation = zero ; // remet le compteur d'osmolation a zero
        //DPRINTF("reserve OK");  DPRINTLN(); // debug
      }
    }
  }
  else { // si reserve VIDE
    const uint16_t delayFlottement = 5000u; // 5 secondes en millis pour ne pas avoir d'alerte/redemarage a repetition lors de l'arret de la pompe
    if ( millis() - compteurPinInFloteurReserveHaut > delayFlottement ) {
      digitalWrite(pinOutRelaisOsmolation, LOW); // relais osmolateur OFF
      compteurPinInFloteurReserveBas = millis();
      // DPRINTF(" !!! reserve d'eau osmose vide");  DPRINTLN(); // debug
      if ( smsAlerteReserve ) { // si boolean true
        smsAlerteReserve = ! smsAlerteReserve; // boolean pour ne pas avoir de message a repetition
        completerMessageAlerte(alerte, smsReserveVide); // lance la fonction
        afficherAlerteOsmolationOffReserveVide(); // message erreur plus d'eau
        alarme(); // lance la fonction
        DPRINTF("alarme reserve bas "); DPRINTLN() // debug
      }
    }
  }
}

/* lit l'etat du flotteur niveau constant de la decante */
void flotteurNiveauConstant() {
  if (sauvegarde.dureeOsmolationMillis != zero) { // si la durée d'osmolation n'est pas OFF
    if (digitalRead (pinInFloteurOsmolation) == LOW) { // si floteur en position BAS
      if (osmolationOn ) { // si boolean true
        tempsPrecedentOsmolation = millis(); // activation de la duree d'osmolation
        osmolationOn = !osmolationOn; // boolean pour ne pas avoir de remise a zero du compteur a repetition
      }
      if (compteurOsmolation < sauvegarde.compteurOsmolationMax) { //si compteur MAX pas atteind
        if (millis() - tempsPrecedentOsmolation < sauvegarde.dureeOsmolationMillis) { // osmolation si duree d'osmolation non depassé
          digitalWrite(pinOutRelaisOsmolation, HIGH); // relais osmolateur ON
          tempsAttenteOsmolation = millis(); // activation de la duree d'attente apres osmolation
          DPRINTF("osmolation en cours");  DPRINTLN(); // debug
        }
        else if (millis() - tempsAttenteOsmolation <= (sauvegarde.dureeOsmolationMillis * deux)) { // pause de 2 fois duree osmolation si duree osmolation depassee
          digitalWrite(pinOutRelaisOsmolation, LOW); // relais osmolateur OFF
          DPRINTF("osmolation en pause");  DPRINTLN(); // debug
        }
        else {
          digitalWrite(pinOutRelaisOsmolation, LOW);
          compteurOsmolation++; // +1 au compteur d'osmolation
          osmolationOn = !osmolationOn; // boolean pour avoir la remise a zero du compteur
          horodatageEtEcritSurSD (texteSurSD14, "");
          DPRINTF("+1 compte tour osmolation");  DPRINTLN(); // debug
        }
      }
      else { // quand compteur MAX atteind
        digitalWrite(pinOutRelaisOsmolation, LOW); // on coupe l'osmolation
        if (smsAlerteOsmolation ) { // si true
          smsAlerteOsmolation = !smsAlerteOsmolation; // boolean pour ne pas avoir de message a repetition
          afficherAlerteOsmolation(); // affiche le message d'erreur a l'ecran
          completerMessageAlerte(alerte, smsTropOsmolation); // lance la fonction
          alarme(); // lance la fonction
          horodatageEtEcritSurSD (texteSurSD13, "");
          DPRINTF("alarme trop osmolation "); DPRINTLN() // debug
        }
      }
    }
    else { // quand floteur osmolation en position haute
      digitalWrite(pinOutRelaisOsmolation, LOW); // relais osmolateur OFF
      if (osmolationOn == false) { // si false
        afficherAlerteOsmolationOff(); // efface le message d'alerte a l'ecran
        osmolationOn = true; // boolean pour remetre l'osmolation ON
        compteurOsmolation = zero ; // remet le compteur d'osmolation a zero

      }
      if ( smsAlerteOsmolation == false) { // si false
        completerMessageAlerte(finAlerte, smsOsmolationOk ); // envoie un SMS de fin d'alerte
        smsAlerteOsmolation = true; // boolean pour ne pas avoir de message a repetition
        alarmeOnOff = true; // remet l'alarme sonore On
        horodatageEtEcritSurSD (texteSurSD15, "");
        DPRINTF("niveau constant OK"); DPRINTF("/// "); DPRINT(variable.Heure); DPRINTF("h"); DPRINT(variable.minut); DPRINTLN(); // debug
      }
    }
  }
}

/* lit l'etat du flotteur niveau ecumeur + godet ecumeur */
void flotteurNiveauEcumeur() { //
  if ((digitalRead (pinInFloteurNiveauEcumeur) == LOW) && (digitalRead (pinInFloteurGodetEcumeur) == LOW)) { // flotteur niveau ecumeur + godet en bas
    digitalWrite(pinOutRelaisEcumeur, LOW); // relais ouvert = ecumeur ON
    if (smsAlerteGodet == false) { // fin d'alerte godet plein
      smsAlerteGodet = !smsAlerteGodet;  // boolean pour ne pas avoir de message a repetition
      alarmeOnOff = true; // remet l'alarme sonore On
      DPRINTF("godet ecumeur OK");  DPRINTLN(); // debug
    }
  }
  else {
    if (digitalRead (pinInFloteurGodetEcumeur) == HIGH) {// flotteur godet ecumeur haut
      digitalWrite(pinOutRelaisEcumeur, HIGH); // relais fermer = cumeur OFF
      DPRINTF("godet plein, ecumeur Off");  DPRINTLN(); // debug

      if (smsAlerteGodet ) { // alerte godet plein
        completerMessageAlerte(alerte, smsGodetPlein);
        alarme(); // lance la fonction
        smsAlerteGodet = !smsAlerteGodet; // boolean pour ne pas avoir de message a repetition
        DPRINTF("SMS godet plein, ecumeur Off");  DPRINTLN(); // debug
      }
    }
    if (digitalRead (pinInFloteurNiveauEcumeur) == HIGH) {// flotteur godet ecumeur haut
      digitalWrite(pinOutRelaisEcumeur, HIGH); // relais fermer = cumeur OFF
      DPRINTF("flotteurs haut ecumeur Off");  DPRINTLN(); // debug
    }
  }
}
