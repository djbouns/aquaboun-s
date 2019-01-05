
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

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ Déclarations $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
/* entrées/sorties analogique */
const uint8_t pinInCoupureCourant = A4; // entree presence tension d'alimentation
const uint8_t pinInBatterie = A5; // entree niveau batterie
const uint8_t pinInPhBac = A6; // entree sonde PH BAC
const uint8_t pinInPhRac = A7; // entree sonde PH RAC
const uint8_t pinOutRelaisOsmolation = A8; // relais pompe osmolation
const uint8_t pinOutRelaisVentilateurBac = A9; // relais ventilateur bac
//A10 libre pour relais tableau
//A11 libre pour relais tableau
const uint8_t pinOutRelaisRemontee = A12; // relais coupure remontee // ouvert = ON
const uint8_t pinOutRelaisChauffage = A13; // relais coupure chauffage // ouvert = ON
const uint8_t pinOutRelaisElectovanneRac = A14; // relais electrovanne RAC
const uint8_t pinOutRelaisEcumeur = A15; // relais ecumeur // ouvert = ON


/* entrées/sorties digital */
//0 rx non disponible sur pcb
//1 tx non disponible sur pcb
const uint8_t pinOutBrassage[3] {2, 3, 4}; // sortie Pwmpompe brassage 1
#define pinInSondeDs18b20 5 // entree sonde temperature aquarium + rampe
const uint8_t pinOutRelaisVentilateurRampe = 6; // relais ventilateur rampe
const uint8_t pinOutEclairage[4] {8, 9, 7, 10}; // sortie PWM eclairage bleu1, bleu2, blanc1, blanc2
const uint8_t pinOUTOscillo1 = 11; // sortie PWM ocsillateur brassage 1
const uint8_t pinOUTOscillo2 = 12; // sortie PWM ocsillateur brassage 2
const uint8_t pinOUTOscillo3 = 13; // sortie PWM ocsillateur brassage 3
#define gsm Serial1 // pin18 = tx1 + pin19 = rx1
#define nextion Serial2 // pin16 = tx2 + pin17 = rx2
#define d1mini Serial3 // pin14 = tx3 + pin15 = rx3
//20 sda > ds3231
//21 scl > ds3231
//22 non disponible sur pcb
//23 non disponible sur pcb
const uint8_t hardResetGSM = 24; // reset du gsm
const uint8_t hardResetWIFI = 25; // reset du wifi
//26 non disponible sur pcb
//27 non disponible sur pcb
//28 non disponible sur pcb
//29 non disponible sur pcb
//30 non disponible sur pcb
const uint8_t pinInFloteurSecurite = 31;// entree flotteur securité decante (ouvert si niveau haut decante)
//32 non disponible sur pcb
const uint8_t pinInFloteurOsmolation = 33; // entree flotteur osmolation (ferme = ON)
//34 non disponible sur pcb
const uint8_t pinInFloteurGodetEcumeur = 35; // entree flotteur godet ecumeur (ferme = ON)
//33 non disponible sur pcb
const uint8_t pinInFloteurReserve = 37; // entree flotteur niveau bas reserve d'eau (ferme si niveau bas reserve d'eau osmose)
//38 non disponible sur pcb
const uint8_t pinInFloteurNiveauEcumeur = 39; // entree floteur niveau ecumeur (ferme = ON)
//40 non disponible sur pcb
const uint8_t pinOutRelaisDistributeurNouriture = 41; // relais distributeur nouriture
//42 non disponible sur pcb
const uint8_t pinOutBatterie = 43; // relais distributeur nouriture
//44 non disponible sur pcb
//45 non disponible sur pcb
//46 non disponible sur pcb
//47 non disponible sur pcb
const uint8_t pinOutbuzzer = 0; // sortie buzzer 48
//49 non disponible sur pcb
//50 miso > carte sd
//51 mosi > carte sd
//52 sck > carte sd
#define SD_CS_PIN // pin53  > carte sd
