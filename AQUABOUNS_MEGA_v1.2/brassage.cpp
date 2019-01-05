
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
#include <arduino.h>

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ Fichiers $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
#include "brassage.h"
#include "pin.h"
#include "global.h"
#include "debug.h"
#include "affichage.h"
#include "wifi.h"

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ Déclarations $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
boolean tempeteManuelEnCours = false;
boolean tempeteAleatoireOn = false;
boolean tempeteAleatoireEnCours = false;
boolean MajEcranTempeteEnCours = false;
boolean MajEcranNourrissageEnCours = false;
boolean nourissageEnCours = false;
boolean definiTempeteDuLendemain = true;
uint8_t puissanceMinBrassage[trois];
uint32_t heureTempeteAleatoire;
uint32_t dureeTempeteAleatoire;
float pourCentEnPwm = 2.55;

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ Fonctions $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
/* defini la puissance du brassage */
void brassage() {
  for (uint8_t numeroBrassage = numeroUn; numeroBrassage <= numeroTrois ; numeroBrassage++) { // lance la fonction pour chaque pompe de brassage
    /* si nourissage en cours */
    if (nourissageEnCours ) {
      if (millis() - tempsPrecedentNourrissage <= sauvegarde.dureeNourrissageMillis) { // si duree du nourisssage non ateinte
        if (sauvegarde.brassageOnOffPwm[numeroBrassage] != Arret) { // si pas en OFF baisse du brassage au MIN
          variable.pwmBrassage[numeroBrassage] = puissanceMinBrassage[numeroBrassage];
          analogWrite(pinOutBrassage[numeroBrassage], variable.pwmBrassage[numeroBrassage] * pourCentEnPwm);
          // DPRINTF("brassage minimum car Nourrissage"); DPRINTLN();
        }
        else { // si en OFF
          variable.pwmBrassage[numeroBrassage] = zero;
          analogWrite(pinOutBrassage[numeroBrassage], zero);
          // DPRINTF("Nourrissage mais en OFF"); DPRINTLN();
        }
        if (MajEcranTempeteEnCours ) { // si tempette en cours, tempette OFF
          MajEcranTempeteEnCours = !MajEcranTempeteEnCours;
          statutTempete(); // met a jour l'icone sur l'ecran
        }
        if (numeroBrassage == numeroTrois) {
          if (MajEcranNourrissageEnCours == false) { // met a jour les info a l'ecran
            MajEcranNourrissageEnCours = !MajEcranNourrissageEnCours; /// boolean pour ne rafraichir en boucle
            rafraichirBrassage1Nextion(); // rafraichi les info a l'ecran
            rafraichirBrassage2Nextion(); // rafraichi les info a l'ecran
            rafraichirBrassage3Nextion(); // rafraichi les info a l'ecran
            onOffalimentation = false; // met le bouton en false
          }
        }
        if (tempeteAleatoireEnCours ) { // si la tempette etait ON
          tempeteAleatoireEnCours = !tempeteAleatoireEnCours; // met tempete en OFF
        }
      }
      else { // nourissage fini
        nourissageEnCours = !nourissageEnCours;
        onOffalimentation = true; // met le bouton en true
      }
    }
    /* si tempete manuel en cours */
    else if (tempeteManuelEnCours ) { // tempete manuel ON
      if (millis() - tempsPrecedentTempete <= sauvegarde.dureeTempeteMillis) { // si tempete manuel
        //DPRINT(MajEcranTempeteEnCours);  DPRINTLN(); // debug
        if (sauvegarde.brassageOnOffPwm[numeroBrassage] != Arret) { // si pas en OFF
          variable.pwmBrassage[numeroBrassage] = sauvegarde.puissanceTempete; // puissance definie en tempete
          analogWrite(pinOutBrassage[numeroBrassage], variable.pwmBrassage[numeroBrassage] * pourCentEnPwm);
        }
        else { // si en OFF
          variable.pwmBrassage[numeroBrassage] = zero; // puissance zero
          analogWrite(pinOutBrassage[numeroBrassage], zero);
        }
        if (MajEcranTempeteEnCours == false) { // boolean pour ne pas mettre a jour les info a l'ecran en boucle
          MajEcranTempeteEnCours = !MajEcranTempeteEnCours;
          statutTempete(); // MAJ l'icone sur l'ecran
        }
        if (MajEcranNourrissageEnCours ) { // boolean pour ne pas mettre a jour les info a l'ecran en boucle
          MajEcranNourrissageEnCours = !MajEcranNourrissageEnCours;
        }
        if (tempeteAleatoireEnCours ) {// boolean pour ne pas mettre a jour les info a l'ecran en boucle
          tempeteAleatoireEnCours = !tempeteAleatoireEnCours;
        }
        //DPRINTF("brassage TEMPETE manuel "); DPRINT(MajEcranTempeteEnCours); DPRINTLN();
      }
      else { // fin de la tempette manuel
        tempeteManuelEnCours = !tempeteManuelEnCours; // boolean pour mettre a jour les info a l'ecran
      }
    }
    /* si tempete aleatoir en cours */
    else if (tempeteAleatoireEnCours ) {
      if (millis() - tempsPrecedentTempeteAleatoire <= dureeTempeteAleatoire) {
        variable.pwmBrassage[numeroBrassage] = sauvegarde.puissanceTempete;
        if (sauvegarde.brassageOnOffPwm[numeroBrassage] > Arret) {
          analogWrite(pinOutBrassage[numeroBrassage], variable.pwmBrassage[numeroBrassage] * pourCentEnPwm);
        }
        else {
          variable.pwmBrassage[numeroBrassage] = zero;
          analogWrite(pinOutBrassage[numeroBrassage], zero);
        }
        if (numeroBrassage == numeroTrois) {
          if (MajEcranTempeteEnCours == false) {
            MajEcranTempeteEnCours = !MajEcranTempeteEnCours;
            statutTempete();
            rafraichirBrassage1Nextion(); // rafraichi les info a l'ecran
            rafraichirBrassage2Nextion(); // rafraichi les info a l'ecran
            rafraichirBrassage3Nextion(); // rafraichi les info a l'ecran
          }
        }
        if (MajEcranNourrissageEnCours ) {
          MajEcranNourrissageEnCours = !MajEcranNourrissageEnCours;

        }
        //  DPRINTF("brassage TEMPETE aleatoire"); DPRINTLN(); //debug
      }
      else { // si tempete aleatoir fini
        tempeteAleatoireEnCours = false;
        rafraichirBrassage1Nextion(); // rafraichi les info a l'ecran
        rafraichirBrassage2Nextion(); // rafraichi les info a l'ecran
        rafraichirBrassage3Nextion(); // rafraichi les info a l'ecran
      }
    }
    /* sinon brassage "normal" selon horraire */
    else {
      if (sauvegarde.brassageOnOffPwm[numeroBrassage] == Arret) { // si brassage OFF
        variable.pwmBrassage[numeroBrassage] = zero; // puissance zero
        analogWrite(pinOutBrassage[numeroBrassage], variable.pwmBrassage[numeroBrassage]);
        //DPRINTF("brassage OFF");DPRINT(numeroBrassage); DPRINTLN(); //debug
      }
      else if (sauvegarde.brassageOnOffPwm[numeroBrassage] == Actif) { // si brassage ON
        variable.pwmBrassage[numeroBrassage] = sauvegarde.puissanceMaxBrassage[numeroBrassage]; // puissance MAX defini
        analogWrite(pinOutBrassage[numeroBrassage], variable.pwmBrassage[numeroBrassage] * pourCentEnPwm);
        //DPRINTF("brassage nominal");DPRINT(numeroBrassage); DPRINTLN(); //debug
      }
      else if (sauvegarde.brassageOnOffPwm[numeroBrassage] == Pwm) { // si brassage PWM acalmie la nuit, lever/coucher associer au soleil
        if ((Time >= debutleverSoleil[bleu1]) && (Time < finleverSoleil[blanc2])) { // augmentation pendent lever soleil
          variable.pwmBrassage[numeroBrassage] = map(Time, debutleverSoleil[bleu1], finleverSoleil[blanc2], puissanceMinBrassage[numeroBrassage], sauvegarde.puissanceMaxBrassage[numeroBrassage]);
          analogWrite (pinOutBrassage[numeroBrassage], variable.pwmBrassage[numeroBrassage] * pourCentEnPwm);
        }
        else if ((Time >= finleverSoleil[blanc2]) && (Time < debutcoucherSoleil[blanc1])) { // puissance MAX defini pendent le zenith
          variable.pwmBrassage[numeroBrassage] = sauvegarde.puissanceMaxBrassage[numeroBrassage];
          analogWrite(pinOutBrassage[numeroBrassage], variable.pwmBrassage[numeroBrassage] * pourCentEnPwm);
        }
        else if ((Time >= debutcoucherSoleil[blanc1]) && (Time < fincoucherSoleil[bleu2])) { // dimminition pendent coucher soleil
          variable.pwmBrassage[numeroBrassage] = map(Time, debutcoucherSoleil[blanc1], fincoucherSoleil[bleu2], sauvegarde.puissanceMaxBrassage[numeroBrassage], puissanceMinBrassage[numeroBrassage]);
          analogWrite (pinOutBrassage[numeroBrassage], variable.pwmBrassage[numeroBrassage] * pourCentEnPwm);
        }
        else {
          variable.pwmBrassage[numeroBrassage] = puissanceMinBrassage[numeroBrassage]; // puissance MAX minoré de la baisse accalmie/nourissage
          analogWrite(pinOutBrassage[numeroBrassage], variable.pwmBrassage[numeroBrassage] * pourCentEnPwm);
        }
        //DPRINTF("brassage pwm");DPRINT(numeroBrassage); DPRINTLN(); //debug
      }
      if (MajEcranTempeteEnCours ) { // boolean pour ne pas avoir de mise a jour de l'ecran a repetition
        MajEcranTempeteEnCours = !MajEcranTempeteEnCours;
        statutTempete();
      }
      if (MajEcranNourrissageEnCours ) { // boolean pour ne pas avoir de mise a jour de l'ecran a repetition
        MajEcranNourrissageEnCours = !MajEcranNourrissageEnCours;
      }
      if (tempeteAleatoireEnCours ) { // boolean pour ne pas avoir de mise a jour de l'ecran a repetition
        tempeteAleatoireEnCours = !tempeteAleatoireEnCours;
      }
    }
  }
}

/* defini l'heure et la duree de la tempete aleatoir */
void definiTempeteAleatoire() {
  heureTempeteAleatoire = random(finleverSoleil[blanc2], debutcoucherSoleil[blanc1]); // defini aléatoirement l'heure de la tempête (forcement pendant le zenith)
  dureeTempeteAleatoire = random((sauvegarde.dureeTempeteMillis / dix), (sauvegarde.dureeTempeteMillis )); // defini aléatoire la duree de la tempête (minimum : MAX/10 maximun: MAX defini)
  tempsPrecedentTempeteAleatoire = zero;
  DPRINTF("heure tempete aleatoire :"); DPRINT(heureTempeteAleatoire);  DPRINTLN(); // debug //debug
  DPRINTF("duree tempete aleatoire :"); DPRINT((dureeTempeteAleatoire / mille) / minuteEnSeconde);  DPRINTLN(); // debug //debug
}

/* declanche la tempete aleatoir si l'heure defini est ateint */
void tempeteAleatoireAutomatique() {
  if (sauvegarde.tempeteAleatoireOn ) { // si mode tempête aléatoire ON
    if (tempeteAleatoireEnCours == false) {
      if (Time == heureTempeteAleatoire ) { // declanchement de la tempête selon heure aléatoire
        tempsPrecedentTempeteAleatoire = millis(); // met le compteur a zero
        tempeteAleatoireEnCours = !tempeteAleatoireEnCours; // temepete aleatoir ON
        definiTempeteDuLendemain = true; // pour definir la tempetre du lendemain
        DPRINTF("declenchement automatique de la tempete Aleatoire");  DPRINTLN(); // debug //debug
        DPRINTF("time :"); DPRINT(Time); DPRINTF(" / heure tempete :"); DPRINT(heureTempeteAleatoire);  DPRINTLN(); // debug //debug
        DPRINT(millis()); DPRINTF(" / duree :"); DPRINT(dureeTempeteAleatoire);  DPRINTLN(); // debug //debug
        //delay (mille); // pour ne pas avoir un declanchement a repetition
      }
    }
  }
  if (sauvegarde.tempeteAleatoireOn ) { // si mode tempête aléatoire ON
    if (definiTempeteDuLendemain ) {
      if (Time == sauvegarde.coucherSoleil) { // prevoi la tempette du lendemain une fois le soleil couché
        definiTempeteAleatoire(); // lance la fonction
        definiTempeteDuLendemain = !definiTempeteDuLendemain; // pour ne pas definir la tempete aleatoir en boucle
        if (pageActuelNextion == menu) { // si sur page menu
          tempeteNextion(); // MAJ l'heure et duree de la tempete aleatoir a l'ecran
        }
      }
    }
  }
}
