
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
#include <EEPROM.h>
#include "Nextion.h"

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ Fichiers $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
#include "affichage.h"
#include "global.h"
#include "horloge.h"
#include "autres.h"
#include "pin.h"
#include "debug.h"
#include "eclairage.h"
#include "brassage.h"
#include "carteSD.h"
#include "gsm.h"
#include "temperature_ph.h"
#include "oscillo.h"
#include "wifi.h"
#include "eeprom.h"

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ *** ECRAN NEXTION 800*480 *** $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
/* Vous avez la possibilité d'utiliser plusieurs model d'ecran nextion du momment que la resolution est 800*480
  !!! IMPORTANT !!! >>> il faut imperativement utiliser un ecran nextion ORIGINAL, les copies ne reconnaise pas les fichier fait avec nextion editor <<< !!! IMPORTANT !!!
  https://nextion.itead.cc/nextion-shop/
  Afin de faire votre choix il faut evaluer vos besoins : taille 5" ou 7", version standard ou amelioré (processeur plus puissant, les autres ameliorations ne sont pas utilisées, nu ou avec boitier)
  >>> Le principe de l'écran tactile résistif : Un écran résistif est constitué de deux surfaces séparées par de minuscules entretoises et parcourues de connecteurs.
  Le touché du doigt ou d'une pointe quelconque déforme la surface supérieure et met en contact ses connecteurs avec ceux de la surface intérieure. L'utilisation d'un stylet est souvent necessaire.
  >>> Le principe de l'écran tactile capacitif : Quand le doigt, conducteur d'électricité, touche l'écran, des charges électriques lui sont transférées.
  voici les models compatibles actuelement :
  ecran 5" NX8048T050_011R <<< version standard resistif nue
  ecran 5" NX8048K050_011R <<< version amelioré resistif nue
  ecran 7" NX8048T070_011R <<< version standard resistif nue
  ecran 7" NX8048K070_011R <<< version amelioré resistif nue
  ecran 7" NX8048K070_011C <<< version amélioré capacitif boitier */

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ Déclarations $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
pageNextion pageActuelNextion;
uint32_t number; // pour stocker les numerique lu du nextion
uint32_t recupHorraireTemporraire; // pour recuperer calcul de l'horraire
uint8_t statutIconeTemperatureBac, statutIconeBatterie; // pour le statut des icones sur l'ecran
boolean statutIconeTemperatureRampe = true;
uint8_t valeurThermometreBac, valeurThermometreRampe; // pour le statut des thermometres sur l'ecran
uint8_t boucle = 1; // compteur pour rafraichir les parametre de la page menu a tour de role.
uint32_t refreshMenu; // millis pour rafraichire a tour de role la page menu

const uint16_t mille = 1000u, deuxMille = 2000u, cinqMille = 5000;
const uint32_t minuteEnMillis = 60000ul; // correspond a 1 minute en millisseconde
boolean onOffalimentation = true; // pour le statut du nourissage manuel
boolean alerteOsmolation = true; // pour ne pas avoir d'alerte a repetition
boolean etalonnageOnOFF = false; // pour le statut de l'etalonage ph
uint8_t changementPwmBrassage1, changementPwmBrassage2, changementPwmBrassage3; // pour le statut du brassage sur l'ecran
uint8_t changementPwmBlanc, changementPwmBleu; // pour le statut de l'eclairage sur l'ecran
uint16_t changementPhRac, changementTemperatureBac; // pour le statut du ph sur l'ecran
uint16_t changementPhBac; // pour le statut temperature sur l'ecran
uint8_t changementTemperatureRampe, changementThermometreRampe;  // pour le statut temperature sur l'ecran
uint8_t Mprecedent; // stock la minute actuel pour mise a jour des minutes dans nextion quand minute != Mprecedent
const uint8_t maxbufferTexte = 100; // place pour recuperer les message en progmem
char bufferTexte[maxbufferTexte + 1]; // place pour recuperer les message en progmem + carractere de fin
const char RampeAqua[] PROGMEM = "Rampe/Aqua"; // texte stocker en progmem
const char AquaRampe[] PROGMEM = "Aqua/Rampe"; // texte stocker en progmem
const char vide[] PROGMEM = ""; // texte stocker en progmem
const char texteNextionAlerteTemperatureBac[] PROGMEM = "ALERTE TEMPERATURE"; // texte stocker en progmem
const char texteNextionOsmolationOff[] PROGMEM = "OSMOLATEUR OFF"; // texte stocker en progmem
const char texteNextionPhHaut[] PROGMEM = "ALERTE PH HAUT"; // texte stocker en progmem
const char texteNextionPhBas[] PROGMEM = "ALERTE PH BAS"; // texte stocker en progmem
const char texteNextionReserveVide[] PROGMEM = "RESERVE VIDE"; // texte stocker en progmem
const char texteNextionNourissage[] PROGMEM = "Nourrissage en cours ..."; // texte stocker en progmem
const char texteNextionEnCharge[] PROGMEM = "En charge"; // texte stocker en progmem
const char texteNextionNonConnecte[] PROGMEM = "Non connecte"; // texte stocker en progmem
const char texteNextionModuleNonDetecte[] PROGMEM = "Module non detecte !!!"; // texte stocker en progmem
const char texteNextionetalonnage[] PROGMEM = "etalonnage en cours"; // texte stocker en progmem
boolean initialisationPhDemarrage = true;
boolean mesurePhDemarrage = true;
const uint16_t vert = 1632u; // code couleur du texte sur nextion
const uint16_t orange = 64962u; // code couleur du texte sur nextion
const uint16_t rouge = 63488u; // code couleur du texte sur nextion
const uint8_t pas2Changement2Couleur = 0; // garde le texte de la meme couleur que precedement
uint16_t mesureNiveauBatterie; // pour stocker le niveau de batterie
const uint16_t batterieNonConnecte = 760u; // batterie non connecté = 8.40V = 3.05V avec pont diviseur = 760 sur l'echelle analogique de 1024
const uint16_t batterie100 = 652u; // batterie 7.2V NiMh chargée a 7.2V = 2.60V = 652 avec un pont diviseur sur l'echelle analogique de 1024
const uint16_t batterie0 = 588u; // batterie 7.2V NiMh vide(6.5V) = 2.35V = 588 avec un pont diviseur sur l'echelle analogique de 1024
enum {Ibatterie0, Ibatterie16, Ibatterie32, Ibatterie50, Ibatterie68, Ibatterie84, Ibatterie100};
enum {ItempeteOn = 4, Ion = 5, Ioff = 6, Ipwm = 7, ItempeteOff = 8, IpetitVent = 9, IgrandVent = 10, Idelay = 11, Idanger = 12, IgrandVide = 15, IgrandOff = 17, IpetitVide = 19,
      Ipile100 = 28, Ipile84 = 29, Ipile68 = 30, Ipile50 = 31, Ipile32 = 32, Ipile16 = 33, Ipile0 = 34
     }; // Attribut un nom au icone du nextion

/*déclarations de tout les elements de l'ecran etant en interaction avec l'arduino */
// page demarage
NexPage pagedemarrage = NexPage(0, 0, "demarrage"); // adresse de la page demarage
NexText texte1 = NexText(0, 1, "t0"); // champ texte vierge
NexText texte2 = NexText(0, 1, "t1"); // champ texte vierge
NexText texte3 = NexText(0, 1, "t2"); // champ texte vierge
NexText texte4 = NexText(0, 1, "t3"); // champ texte vierge
NexText texte5 = NexText(0, 1, "t4"); // champ texte vierge
NexText texte6 = NexText(0, 1, "t5"); // champ texte vierge
// page menu
NexPage pageMenu = NexPage(1, 0, "menu"); // adresse de la page menu
NexVariable affichagepourcentblanc = NexVariable(1, 19, "n0"); // adresse pour l'affichage % blanc
NexVariable affichagepourcentbleu = NexVariable(1, 20, "n1"); // adresse pour l'affichage % bleu
NexVariable affichagepourcentbrassage1 = NexVariable(1, 1, "n2"); // adresse pour l'affichage % brassage 1
NexVariable affichagepourcentbrassage2 = NexVariable(1, 2, "n3"); // adresse pour l'affichage % brassage 2
NexVariable affichagepourcentbrassage3 = NexVariable(1, 21, "n4"); // adresse pour l'affichage % brassage 3
NexVariable affichagetemperaturerampe = NexVariable(1, 26, "n5"); // adresse pour l'affichage temperature rampe
NexVariable affichagetemperaturebac = NexVariable(1, 3, "n6"); // adresse pour l'affichage temperature aquarium
NexVariable affichagephbac = NexVariable(1, 28, "n10"); // adresse pour l'affichage PH BAC
NexVariable affichagephrac = NexVariable(1, 29, "n11"); // adresse pour l'affichage PH RAC
NexVariable variableetatboutons = NexVariable(1, 34, "va0"); // adresse de la variable qui retourne etat bouton
NexPicture iconetemperaturebac = NexPicture(1, 8, "p9"); // adresse de l'icone temperature aquarium
NexPicture iconeTemperatureRampe = NexPicture(1, 9, "p10"); // adresse de l'icone temperature rampe
NexButton onoffpomperemonte = NexButton(1, 10, "b0"); // adresse du bouton on/off pompe remonté
NexButton onoffalimentation1 = NexButton(1, 11, "b1"); // adresse du bouton on/off alimentation 1
NexButton onoffalimentation2 = NexButton(1, 12, "b2"); // adresse du bouton on/off alimentation 2
NexButton onoffpwmbrassage1 = NexButton(1, 13, "b3"); // adresse du bouton on/off/pwm brassage 1
NexButton onoffpwmbrassage2 = NexButton(1, 14, "b4"); // adresse du bouton on/off/pwm brassage 2
NexButton onoffpwmbrassage3 = NexButton(1, 15, "b5"); // adresse du bouton on/off/pwm brassage 3
NexButton onoffpwmblanc = NexButton(1, 16, "b6"); // adresse du bouton on/off/pwm blanc
NexButton onoffpwmbleu = NexButton(1, 17, "b7"); // adresse du bouton on/off/pwm bleu
NexButton onofftempete = NexButton(1, 18, "b8"); // adresse du bouton on/off tempete
NexButton onofftempetealeatoire = NexButton(1, 30, "b9"); // adresse du bouton on/off tempete aleatoire
NexButton boutonmenuversconfig = NexButton(1, 6, "m0"); // adresse du bouton menu vers config
NexButton boutonalimentationmanuel = NexButton(1, 31, "m2"); // adresse du bouton alimentation manuel
NexButton boutonetalonnage = NexButton(1, 37, "m3"); // adresse du bouton étalonnage ph
NexText affichageheure = NexText(1, 35, "t0"); // adresse pour l'affichage heure
NexText affichageheurealimentation1 = NexText(1, 22, "t4"); // adresse pour l'affichage heure alimentation 1
NexText affichageheurealimentation2 = NexText(1, 23, "t5"); // adresse pour l'affichage heure alimentation 2
NexText affichageheuretempete = NexText(1, 24, "t6"); // adresse pour l'affichage heure tempete
NexText affichagedureetempete = NexText(1, 25, "t7"); // adresse pour l'affichage duree tempete
NexScrolltext messagedefilantalimentationencours = NexScrolltext(1, 33, "g0"); // adresse pour l'affichage du message distrubution en cours ...
NexText messageetalonnage = NexText(1, 38, "t2"); //
NexText messageanomalie = NexText(1, 41, "t8"); // adresse pour l'affichage du message d'erreur temperature bac
NexText messageanomalie2 = NexText(1, 41, "t9"); // adresse pour l'affichage du message d'erreur osmolation
NexText messageanomalie3 = NexText(1, 42, "t10"); // adresse pour l'affichage du message d'erreur ph
NexProgressBar affichagethermometrebac = NexProgressBar(1, 4, "j0"); // adresse de la "progress bar" du thermometre de l'aquarium
NexProgressBar affichagethermometrerampe = NexProgressBar(1, 5, "j1"); // adresse de la "progress bar" du thermometre de la rampe
NexButton boutonversstandby = NexButton(1, 39, "m4"); // adresse du bouton config vers menu
// page config
NexPage pageConfig = NexPage(2, 0, "parametre"); // adresse de la page menu
NexButton boutonconfigversinfo = NexButton(2, 55, "m4"); // adresse du bouton bouton config vers info
NexButton boutonconfigverstel = NexButton(2, 28, "m2"); // adresse du bouton bouton config vers tel
NexButton boutonconfigverswifi = NexButton(2, 27, "m1"); // adresse du bouton bouton config vers wifi
NexVariable valeurblanc = NexVariable(2, 1, "n10"); // adresse pour l'affichage % blanc
NexVariable valeurbleu = NexVariable(2, 3, "n0"); // adresse pour l'affichage % bleu
NexVariable valeurleverSoleilh = NexVariable(2, 5, "n6"); // adresse pour l'affichage heure lever soleil
NexText valeurleverSoleilm = NexText(2, 6, "t1"); // adresse pour l'affichage minute lever soleil
NexVariable valeurcoucherSoleilh = NexVariable(2, 7, "n9"); // adresse pour l'affichage heure coucher soleil
NexText valeurcoucherSoleilm = NexText(2, 8, "t3"); // adresse pour l'affichage minute coucher soleil
NexVariable valeurdureelevercoucher = NexVariable(2, 10, "n22"); // adresse pour l'affichage duree lever/coucher soleil
NexVariable valeuralertetemperaturebasse = NexVariable(2, 11, "n1"); // adresse pour l'affichage alerte temperature bac basse
NexVariable valeuralertetemperaturehaute = NexVariable(2, 12, "n2"); // adresse pour l'affichage alerte temperature bac haute
NexVariable valeurventilationaquarium = NexVariable(2, 13, "n19"); // adresse pour l'affichage temperature ventilation bac
NexVariable valeurventilationrampe = NexVariable(2, 14, "n3"); // adresse pour l'affichage temperature ventilation rampe
NexVariable adressesonde = NexVariable(2, 81, "va2"); // adresse de la variable indique adresse sonde temperature bac / rampe
NexButton boutonadressesonde = NexButton(2, 15, "b30"); // adresse pour le bouton adresse sonde temperature bac / rampe
NexVariable valeurheureNourrissage1h = NexVariable(2, 16, "n13"); // adresse pour l'affichage heure nourissage 1
NexText valeurheureNourrissage1m = NexText(2, 17, "t4"); // adresse pour l'affichage minute nourissage 1
NexVariable valeurheureNourrissage2h = NexVariable(2, 18, "n16"); // adresse pour l'affichage heure nourissage 2
NexText valeurheureNourrissage2m = NexText(2, 19, "t5"); // adresse pour l'affichage minute nourissage 2
NexVariable valeurdureeNourrissageMillis = NexVariable(2, 20, "n21"); // adresse pour l'affichage duree nourissage
NexVariable valeurdureeosmolation = NexVariable(2, 24, "n7"); // adresse pour l'affichage duree osmolation
NexPicture valeurdureeosmolationoff = NexPicture(2, 79, "p0"); // adresse pour l'affichage OFF duree osmolation
NexVariable valeuralerteosmolation = NexVariable(2, 26, "n8"); // adresse pour l'affichage alerte trop osmolation
NexVariable valeurconsignePhRac = NexVariable(2, 21, "n4"); // adresse pour l'affichage  consigne ph rac
NexPicture valeurconsignePhRacoff = NexPicture(2, 80, "p1"); // adresse pour l'affichage OFF consigne ph rac
NexVariable valeuralertephbacbas = NexVariable(2, 22, "n5"); // adresse pour l'affichage ph bac bas
NexPicture valeuralertephbacbasoff = NexPicture(2, 82, "p2"); // adresse pour l'affichage OFF ph bac bas
NexVariable valeuralertephbachaut = NexVariable(2, 23, "n18"); // adresse pour l'affichage ph bac haut
NexPicture valeuralertephbachautoff = NexPicture(2, 81, "p3"); // adresse pour l'affichage OFF ph bac haut
NexButton boutonenregistrerconfigversmenu = NexButton(2, 54, "m3"); // // adresse du bouton enregistrer + config vers menu
NexButton boutonconfigversbrassage = NexButton(2, 64, "m5"); // adresse du bouton config vers brassage
NexButton boutonconfigvershorloge = NexButton(2, 68, "m6"); // adresse du bouton config vers horloge
NexButton boutonconfigversmenu = NexButton(2, 29, "m0"); // adresse du bouton config vers menu
NexText affichagebatterieencharge = NexText(2, 84, "t11"); // adresse pour l'affichage duree tempete
NexPicture iconebatterie = NexPicture(2, 83, "p4"); // adresse de l'icone temperature rampe
// page brassage
NexPage pageBrassage = NexPage(3, 0, "brassage"); // adresse de la page brassage
NexVariable valeurbrassage1 = NexVariable(3, 24, "n8"); // adresse pour l'affichage brassage 1
NexVariable valeurbrassage2 = NexVariable(3, 27, "n12"); // adresse pour l'affichage brassage 2
NexVariable valeurbrassage3 = NexVariable(3, 30, "n15"); // adresse pour l'affichage brassage 3
NexVariable valeuroscillo1min = NexVariable(3, 25, "n9"); // adresse pour l'affichage oscillo 1 angle min
NexVariable valeuroscillo2min = NexVariable(3, 28, "n13"); // adresse pour l'affichage oscillo 2 angle min
NexVariable valeuroscillo3min = NexVariable(3, 31, "n16"); // adresse pour l'affichage oscillo 3 angle min
NexVariable valeuroscillo1max = NexVariable(3, 26, "n11"); // adresse pour l'affichage oscillo 1 angle max
NexVariable valeuroscillo2max = NexVariable(3, 29, "n14"); // adresse pour l'affichage oscillo 2 angle max
NexVariable valeuroscillo3max = NexVariable(3, 32, "n17"); // adresse pour l'affichage oscillo 3 angle max
NexButton boutonomoinsoscillo1min = NexButton(3, 7, "b31"); // adresse pour le bouton oscillo 1 angle min -
NexButton boutonplusoscillo1min = NexButton(3, 10, "b34"); // adresse pour le bouton oscillo 1 angle min +
NexButton boutonomoinsoscillo1max = NexButton(3, 8, "b32"); // adresse pour le bouton oscillo 1 angle max -
NexButton boutonplusoscillo1max = NexButton(3, 11, "b35"); // adresse pour le bouton oscillo 1 angle max +
NexButton boutonomoinsoscillo2min = NexButton(3, 13, "b37"); // adresse pour le bouton oscillo 2 angle min -
NexButton boutonplusoscillo2min = NexButton(3, 16, "b40"); // adresse pour le bouton oscillo 2 angle min +
NexButton boutonomoinsoscillo2max = NexButton(3, 14, "38"); // adresse pour le bouton oscillo 2 angle max -
NexButton boutonplusoscillo2max = NexButton(3, 17, "b41"); // adresse pour le bouton oscillo 2 angle max +
NexButton boutonomoinsoscillo3min = NexButton(3, 19, "b43"); // adresse pour le bouton oscillo 3 angle min -
NexButton boutonplusoscillo3min = NexButton(3, 22, "b46"); // adresse pour le bouton oscillo 3 angle min +
NexButton boutonomoinsoscillo3max = NexButton(3, 20, "44"); // adresse pour le bouton oscillo 3 angle max -
NexButton boutonplusoscillo3max = NexButton(3, 23, "b47"); // adresse pour le bouton oscillo 3 angle max +
NexVariable affichagedegreoscillo1 = NexVariable(3, 3, "n21"); // adresse pour l'affichage angle oscillo 1
NexVariable affichagedegreoscillo2 = NexVariable(3, 4, "n22"); // adresse pour l'affichage angle oscillo 2
NexVariable affichagedegreoscillo3 = NexVariable(3, 5, "n23"); // adresse pour l'affichage angle oscillo 3
NexVariable valeurmouvementoscillo1 = NexVariable(3, 54, "n7"); // adresse pour l'affichage vitesse oscillo 1
NexVariable valeurmouvementoscillo2 = NexVariable(3, 55, "n1"); // adresse pour l'affichage vitesse oscillo 2
NexVariable valeurmouvementoscillo3 = NexVariable(3, 56, "n2"); // adresse pour l'affichage vitesse oscillo 3
NexPicture valeurmouvementoscillo1off = NexPicture(3, 63, "p2"); // adresse pour l'affichage OFF oscillo 1
NexPicture valeurmouvementoscillo2off = NexPicture(3, 62, "p1"); // adresse pour l'affichage OFF oscillo 2
NexPicture valeurmouvementoscillo3off = NexPicture(3, 64, "p3"); // adresse pour l'affichage OFF oscillo 3
NexVariable valeurdureetempete = NexVariable(3, 2, "n6");  // adresse pour l'affichage duree tempete
NexVariable valeurtempete = NexVariable(3, 39, "n20"); // adresse pour l'affichage puissance tempete
NexVariable valeuraccalemienocturne = NexVariable(3, 64, "n0");  // adresse pour l'affichage accalmie
NexPicture valeuraccalemienocturneoff = NexPicture(3, 65, "p7"); // adresse pour l'affichage OFF accalmie
NexButton boutonenregistrerbrassageversconfig = NexButton(3, 41, "m3"); // bouton enregistrer + brassage vers parametre
NexButton boutonbrassageversconfig = NexButton(3, 40, "m0"); // bouton brassage vers parametre
// page horloge
NexPage pageHorloge = NexPage(4, 0, "horloge"); // adresse de la page horloge
NexButton boutonhorlogeversconfig = NexButton(4, 2, "m0"); // bouton horloge vers parametre
NexButton boutonenregistrerhorlogeversconfig = NexButton(4, 1, "b0"); // bouton enregistrer + horloge vers parametre
NexText affichageH = NexText(4, 20, "t3"); // champ texte heure
NexText affichageM = NexText(4, 6, "t4"); // champ texte minute
NexText affichagejour = NexText(4, 21, "t5"); // champ texte jour
NexText affichagemois = NexText(4, 22, "t6"); // champ texte mois
NexText affichageannee = NexText(4, 23, "t7"); // champ texte annee
NexText affichageHdebutalerte = NexText(4, 29, "t11"); // champ texte heure
NexText affichageHfinalerte = NexText(4, 34, "t15"); // champ texte heure
// page wifi
NexPage pageWifi = NexPage(5, 0, "wifi"); // adresse de la page wifi
NexButton boutonwifiversconfig = NexButton(5, 1, "m0"); // bouton wifi vers parametre
NexButton boutonenregistrerwifiversconfig = NexButton(5, 92, "m84"); // bouton enregistrer + wifi vers parametre
NexText valeurssid = NexText(5, 2, "t0"); // champ texte SSID
NexText valeurmdp = NexText(5, 89, "t1"); // champ texte MDP
NexText valeurconnecte = NexText(5, 94, "t4"); // champ texte connecte
NexProgressBar dbsignalwifi1 = NexProgressBar(5, 95, "j1"); // gauge niveau signal
NexProgressBar dbsignalwifi2 = NexProgressBar(5, 96, "j2"); // gauge niveau signal
NexProgressBar dbsignalwifi3 = NexProgressBar(5, 97, "j3"); // gauge niveau signal
NexProgressBar dbsignalwifi4 = NexProgressBar(5, 98, "j4"); // gauge niveau signal
NexProgressBar dbsignalwifi5 = NexProgressBar(5, 99, "j5"); // gauge niveau signal
// page tel
NexPage pageTel = NexPage(6, 0, "tel"); // adresse de la page tel
NexButton boutontelversconfig = NexButton(6, 1, "m0"); // bouton tel vers parametre
NexButton boutonenregistrertelversconfig = NexButton(6, 15, "m84"); // bouton enregistrer + tel vers parametre
NexText valeurtel = NexText(6, 13, "t0"); // champ texte tel
NexText valeuroperateur = NexText(6, 17, "t4"); // champ texte connecte
NexProgressBar dbsignalgsm1 = NexProgressBar(6, 18, "j1"); // gauge niveau signal
NexProgressBar dbsignalgsm2 = NexProgressBar(6, 19, "j2"); // gauge niveau signal
NexProgressBar dbsignalgsm3 = NexProgressBar(6, 20, "j3"); // gauge niveau signal
NexProgressBar dbsignalgsm4 = NexProgressBar(6, 21, "j4"); // gauge niveau signal
NexProgressBar dbsignalgsm5 = NexProgressBar(6, 22, "j5"); // gauge niveau signal
// page info
NexPage pageInfo = NexPage(7, 0, "info"); // adresse de la page info
NexButton boutoninfoversconfig = NexButton(7, 1, "m0"); // bouton info vers parametre
// page standby
NexPage pagestandby = NexPage(8, 0, "standby"); // adresse de la page standby

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ LES FONCTIONS $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< affichage des pages
/*  page menu, lance toute les fonction pour connaitre les variables a afficher et l'etat des boutons */
void affichePageMenu() {
  reinitialisationDeLaffichage(); // remet tout les variable a zero avant MAJ
  pageMenu.show(); // affiche la page menu
  pageActuelNextion = menu; // indique a l'arduino que la page actuel est menu
  affichageHeureNextion();
  rafraichirEclairageBlancNextion();
  rafraichirEclairageBleuNextion();
  rafraichirBrassage1Nextion();
  rafraichirBrassage2Nextion();
  rafraichirBrassage3Nextion();
  rafraichirTemperatureBacNextion();
  rafraichirTemperatureRampeNextion();
  rafraichirPhBacNextion();
  rafraichirPhRacNextion();
  statutBlanc();
  statutBleu();
  statutBrassage1();
  statutBrassage2();
  statutBrassage3();
  statutTempete();
  statutTempeteAleatoire();
  statutOnOffAlim1();
  statutOnOffAlim2();
  statutNourrissageEnCours();
  statutOnOffDelay();
  afficherAlerteOsmolationOffReserveVide();
  afficherAlerteOsmolation();
  afficherAlertepPhBacBas();
  afficherAlertepPhBacHaut();
  if (initialisationPhDemarrage) {
    texteProgmemAuNextion(messageetalonnage, texteNextion32, pas2Changement2Couleur); // champ, texte, couleur
  }
  DPRINTF("page menu");  DPRINTLN(); // debug // debug
}

/* page config, envoie les variables a afficher */
void affichePageConfig() {
  uint32_t temporairedureeNourrissageMillis;
  pageConfig.show(); // affiche la page config
  pageActuelNextion = parametre; // indique a l'arduino que la page actuel est config
  valeurblanc.setValue(sauvegarde.puissanceMaxEclairage[blanc1]);
  valeurbleu.setValue(sauvegarde.puissanceMaxEclairage[bleu1]);
  secondeEnHorraire(sauvegarde.leverSoleil); // decoupe horraire en deux, les heure en numerique et les minutes en texte
  valeurleverSoleilh.setValue(number); // heure en numerique
  valeurleverSoleilm.setText(minuteEnTexteOut); // minute en texte
  secondeEnHorraire(sauvegarde.coucherSoleil); // decoupe horraire en deux, les heure en numerique et les minutes en texte
  valeurcoucherSoleilh.setValue(number); // heure en numerique
  valeurcoucherSoleilm.setText(minuteEnTexteOut); // minute en texte
  valeurdureelevercoucher.setValue(sauvegarde.dureelevercoucher / minuteEnSeconde); // affiche la duree du lever en minute
  valeuralertetemperaturebasse.setValue(sauvegarde.alertetemperaturebasse * dix); // converti pour avoir 1 seul decimal
  valeuralertetemperaturehaute.setValue(sauvegarde.alertetemperaturehaute * dix); // converti pour avoir 1 seul decimal
  valeurventilationaquarium.setValue(sauvegarde.ventilationaquarium * dix); // converti pour avoir 1 seul decimal
  valeurventilationrampe.setValue(sauvegarde.ventilationrampe);
  if (sauvegarde.adresseSondeRampe == un) { // recupere adresse des sondes de temperature pour l'afficher
    adressesonde.setValue(un);
    strncpy_P (bufferTexte, RampeAqua, maxbufferTexte); // recupere "char" en memoire flash et le copie
    bufferTexte[maxbufferTexte] = '\0'; // ajoute le caractere de fin
    boutonadressesonde.setText(bufferTexte);// envoi char a l'ecran nextion
  }
  else {
    adressesonde.setValue(zero);
    strncpy_P (bufferTexte, AquaRampe, maxbufferTexte); // recupere "char" en memoire flash et le copie
    bufferTexte[maxbufferTexte] = '\0'; // ajoute le caractere de fin
    boutonadressesonde.setText(bufferTexte);// envoi char a l'ecran nextion
  }
  secondeEnHorraire(sauvegarde.heureNourrissage[numeroUn]) ;
  valeurheureNourrissage1h.setValue(number);
  valeurheureNourrissage1m.setText(minuteEnTexteOut);
  secondeEnHorraire(sauvegarde.heureNourrissage[numeroDeux]) ;
  valeurheureNourrissage2h.setValue(number);
  valeurheureNourrissage2m.setText(minuteEnTexteOut);
  temporairedureeNourrissageMillis = sauvegarde.dureeNourrissageMillis / minuteEnMillis; // affiche la duree en minute
  valeurdureeNourrissageMillis.setValue(temporairedureeNourrissageMillis);
  valeurdureeosmolation.setValue(sauvegarde.dureeOsmolationMillis / mille); // affiche la duree en minute
  if (sauvegarde.dureeOsmolationMillis < mille) { // si duree osmolation = 0 affichage OFF
    valeurdureeosmolationoff.setPic(IgrandOff);
  }
  valeuralerteosmolation.setValue(sauvegarde.compteurOsmolationMax);
  valeurconsignePhRac.setValue(sauvegarde.consignePhRac);
  valeuralertephbacbas.setValue(sauvegarde.alertephbacbas);
  valeuralertephbachaut.setValue(sauvegarde.alertephbachaut);
  rafraichirIconeBatterie();
  etalonnageOnOFF = false;
  DPRINTF("page config");  DPRINTLN(); // debug
}

/*  page info, affiche la page */
void affichePageInfo() {
  pageInfo.show();// affiche la page info
  pageActuelNextion = info; // indique a l'arduino que la page actuel est info
  DPRINTF("page info");  DPRINTLN(); // debug
}

/*  page tel, affiche la page */
void affichePageTel() {
  pageTel.show();// affiche la page tel
  pageActuelNextion = tel; // indique a l'arduino que la page actuel est tel
  valeurtel.setText(numero2tel); // envoie le num de tel actuel
  rafraichirSignalGSM();
  DPRINTF("page tel");  DPRINTLN(); // debug
}

/*  page wifi, affiche la page */
void affichePageWifi() {
  pageWifi.show();// affiche la page wifi
  pageActuelNextion = wifi; // indique a l'arduino que la page actuel est wifi
  valeurssid.setText(identifiantWifi.ssid); // recupere la valeur dans nextion
  valeurmdp.setText(identifiantWifi.mdp); // recupere la valeur dans nextion
  rafraichirSignalWifi();
  DPRINTF("page wifi");  DPRINTLN(); // debug
}

/*  page brassage, envoie les variables a afficher */
void affichePageBrassage() {
  pageBrassage.show(); // affiche la page brassage
  pageActuelNextion = brass;  // indique a l'arduino que la page actuel est
  valeurbrassage1.setValue(sauvegarde.puissanceMaxBrassage[numeroUn]);
  valeurbrassage2.setValue(sauvegarde.puissanceMaxBrassage[numeroDeux]);
  valeurbrassage3.setValue(sauvegarde.puissanceMaxBrassage[numeroTrois]);
  valeuroscillo1min.setValue(sauvegarde.angle1Oscillo[numeroUn]);
  valeuroscillo2min.setValue(sauvegarde.angle1Oscillo[numeroDeux]);
  valeuroscillo3min.setValue(sauvegarde.angle1Oscillo[numeroTrois]);
  valeuroscillo1max.setValue(sauvegarde.angle2Oscillo[numeroUn]);
  valeuroscillo2max.setValue(sauvegarde.angle2Oscillo[numeroDeux]);
  valeuroscillo3max.setValue(sauvegarde.angle2Oscillo[numeroTrois]);
  affichagedegreoscillo1.setValue(positionOscillo[numeroUn]);
  affichagedegreoscillo2.setValue(positionOscillo[numeroDeux]);
  affichagedegreoscillo3.setValue(positionOscillo[numeroTrois]);
  valeurmouvementoscillo1.setValue(sauvegarde.delaisMouvementOscilloMillis[numeroUn] / mille); // si delais mouvement = 0 affichage OFF
  if (sauvegarde.delaisMouvementOscilloMillis[numeroUn] < mille) {
    valeurmouvementoscillo1off.setPic(IgrandOff);
  }
  valeurmouvementoscillo2.setValue(sauvegarde.delaisMouvementOscilloMillis[numeroDeux] / mille); // si delais mouvement = 0 affichage OFF
  if (sauvegarde.delaisMouvementOscilloMillis[numeroDeux] < mille) {
    valeurmouvementoscillo2off.setPic(IgrandOff);
  }
  valeurmouvementoscillo3.setValue(sauvegarde.delaisMouvementOscilloMillis[numeroTrois] / mille); // si delais mouvement = 0 affichage OFF
  if (sauvegarde.delaisMouvementOscilloMillis[numeroTrois] < mille) {
    valeurmouvementoscillo3off.setPic(IgrandOff);
  }
  valeurdureetempete.setValue(sauvegarde.dureeTempeteMillis / minuteEnMillis); // affiche en minute
  valeurtempete.setValue(sauvegarde.puissanceTempete);
  valeuraccalemienocturne.setValue(sauvegarde.accalmieNocturne);
  if (sauvegarde.accalmieNocturne < 5) { // si puissance accalmie = 0 affichage OFF
    valeuraccalemienocturneoff.setPic(IgrandOff);
  }
  DPRINTF("page brassage");  DPRINTLN(); // debug
}

/*  page horloge, envoie les variables a afficher */
void affichePageHorloge() {
  uint32_t stockNumerique;
  pageHorloge.show(); // affiche la page horloge
  DPRINTF("minute avant H");  DPRINTLN(); // debug // debug
  pageActuelNextion = horloge; // indique a l'arduino que la page actuel est
  numEnChar(variable.Heure); // converti numerique en texte
  DPRINTF("minute apres H");  DPRINTLN(); // debug // debug
  affichageH.setText(minuteEnTexteOut);
  DPRINTF("Minute apres numenchar");  DPRINTLN(); // debug // debug
  numEnChar(variable.minut); // converti numerique en texte
  affichageM.setText(minuteEnTexteOut);
  numEnChar(jour); // converti numerique en texte
  affichagejour.setText(minuteEnTexteOut);
  numEnChar(mois); // converti numerique en texte
  affichagemois.setText(minuteEnTexteOut);
  stockNumerique = annee - deuxMille; // pour ne garder que les 2 derniers chiffres des annees
  numEnChar(stockNumerique); // converti numerique en texte
  affichageannee.setText(minuteEnTexteOut);
  stockNumerique = sauvegarde.heureDebutAlerte / 3600ul;
  numEnChar(stockNumerique); // converti numerique en texte
  affichageHdebutalerte.setText(minuteEnTexteOut);
  stockNumerique = sauvegarde.heureFinAlerte / 3600ul;
  numEnChar(stockNumerique); // converti numerique en texte
  affichageHfinalerte.setText(minuteEnTexteOut);
  DPRINTF("page horloge");  DPRINTLN(); // debug
}

/* rafraichi l'heure a chaque changement de minute si la page menu est en cours */
void rafraichirHeureSiSurMenu() {
  if (Mprecedent != variable.minut) {
    if  (pageActuelNextion == menu) { // MAJ de l'heure sur nextion a chaque changement de minute.
      affichageHeureNextion();
      DPRINTF("refresh heure");  DPRINTLN(); // debug
      Mprecedent = variable.minut;
    }
  }
}

/* reinitialise les variable a afficher */
void reinitialisationDeLaffichage() {
  // temperature();
  // horaire();
  // eclairage();
  // brassage();
  //ph();
  changementPwmBrassage1 = zero;
  changementPwmBrassage2 = zero;
  changementPwmBrassage3 = zero;
  changementTemperatureRampe = zero;
  changementTemperatureBac = zero;
  changementPwmBlanc = zero;
  changementPwmBleu = zero;
  changementPhBac = zero;
  changementPhRac = zero;
  valeurThermometreBac = zero;
  valeurThermometreRampe = zero;
  statutIconeTemperatureRampe = !statutIconeTemperatureRampe;
  statutIconeTemperatureBac = !statutIconeTemperatureBac;
  Mprecedent = minuteEnSeconde; // Mprecedent = 60 pour etre forcement MAJ
}

/* MAJ des temperature rampe + thermometre rampe si les valeurs on changées */
void rafraichirTemperatureRampeNextion() {
  if (pageActuelNextion == menu) { // si sur page menu
    const uint8_t valeurMaxThermometre = 100; // valeur max du thermo nextion
    const uint8_t temperatureMaxThermometreRampe = 65; // temperature max pris par le thermometre rampe
    const uint8_t PourAdapterAuThermometre1 = 20; // pour adapter la temperature a l'echelle du thermometre
    const uint8_t PourAdapterAuThermometre2 = 2; // pour adapter la temperature a l'echelle du thermometre
    uint8_t adaptationTemperatureRampeAuThermometre = ((variable.temperatureRampe - PourAdapterAuThermometre1) * PourAdapterAuThermometre2); // adapte la temperature rampe au thermometre
    if (changementTemperatureRampe != variable.temperatureRampe) { // si la temperature de la rampe a changer
      changementTemperatureRampe = variable.temperatureRampe;
      affichagetemperaturerampe.setValue(changementTemperatureRampe); // affiche la nouvelle valeur
      if (variable.temperatureRampe >= temperatureMaxThermometreRampe) { // si temperature rampe superieur l'echelle du thermometre
        if (valeurThermometreRampe != valeurMaxThermometre) {
          valeurThermometreRampe = valeurMaxThermometre; // prend la valeur max
          affichagethermometrerampe.setValue(valeurThermometreRampe); // affiche la nouvelle valeur
          DPRINTF("thermometre rampe hors echelle ");  DPRINTLN(); // debug
        }
      }
      else { // si la temperature de la rampe est dans l'echelle du thermometre
        if (valeurThermometreRampe != adaptationTemperatureRampeAuThermometre) { // si la valeur du thermometre a changer
          valeurThermometreRampe = adaptationTemperatureRampeAuThermometre;
          affichagethermometrerampe.setValue(valeurThermometreRampe); // affiche la nouvelle valeur
          DPRINTF("adapte thermometre ");  DPRINTLN(); // debug
        }
        if (etatVentilationRampe) { // variation de l'icone temperature rampe
          DPRINTF("etat ventilo true ");  DPRINTLN(); // debug
          if (statutIconeTemperatureRampe == false) {
            iconeTemperatureRampe.setPic(IpetitVent);
            statutIconeTemperatureRampe = !statutIconeTemperatureRampe;
            DPRINTF("icone rampe ventilo ");  DPRINTLN(); // debug
          }
        }
        else {
          DPRINTF("else ... rampe ");  DPRINTLN(); // debug
          if (statutIconeTemperatureRampe ) { // variation de l'icone temperature rampe
            iconeTemperatureRampe.setPic(IpetitVide);
            statutIconeTemperatureRampe = !statutIconeTemperatureRampe;
            DPRINTF("icone rampe vide ");  DPRINTLN(); // debug
          }
        }
      }
      DPRINTF("refresh temperature rampe : ")DPRINT(variable.temperatureRampe);  DPRINTLN(); // debug
    }
  }
}

/* MAJ des temperature bac + thermometre bac si les valeurs on changées */
void rafraichirTemperatureBacNextion() {
  if (pageActuelNextion == menu) { // si sur page menu
    const uint8_t valeurMaxThermometre = 100; // valeur max du thermo nextion
    const uint8_t valeurMinThermometre = 0; // valeur min du thermo nextion
    const uint8_t temperatureBacMaxDuThermometre = 28; // temperature max pris par le thermometre aquarium
    const uint8_t temperatureBacMinDuThermometre = 24; // temperature min pris par le themrometre aquarium
    const uint8_t PourAdapterAuThermometre = 25; // pour adapter la temperature a l'echelle du thermometre
    uint8_t adaptationTemperatureBacAuThermometre = ((variable.temperatureBac - temperatureBacMinDuThermometre) * PourAdapterAuThermometre); // adapte la temperature aquarium au thermometre
    if (changementTemperatureBac != variable.temperatureBac * dix) { // si la valeur a changer
      changementTemperatureBac = variable.temperatureBac * dix; // multipli pour avoir un entier
      affichagetemperaturebac.setValue(changementTemperatureBac); // affiche temperature aquarium
      if (variable.temperatureBac >= temperatureBacMaxDuThermometre) { // variation thermo temperature aquarium
        if (valeurThermometreBac != valeurMaxThermometre) { // si temperature bac superieur l'echelle du thermometre
          valeurThermometreBac = valeurMaxThermometre; // prend la valeur max
          affichagethermometrebac.setValue(valeurThermometreBac); // affiche la nouvelle valeur
        }
      }
      else if (variable.temperatureBac <= temperatureBacMinDuThermometre) { // si temperature bac inferieur l'echelle du thermometre
        if (valeurThermometreBac != valeurMinThermometre) {
          valeurThermometreBac = valeurMinThermometre;
          affichagethermometrebac.setValue(valeurThermometreBac); // affiche la nouvelle valeur
        }
      }
      else {// si temperature bac dans l'echelle du thermometre
        if (valeurThermometreBac != adaptationTemperatureBacAuThermometre) {// variation de l'icone temperature bac
          valeurThermometreBac = adaptationTemperatureBacAuThermometre;
          affichagethermometrebac.setValue(valeurThermometreBac); // affiche la nouvelle valeur
        }
      }
      if (etatVentilationBac && variable.temperatureBac < sauvegarde.alertetemperaturehaute) {// variation de l'icone temperature bac
        if (statutIconeTemperatureBac != IgrandVent) {
          iconetemperaturebac.setPic(IgrandVent); // affiche la nouvelle icone
          statutIconeTemperatureBac = IgrandVent;
          texteProgmemAuNextion(messageanomalie, vide, pas2Changement2Couleur); // champ, texte, couleur
        }
      }
      else if (variable.temperatureBac >= sauvegarde.alertetemperaturehaute) {// variation de l'icone temperature bac
        if (statutIconeTemperatureBac != Idanger) {
          iconetemperaturebac.setPic(Idanger); // affiche la nouvelle icone
          statutIconeTemperatureBac = Idanger;
          texteProgmemAuNextion(messageanomalie, texteNextionAlerteTemperatureBac, pas2Changement2Couleur); // champ, texte, couleur
        }
      }
      else if (variable.temperatureBac <= sauvegarde.alertetemperaturebasse) {// variation de l'icone temperature bac
        if (statutIconeTemperatureBac != Idanger) {
          iconetemperaturebac.setPic(Idanger); // affiche la nouvelle icone
          statutIconeTemperatureBac = Idanger;
          texteProgmemAuNextion(messageanomalie, texteNextionAlerteTemperatureBac, pas2Changement2Couleur); // champ, texte, couleur
        }
      }
      else {
        if (statutIconeTemperatureBac != IgrandVide) {// variation de l'icone temperature bac
          iconetemperaturebac.setPic(IgrandVide); // affiche la nouvelle icone
          statutIconeTemperatureBac = IgrandVide;
          texteProgmemAuNextion(messageanomalie, vide, pas2Changement2Couleur); // champ, texte, couleur
        }
      }
      DPRINTF("refresh temperature aquarium : "); DPRINT(variable.temperatureBac);  DPRINTLN(); // debug
    }
  }
  messageAlerteTemperature();
}

/* MAJ de l'eclairage blanc */
void rafraichirEclairageBlancNextion() {
  if (pageActuelNextion == menu) { // si sur page menu
    if (changementPwmBlanc != ((variable.pwmEclairage[blanc1] + variable.pwmEclairage[blanc2]) / 2)) { // rafraichit si valeur different de precedement
      changementPwmBlanc = ((variable.pwmEclairage[blanc1] + variable.pwmEclairage[blanc2]) / 2);
      affichagepourcentblanc.setValue(changementPwmBlanc); // affiche % Blanc
      DPRINTF("refresh pwm blanc");  DPRINTLN(); // debug
    }
  }
}

/* MAJ de l'eclairage bleu */
void rafraichirEclairageBleuNextion() {
  if (pageActuelNextion == menu) { // si sur page menu
    if (changementPwmBleu != ((variable.pwmEclairage[bleu1] + variable.pwmEclairage[bleu2]) / 2)) { // rafraichit si valeur different de precedement
      changementPwmBleu = ((variable.pwmEclairage[bleu1] + variable.pwmEclairage[bleu2]) / 2);
      affichagepourcentbleu.setValue(changementPwmBleu); // affiche % bleu
      DPRINTF("refresh pwm bleu");  DPRINTLN(); // debug
    }
  }
}

/* MAJ du brassage 1 */
void rafraichirBrassage1Nextion() {
  if (pageActuelNextion == menu) { // si sur page menu
    if (changementPwmBrassage1 != variable.pwmBrassage[numeroUn]) { // rafraichit si valeur different de precedement
      changementPwmBrassage1 = variable.pwmBrassage[numeroUn];
      affichagepourcentbrassage1.setValue(changementPwmBrassage1); // affiche % brassage 1
      DPRINTF("refresh pwm brassage1");  DPRINTLN(); // debug
    }
  }
}

/* MAJ du brassage 2 */
void rafraichirBrassage2Nextion() {
  if (pageActuelNextion == menu) { // si sur page menu
    if (changementPwmBrassage2 != variable.pwmBrassage[numeroDeux]) { // rafraichit si valeur different de precedement
      changementPwmBrassage2 = variable.pwmBrassage[numeroDeux];
      affichagepourcentbrassage2.setValue(changementPwmBrassage2); // affiche % brassage 2
      DPRINTF("refresh pwm brassage2");  DPRINTLN(); // debug
    }
  }
}

/* MAJ du brassage 3 */
void rafraichirBrassage3Nextion() {
  if (pageActuelNextion == menu) { // si sur page menu
    if (changementPwmBrassage3 != variable.pwmBrassage[numeroTrois]) { // rafraichit si valeur different de precedement
      changementPwmBrassage3 = variable.pwmBrassage[numeroTrois];
      affichagepourcentbrassage3.setValue(changementPwmBrassage3); // affiche % brassage 3
      DPRINTF("refresh pwm brassage3");  DPRINTLN(); // debug
    }
  }
}

/* MAJ ddu ph bac */
void rafraichirPhBacNextion() {
  if (pageActuelNextion == menu) { // si sur page menu
    if (changementPhBac != (variable.phBac)) {// rafraichit si valeur different de precedement
      changementPhBac = (variable.phBac);
      affichagephbac.setValue(changementPhBac); // affiche le ph du bac
      DPRINTF("refresh ph bac");  DPRINTLN(); // debug
    }
  }
}

/* MAJ ddu ph rac */
void rafraichirPhRacNextion() {
  if (pageActuelNextion == menu) { // si sur page menu
    if (changementPhRac != (variable.phRac)) {// rafraichit si valeur different de precedement
      changementPhRac = (variable.phRac);
      affichagephrac.setValue(changementPhRac); // affiche le ph du rac
      DPRINTF("refresh ph rac");  DPRINTLN(); // debug
    }
  }
}

void rafraichirIconeBatterie() {
  const uint8_t margeBatterie = 20; // 20 = marge de 0.08V apres pont diviseur
  mesureNiveauBatterie = analogRead(pinInBatterie);
  DPRINTF("mesure batterie : "); DPRINT(mesureNiveauBatterie); DPRINTF(" = "); DPRINT(mesureNiveauBatterie / 90.22); DPRINTF("V "); DPRINTLN(); // debug
  if ( mesureNiveauBatterie > (batterieNonConnecte - margeBatterie)) {
    mesureNiveauBatterie = Ibatterie0;
  }
  else if (mesureNiveauBatterie < batterie0) {
    mesureNiveauBatterie = Ibatterie0;
  }
  else {
    mesureNiveauBatterie = map(mesureNiveauBatterie, batterie0, batterie100, Ibatterie0, Ibatterie100);
    DPRINTF(" Map batterie : "); DPRINT(mesureNiveauBatterie);  DPRINTLN(); // debug
  }
  if (mesureNiveauBatterie > Ibatterie84) {
    iconebatterie.setPic(Ipile100); // affiche la nouvelle icone
    DPRINTF("icone batterie 100%");  DPRINTLN(); // debug
  }
  else if (mesureNiveauBatterie > Ibatterie68) {
    iconebatterie.setPic(Ipile84); // affiche la nouvelle icone
    DPRINTF("icone batterie 84%");  DPRINTLN(); // debug
  }
  else if (mesureNiveauBatterie > Ibatterie50) {
    iconebatterie.setPic(Ipile68); // affiche la nouvelle icone
    DPRINTF("icone batterie 68%");  DPRINTLN(); // debug
  }
  else if (mesureNiveauBatterie > Ibatterie32) {
    iconebatterie.setPic(Ipile50); // affiche la nouvelle icone
    DPRINTF("icone batterie 50%");  DPRINTLN(); // debug
  }
  else if (mesureNiveauBatterie > Ibatterie16) {
    iconebatterie.setPic(Ipile32); // affiche la nouvelle icone
    DPRINTF("icone batterie 32%");  DPRINTLN(); // debug
  }
  else if (mesureNiveauBatterie > Ibatterie0) {
    iconebatterie.setPic(Ipile16); // affiche la nouvelle icone
    DPRINTF("icone batterie 16%");  DPRINTLN(); // debug
  }
  else {
    iconebatterie.setPic(Ipile0); // affiche la nouvelle icone
    DPRINTF("icone batterie 0%");  DPRINTLN(); // debug
  }
}


void rafraichirSignalWifi() { // affiche le statut de connection et niveau du signal
  //qualité du signal en DB negatif, -80 signal trop faible, -50DB signal tres fort // convertie en positif
  uint8_t db65 = 65;
  uint8_t db70 = 70;
  uint8_t db75 = 75;
  uint8_t db80 = 80;
  uint8_t db85 = 85;
  uint8_t full = 100; // 100% de la jauge
  executer(RECEVOIRconnectionWifi, true); // demande le statut de connection, signal, IP
  if (connectionWifi.presenceModule ) {
    if (connectionWifi.connecter == false) { // si non connecter
      connectionWifi.puissanceSignal = db85;
      texteProgmemAuNextion(valeurconnecte, texteNextionNonConnecte, orange); // champ, texte, couleur
    }
    else { // si conecte on affiche l'IP
      valeurconnecte.Set_font_color_pco(vert); // texte couleur vert
      valeurconnecte.setText(connectionWifi.adresseIP);// envoi le texte a l'ecran nextion
      DPRINT(connectionWifi.adresseIP);  DPRINTLN(); // debug
    }
  }
  else {
    connectionWifi.puissanceSignal = db85;
    texteProgmemAuNextion(valeurconnecte, texteNextionModuleNonDetecte, rouge); // champ, texte, couleur
    connectionWifi.connecter = false; // "non connecter au wifi" // passe en true si connecter au reseau wifi
    connectionWifi.presenceModule = false; // "non connecter au wifi" // passe en true si reponse du module
    etatWifi();
  }
  if (connectionWifi.puissanceSignal < db85) { // si signal >
    dbsignalwifi1.setValue(full); // palier de l'icone signal vide
  }
  if (connectionWifi.puissanceSignal < db80) {// si signal >
    dbsignalwifi2.setValue(full); // palier de l'icone signal vide
  }
  if (connectionWifi.puissanceSignal < db75) {// si signal >
    dbsignalwifi3.setValue(full); // palier de l'icone signal vide
  }
  if (connectionWifi.puissanceSignal < db70) {// si signal >
    dbsignalwifi4.setValue(full); // palier de l'icone signal vide
  }
  if (connectionWifi.puissanceSignal < db65) {// si signal >
    dbsignalwifi5.setValue(full); // palier de l'icone signal vide
  }
  DPRINTF("niveau signal Wifi : "); DPRINT(connectionWifi.puissanceSignal);  DPRINTLN(); // debug
}

void rafraichirSignalGSM() { // affiche le statut de connection et niveau du signal
  uint32_t compteurMillis;
  uint8_t db50 = 31;
  uint8_t db60 = 26;
  uint8_t db70 = 21;
  uint8_t db80 = 16;
  uint8_t db90 = 11;
  uint8_t full = 100; // 100% de la jauge
  signalGSM = 0;
  if (gsmPrintlnAndWaitATCommand(ATString, OKLongString, cinqMille, true)) {
    effaceBufferTexte();
    memset(nomOperateur, '\0', maxNomOperateur); // effacer buffer
    boolean timeOut = true;
    compteurMillis = millis();
    gsmPrintlnATCommand("AT+COPS?");
    while (millis() - compteurMillis < cinqMille ) {
      ecouterGSM();
      if (nomOperateurRecu()) {
        DPRINTF("operateur recu delais timer : ");  DPRINTLN(((float)millis() - compteurMillis) / 1000); // debug
        timeOut = false;
        texteAuNextion(valeuroperateur, nomOperateur, vert); // champ, texte, couleur
        break;
      }
    }
    if (timeOut) {
      texteProgmemAuNextion(valeuroperateur, texteNextionNonConnecte , orange); // champ, texte, couleur
    }
    else {
      compteurMillis = millis();
      gsmPrintlnATCommand("AT+CSQ");
      while (millis() - compteurMillis < cinqMille ) {
        ecouterGSM();
        if (signalGSMRecu()) {
          DPRINTF("puissance signal recu delais timer : ");  DPRINTLN(((float)millis() - compteurMillis) / 1000); // debug
          break;
        }
      }
    }
  }
  else {
    texteProgmemAuNextion(valeuroperateur, texteNextionModuleNonDetecte , rouge); // champ, texte, couleur
  }
  if (signalGSM > db90) { // si signal >
    dbsignalgsm1.setValue(full); // palier de l'icone signal vide
  }
  if (signalGSM > db80) {// si signal >
    dbsignalgsm2.setValue(full); // palier de l'icone signal vide
  }
  if (signalGSM > db70) {// si signal >
    dbsignalgsm3.setValue(full); // palier de l'icone signal vide
  }
  if (signalGSM > db60) {// si signal >
    dbsignalgsm4.setValue(full); // palier de l'icone signal vide
  }
  if (signalGSM > db50) {// si signal >
    dbsignalgsm5.setValue(full); // palier de l'icone signal vide
  }
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< STATUTS DES BOUTONS
/* regarde le statut des fonctions et affiche les bouton en consequence */

/* statut de l'alimentation 1, ON ou OFF */
void statutOnOffAlim1() {
  if (sauvegarde.nourrissageON[numeroUn] ) { // Si nourissage 1 ON
    onoffalimentation1.Set_background_image_pic(5); // affiche le bouton ON
    TIMEenTEXTE(sauvegarde.heureNourrissage[numeroUn]); // convertie numerique en char
    affichageheurealimentation1.setText(textime); // affichage de l'heure de la tempete Aleatoire
    DPRINTF("Nourrissage 1 ON");  DPRINTLN(); // debug //debug
  }
  else if (sauvegarde.nourrissageON[numeroUn] == false) { // si nourissage 1 OFF
    onoffalimentation1.Set_background_image_pic(6); // affiche le bouton OFF
    texteProgmemAuNextion(affichageheurealimentation1, vide , pas2Changement2Couleur); // champ, texte, couleur
    DPRINTF("Nourrissage 1 OFF");  DPRINTLN(); // debug //debug
  }
}

/* statut de l'alimentation 2, ON ou OFF */
void statutOnOffAlim2() {
  if (sauvegarde.nourrissageON[numeroDeux] ) { // si nourissage 2 ON
    onoffalimentation2.Set_background_image_pic(5); // affiche le bouton ON
    TIMEenTEXTE(sauvegarde.heureNourrissage[numeroDeux]); // convertie numerique en char
    affichageheurealimentation2.setText(textime); // affichage de l'heure de la tempete Aleatoire
    DPRINTF("Nourrissage 2 ON");  DPRINTLN(); // debug //debug
  }
  else if (sauvegarde.nourrissageON[numeroDeux] == false) { // si nourissage OFF
    onoffalimentation2.Set_background_image_pic(6); // affiche le bouton OFF
    texteProgmemAuNextion(affichageheurealimentation2, vide , pas2Changement2Couleur); // champ, texte, couleur
    DPRINTF("Nourrissage 2 OFF");  DPRINTLN(); // debug //debug
  }
}

/* statut nourrissage en cours */
void statutNourrissageEnCours() {
  if (nourissageEnCours) {
    afficherNourrissageEnCours(); // affiche le message a l'ecran
  }
}

/* statut pompe remonté */
void statutOnOffDelay() {
  if (sauvegarde.remonteDelay == Delay) {
    onoffpomperemonte.Set_background_image_pic(Idelay); // affiche le bouton delay
  }
  else if (sauvegarde.remonteDelay == Actif) {
    onoffpomperemonte.Set_background_image_pic(5); // affiche le bouton ON
  }
  else if (sauvegarde.remonteDelay == Arret) {
    onoffpomperemonte.Set_background_image_pic(6); // affiche le bouton OFF
  }
}

/* statut tempete aleatoire, ON ou OFF */
void statutTempeteAleatoire() {
  if (sauvegarde.tempeteAleatoireOn ) { // Si tempete aleatoir ON
    onofftempetealeatoire.Set_background_image_pic(5); // affiche bouton ON
    tempeteNextion();// ecrit heure et duree tout deux determiné aleatoirement
    DPRINTF("tempete aléatoir ON");  DPRINTLN(); // debug //debug
  }
  else if (sauvegarde.tempeteAleatoireOn == false) { // Si tempete aleatoir OFF
    onofftempetealeatoire.Set_background_image_pic(6); // affiche bouton OFF
    texteProgmemAuNextion(affichageheuretempete, vide , pas2Changement2Couleur); // champ, texte, couleur
    texteProgmemAuNextion(affichagedureetempete, vide , pas2Changement2Couleur); // champ, texte, couleur
    DPRINTF("tempete aléatoir OFF");  DPRINTLN(); // debug //debug
  }
}

/* statut eclairage blanc, ON, PWM ou OFF */
void statutBlanc() {
  if (sauvegarde.eclairageOnOffPwm[blanc1] == Actif) { // si ON
    onoffpwmblanc.Set_background_image_pic(Ion); // affiche bouton ON
    DPRINTF("blanc on");  DPRINTLN(); // debug //debug
  }
  else if (sauvegarde.eclairageOnOffPwm[blanc1] == Arret) { // si OFF
    onoffpwmblanc.Set_background_image_pic(Ioff); // affiche bouton OFF
    DPRINTF("blanc off");  DPRINTLN(); // debug //debug
  }
  else if (sauvegarde.eclairageOnOffPwm[blanc1] == Pwm) { // si pwm
    onoffpwmblanc.Set_background_image_pic(Ipwm); // affiche bouton PWM
    DPRINTF("blanc pwm");  DPRINTLN(); // debug //debug
  }
}

/* statut eclairage bleu, ON, PWM ou OFF */
void statutBleu() {
  if (sauvegarde.eclairageOnOffPwm[bleu1] == Actif) { // si ON
    onoffpwmbleu.Set_background_image_pic(Ion); // affiche bouton ON
    DPRINTF("bleu on");  DPRINTLN(); // debug //debug
  }
  else if (sauvegarde.eclairageOnOffPwm[bleu1] == Arret) { // si OFF
    onoffpwmbleu.Set_background_image_pic(Ioff); // affiche bouton OFF
    DPRINTF("bleu off");  DPRINTLN(); // debug //debug
  }
  else if (sauvegarde.eclairageOnOffPwm[bleu1] == Pwm) { // Si PWM
    onoffpwmbleu.Set_background_image_pic(Ipwm); // affiche bouton PWM
    DPRINTF("bleu pwm");  DPRINTLN(); // debug //debug
  }
}

/* statut brassage 1, ON, PWM ou OFF */
void statutBrassage1() {
  if (sauvegarde.brassageOnOffPwm[numeroUn] == Actif) { // Si ON
    onoffpwmbrassage1.Set_background_image_pic(Ion); // affiche bouton ON
    DPRINTF("brassage1 on");  DPRINTLN(); // debug //debug
  }
  else if (sauvegarde.brassageOnOffPwm[numeroUn] == Arret) { // si OFF
    onoffpwmbrassage1.Set_background_image_pic(Ioff); // affiche bouton OFF
    DPRINTF("brassage1 off");  DPRINTLN(); // debug //debug
  }
  else if (sauvegarde.brassageOnOffPwm[numeroUn] == Pwm) { // si PWM
    onoffpwmbrassage1.Set_background_image_pic(Ipwm); // affiche bouton PWM
    DPRINTF("brassage1 pwm");  DPRINTLN(); // debug //debug
  }
}

/* statut brassage 2, ON, PWM ou OFF */
void statutBrassage2() {
  if (sauvegarde.brassageOnOffPwm[numeroDeux] == Actif) { // si ON
    onoffpwmbrassage2.Set_background_image_pic(Ion); // affiche bouton ON
    DPRINTF("brassage2 on");  DPRINTLN(); // debug //debug
  }
  else if (sauvegarde.brassageOnOffPwm[numeroDeux] == Arret) { // si OFF
    onoffpwmbrassage2.Set_background_image_pic(Ioff); // affiche bouton OFF
    DPRINTF("brassage2 off");  DPRINTLN(); // debug //debug
  }
  else if (sauvegarde.brassageOnOffPwm[numeroDeux] == Pwm) { // si PWM
    onoffpwmbrassage2.Set_background_image_pic(Ipwm); // affiche bouton PWM
    DPRINTF("brassage2 pwm");  DPRINTLN(); // debug //debug
  }
}

/* statut brassage 3, ON, PWM ou OFF */
void statutBrassage3() {
  if (sauvegarde.brassageOnOffPwm[numeroTrois] == Actif) { // si ON
    onoffpwmbrassage3.Set_background_image_pic(Ion); // affiche bouton ON
    DPRINTF("brassage3 on");  DPRINTLN(); // debug //debug
  }
  else if (sauvegarde.brassageOnOffPwm[numeroTrois] == Arret) { // si OFF
    onoffpwmbrassage3.Set_background_image_pic(Ioff); // affiche bouton OFF
    DPRINTF("brassage3 off");  DPRINTLN(); // debug //debug
  }
  else if (sauvegarde.brassageOnOffPwm[numeroTrois] == Pwm) { // si PWM
    onoffpwmbrassage3.Set_background_image_pic(Ipwm); // affiche bouton PWM
    DPRINTF("brassage3 pwm");  DPRINTLN(); // debug //debug
  }
}

/* statut tempete manuel, ON ou OFF */
void statutTempete() {
  if (MajEcranTempeteEnCours ) { // si ON
    onofftempete.Set_background_image_pic(4); // affiche le bouton ON
    DPRINTF("tempete manuel on");  DPRINTLN(); // debug //debug
  }
  else if (MajEcranTempeteEnCours == false) { // si OFF
    onofftempete.Set_background_image_pic(ItempeteOff); // affiche bouton OFF
    DPRINTF("tempete manuel off");  DPRINTLN(); // debug //debug
  }
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< boutons et fonction associé
/* liste des boutons */
NexTouch *nex_listen_list[] = {
  // boutons page menu
  &onoffpomperemonte, &onoffalimentation1, &onoffalimentation2, &onoffpwmbrassage1, &onoffpwmbrassage2, &onoffpwmbrassage3,
  &onoffpwmblanc, &onoffpwmbleu, &boutonetalonnage, &onofftempete, &onofftempetealeatoire, &boutonalimentationmanuel, &boutonmenuversconfig, &pageMenu, &boutonversstandby,
  // boutons page config
  &boutonconfigverstel, &boutonconfigverswifi, &boutonconfigversmenu, &boutonconfigversinfo, &boutonconfigversbrassage, &boutonconfigvershorloge, &boutonenregistrerconfigversmenu,
  // boutons page wifi
  &boutonwifiversconfig, &boutonenregistrerwifiversconfig,
  // boutons page tel
  &boutontelversconfig, &boutonenregistrertelversconfig,
  // bouton page info
  &boutoninfoversconfig,
  // boutons page brassage
  &boutonbrassageversconfig, &boutonenregistrerbrassageversconfig, &boutonomoinsoscillo1min, &boutonplusoscillo1min, &boutonomoinsoscillo1max, &boutonplusoscillo1max,
  &boutonomoinsoscillo2min, &boutonplusoscillo2min, &boutonomoinsoscillo2max, &boutonplusoscillo2max, &boutonomoinsoscillo3min, &boutonplusoscillo3min, &boutonomoinsoscillo3max, &boutonplusoscillo3max,
  // boutons page horloge
  &boutonhorlogeversconfig, &boutonenregistrerhorlogeversconfig,
  // boutons page standby
  &pagestandby,
  NULL
};

/* Regarde si un bouton a été actionné, a mettre dans le loop */
void lectureBoutons() {
  nexLoop(nex_listen_list);
}

/* affiche la page demandé selon le bouton appuyer */
void versConfig(void *ptr) {
  affichePageConfig(); // affiche la page
}
void versInfo(void *ptr) {
  affichePageInfo(); // affiche la page
}
void versTel(void *ptr) {
  affichePageTel(); // affiche la page
}
void versWifi(void *ptr) {
  affichePageWifi(); // affiche la page
}
void versMenu(void *ptr) {
  affichePageMenu(); // affiche la page
}
void versBrassage(void *ptr) {
  affichePageBrassage(); // affiche la page
}
void versHorloge(void *ptr) {
  affichePageHorloge(); // affiche la page
}
void versStandby(void *ptr) {
  pagestandby.show();  // affiche la page standby
  pageActuelNextion = standby; // indique a l'arduino que la page actuel est standby
}

/* le bouton associé est appuyer */
void boutonOnOffRemonte(void *ptr) {
  variableetatboutons.getValue(&number); // lit le statut du bouton
  if (number == 26) { // Si DELAY
    digitalWrite(pinOutRelaisRemontee, LOW);// pompe remonté on
    sauvegarde.remonteDelay = Delay; // remonté ON, coupure de la remonté lors du nourrissage
    DPRINTF("remonté delais");  DPRINTLN(); // debug
  }
  else if (number == 27 ) { // si ON
    digitalWrite(pinOutRelaisRemontee, LOW);// pompe remonté on
    sauvegarde.remonteDelay = Actif; // remonté ON, pas de coupure de la remonté lors du nourrissage
    DPRINTF("remonté ON");  DPRINTLN(); // debug //debug
  }
  else if (number == 28) { // si OFF
    digitalWrite(pinOutRelaisRemontee, HIGH);// pompe remonté off
    sauvegarde.remonteDelay = Arret; // remonté OFF
    DPRINTF("remonté OFF");  DPRINTLN(); // debug
  }
}

/* le bouton associé est appuyer */
void boutonNourrissageManuel(void *ptr) {
  if (onOffalimentation ) { // si ON
    afficherNourrissageEnCours(); // affiche "nourissage en cours" sur l'ecran
    nourissageEnCours = true; // boolean pour ne pas avoir de nourrissage au demarrage
    tempsPrecedentNourrissage = millis(); // demarre le compteur de duree du nourissage
    if (sauvegarde.remonteDelay == Delay) { // si remonté en delay
      digitalWrite(pinOutRelaisRemontee, HIGH); // relais fermé = remonté OFF
      DPRINTF("pompe de remonté OFF");  DPRINTLN(); // debug //debug
    }
    onOffalimentation = !onOffalimentation; // change le statut du bouton pour la prochaine fois
    brassage(); // recalcul la puissance du brassage
    rafraichirBrassage1Nextion(); // MAJ du brassage sur l'ecran
    rafraichirBrassage2Nextion(); // MAJ du brassage sur l'ecran
    rafraichirBrassage3Nextion(); // MAJ du brassage sur l'ecran
    DPRINTF("nourrissage manuel ON");  DPRINTLN(); // debug //debug
  }
  else if (onOffalimentation == false) { //si OFF
    nourissageEnCours = false; // boolean pour ne pas avoir de nourrissage au demarrage
    afficherNourrissageEnCoursOff(); // arret le message 'nourissage en cours" sur l'ecran
    tempsPrecedentNourrissage = sauvegarde.dureeNourrissageMillis + 1; // stop le compteur de duree nourissage
    digitalWrite(pinOutRelaisRemontee, LOW); // relais ouvert = remonté ON
    onOffalimentation = !onOffalimentation; // change le statut du bouton pour la prochaine fois
    brassage(); // recalcul la puissance du brassage
    rafraichirBrassage1Nextion(); // MAJ du brassage sur l'ecran
    rafraichirBrassage2Nextion(); // MAJ du brassage sur l'ecran
    rafraichirBrassage3Nextion(); // MAJ du brassage sur l'ecran
  }
}

/* le bouton associé est appuyer */
void boutonOnOffAlim1(void *ptr) {
  variableetatboutons.getValue(&number); // lit le statut du bouton
  if (number == 3 ) { // si bouton ON
    sauvegarde.nourrissageON[numeroUn] = true; // indique ON nourissage 1
    TIMEenTEXTE(sauvegarde.heureNourrissage[numeroUn]); // convertie numerique en char
    affichageheurealimentation1.setText(textime); // affichage de l'heure de la tempete Aleatoire
    DPRINTF("nourrissage 1 ON");  DPRINTLN(); // debug //debug
  }
  else if (number == 4 ) { // si bouton OFF
    texteProgmemAuNextion(affichageheurealimentation1, vide , pas2Changement2Couleur); // champ, texte, couleur
    sauvegarde.nourrissageON[numeroUn] = false; // indique OFF nourissage 1
    nourissageEnCours = false; // boolean pour ne pas avoir de tempete au demarrage
    DPRINTF("nourrissage 1 OFF");  DPRINTLN(); // debug //debug
  }
}

/* le bouton associé est appuyer */
void boutonOnOffAlim2(void *ptr) {
  variableetatboutons.getValue(&number); // lit le statut du bouton
  if (number == 5) { // si bouton ON
    sauvegarde.nourrissageON[numeroDeux] = true;// indique ON nourissage 2
    TIMEenTEXTE(sauvegarde.heureNourrissage[numeroDeux]); // convertie numerique en char
    affichageheurealimentation2.setText(textime); // affichage de l'heure de la tempete Aleatoire
    DPRINTF("nourrissage 2 ON");  DPRINTLN(); // debug //debug
  }
  else if (number == 6) { // si bouton OFF
    texteProgmemAuNextion(affichageheurealimentation2, vide , pas2Changement2Couleur); // champ, texte, couleur
    sauvegarde.nourrissageON[numeroDeux] = false; // indique OFF nourissage 2
    nourissageEnCours = false; // boolean pour ne pas avoir de tempete au demarrage
    DPRINTF("nourrissage 2 OFF");  DPRINTLN(); // debug //debug
  }
}

/* le bouton associé est appuyer */
void boutonBlanc(void *ptr) {
  variableetatboutons.getValue(&number); // lit le statut du bouton
  if (number == 7) { // si pwm
    sauvegarde.eclairageOnOffPwm[blanc1] = Pwm;
    sauvegarde.eclairageOnOffPwm[blanc2] = Pwm;
    DPRINTF("blanc PWM");  DPRINTLN(); // debug //debug
  }
  else if (number == 8) { // si ON
    sauvegarde.eclairageOnOffPwm[blanc1] = Actif;
    sauvegarde.eclairageOnOffPwm[blanc2] = Actif;
    DPRINTF("blanc ON");  DPRINTLN(); // debug //debug
  }
  else if (number == 9) { // si OFF
    sauvegarde.eclairageOnOffPwm[blanc1] = Arret;
    sauvegarde.eclairageOnOffPwm[blanc2] = Arret;
    DPRINTF("blanc OFF");  DPRINTLN(); // debug //debug
  }
  eclairage();
  affichagepourcentblanc.setValue((variable.pwmEclairage[blanc1] + variable.pwmEclairage[blanc2]) / 2); // affiche % blanc
}

/* le bouton associé est appuyer */
void boutonBleu(void *ptr) {
  variableetatboutons.getValue(&number);  // lit le statut du bouton
  if (number == 10) { // si pwm
    sauvegarde.eclairageOnOffPwm[bleu1] = Pwm;
    sauvegarde.eclairageOnOffPwm[bleu2] = Pwm;
    DPRINTF("bleu PWM");  DPRINTLN(); // debug //debug
  }
  else if (number == 11) { // si On
    sauvegarde.eclairageOnOffPwm[bleu1] = Actif;
    sauvegarde.eclairageOnOffPwm[bleu2] = Actif;
    DPRINTF("bleu ON");  DPRINTLN(); // debug //debug
  }
  else if (number == 12) { // si Off
    sauvegarde.eclairageOnOffPwm[bleu1] = Arret;
    sauvegarde.eclairageOnOffPwm[bleu2] = Arret;
    DPRINTF("bleu OFF");  DPRINTLN(); // debug //debug
  }
  eclairage();
  affichagepourcentbleu.setValue((variable.pwmEclairage[bleu1] + variable.pwmEclairage[bleu2]) / 2); // affiche % bleu

}

/* le bouton associé est appuyer */
void boutonBrassage1(void *ptr) {
  variableetatboutons.getValue(&number); // lit le statut du bouton
  if (number == 13) { // si pwm
    sauvegarde.brassageOnOffPwm[numeroUn] = Pwm;
    DPRINTF("brassage 1 PWM");  DPRINTLN(); // debug //debug
  }
  else if (number == 14) { // si On
    sauvegarde.brassageOnOffPwm[numeroUn] = Actif;
    DPRINTF("brassage 1 ON");  DPRINTLN(); // debug //debug
  }
  else if (number == 15) { // si Off
    sauvegarde.brassageOnOffPwm[numeroUn] = Arret;
    DPRINTF("brassage 1 OFF");  DPRINTLN(); // debug //debug
  }
  brassage();
  rafraichirBrassage1Nextion();
  affichagepourcentbrassage1.setValue(variable.pwmBrassage[numeroUn]);
}

/* le bouton associé est appuyer */
void boutonBrassage2(void *ptr) {
  variableetatboutons.getValue(&number); // lit le statut du bouton
  if (number == 16) { // si pwm
    sauvegarde.brassageOnOffPwm[numeroDeux] = Pwm;
    DPRINTF("brassage 2 PWM");  DPRINTLN(); // debug //debug
  }
  else if (number == 17) { // si On
    sauvegarde.brassageOnOffPwm[numeroDeux] = Actif;
    DPRINTF("brassage 2 ON");  DPRINTLN(); // debug //debug
  }
  else if (number == 18) { // si Off
    sauvegarde.brassageOnOffPwm[numeroDeux] = Arret;
    DPRINTF("brassage 2 OFF");  DPRINTLN(); // debug //debug
  }
  brassage();
  rafraichirBrassage2Nextion();
  affichagepourcentbrassage2.setValue(variable.pwmBrassage[numeroDeux]);

}

/* le bouton associé est appuyer */
void boutonBrassage3(void *ptr) {
  variableetatboutons.getValue(&number);  // lit le statut du bouton
  if (number == 19) { // si pwm
    sauvegarde.brassageOnOffPwm[numeroTrois] = Pwm;
    DPRINTF("brassage 3 PWM");  DPRINTLN(); // debug //debug
  }
  else if (number == 20) { // si On
    sauvegarde.brassageOnOffPwm[numeroTrois] = Actif;
    DPRINTF("brassage 3 ON");  DPRINTLN(); // debug //debug
  }
  else if (number == 21) { // si off
    sauvegarde.brassageOnOffPwm[numeroTrois] = Arret;
    DPRINTF("brassage 3 OFF");  DPRINTLN(); // debug //debug
  }
  brassage();
  rafraichirBrassage3Nextion();
  affichagepourcentbrassage3.setValue(variable.pwmBrassage[numeroTrois]);

}

/* bouton etalonage PH */
void etalonnage(void *ptr) {
  initialisationPhDemarrage = false;
  mesurePhDemarrage = false;
  variableetatboutons.getValue(&number); // lit la valeur du bouton
  if (number == 30 ) { // si OFF
    texteProgmemAuNextion(messageetalonnage, vide , pas2Changement2Couleur); // champ, texte, couleur
    etalonnageOnOFF = false; // met en OFF
    alertePhBac();
  }
  else if (number == 29) { // Si ON
    texteProgmemAuNextion(messageetalonnage, texteNextionetalonnage , pas2Changement2Couleur); // champ, texte, couleur
    etalonnageOnOFF = true; // met en ON
    afficherAlertepPhBacOff();
  }
}

/* déclenche une tempete manuelement */
void boutonTempeteManuel(void *ptr) {
  variableetatboutons.getValue(&number); // lit la valeur du bouton
  if (number == 22) { // si ON
    tempsPrecedentTempete = millis(); // demarre le compteur
    tempeteManuelEnCours = true;
    DPRINTF("tempete manuel déclanché");  DPRINTLN(); // debug //debug
  }
  else if (number == 25) { // si OFF
    tempeteManuelEnCours = false; // tempete OFF
    //dureeTempeteAleatoire = 0; // stop si tempete automatique
    DPRINTF("tempete arrété manuelement");  DPRINTLN(); // debug //debug
    tempeteAleatoireEnCours = false; // tempette aleatoir OFF si etait en ON
  }
  brassage();  // lance la fonction pour affecter les nouveau parametres
  rafraichirBrassage1Nextion(); // MAJ de l'ecran
  rafraichirBrassage2Nextion(); // MAJ de l'ecran
  rafraichirBrassage3Nextion(); // MAJ de l'ecran
}

/* bouton active la tempete Aleatoire */
void boutonTempeteAleatoire(void *ptr) {
  variableetatboutons.getValue(&number); // lit la valeur du bouton
  if (number == 23) { // si ON
    sauvegarde.tempeteAleatoireOn = true; // tempete aleatoir ON
    definiTempeteAleatoire(); // defini horraire et duree de la tempette aleatoir
    tempeteNextion(); // ecrit l'heure et la duree de la tempete aleatoir
    DPRINTF("tempete aléatoir ON");  DPRINTLN(); // debug
  }
  else if (number == 24) { // si OFF
    sauvegarde.tempeteAleatoireOn = false; // tempete aleatoir OFF
    dureeTempeteAleatoire = 0; // remise a zero de la duree de tempete aleatoir
    tempeteAleatoireEnCours = false; // tempete aleatoir en cours OFF
    texteProgmemAuNextion(affichageheuretempete, vide , pas2Changement2Couleur); // champ, texte, couleur
    texteProgmemAuNextion(affichagedureetempete, vide , pas2Changement2Couleur); // champ, texte, couleur
    brassage(); // lance la fonction pour affecter les nouveau parametres
    statutTempete(); // lance la fonction pour affecter les nouveau parametres
    DPRINTF("tempete aléatoir OFF");  DPRINTLN(); // debug
  }
}

/* lit la valeur du bouton et l'affiche a l'ecran puis deplace l'oscillateur a cette valeur */
void boutonoscillo1min(void *ptr) {
  valeuroscillo1min.getValue(&number); // recupere valeur
  affichagedegreoscillo1.setValue(number); // affiche la valeur
  boutonoscillo1(); // deplace l'oscillateur
}

/* lit la valeur du bouton et l'affiche a l'ecran puis deplace l'oscillateur a cette valeur */
void boutonoscillo1max(void *ptr) {
  valeuroscillo1max.getValue(&number); // recupere valeur
  affichagedegreoscillo1.setValue(number); // affiche la valeur
  boutonoscillo1(); // deplace l'oscillateur
}

/* lit la valeur du bouton et l'affiche a l'ecran puis deplace l'oscillateur a cette valeur */
void boutonoscillo2min(void *ptr) {
  valeuroscillo2min.getValue(&number); // recupere valeur
  affichagedegreoscillo2.setValue(number); // affiche la valeur
  boutonoscillo2(); // deplace l'oscillateur
}

/* lit la valeur du bouton et l'affiche a l'ecran puis deplace l'oscillateur a cette valeur */
void boutonoscillo2max(void *ptr) {
  valeuroscillo2max.getValue(&number); // recupere valeur
  affichagedegreoscillo2.setValue(number); // affiche la valeur
  boutonoscillo2(); // deplace l'oscillateur
}

/* lit la valeur du bouton et l'affiche a l'ecran puis deplace l'oscillateur a cette valeur */
void boutonoscillo3min(void *ptr) {
  valeuroscillo3min.getValue(&number); // recupere valeur
  affichagedegreoscillo3.setValue(number); // affiche la valeur
  boutonoscillo3(); // deplace l'oscillateur
}

/* lit la valeur du bouton et l'affiche a l'ecran puis deplace l'oscillateur a cette valeur */
void boutonoscillo3max(void *ptr) {
  valeuroscillo3max.getValue(&number); // recupere valeur
  affichagedegreoscillo3.setValue(number); // affiche la valeur
  boutonoscillo3(); // deplace l'oscillateur
}

/* deplace l'oscillateur a l'angle parametré */
void boutonoscillo1() {
  positionOscillo[numeroUn] = number;
  oscillo[numeroUn].write(positionOscillo[numeroUn]);
}

/* deplace l'oscillateur a l'angle parametré */
void boutonoscillo2() {
  positionOscillo[numeroDeux] = number;
  oscillo[numeroDeux].write(positionOscillo[numeroDeux]);
}

/* deplace l'oscillateur a l'angle parametré */
void boutonoscillo3() {
  positionOscillo[numeroTrois] = number;
  oscillo[numeroTrois].write(positionOscillo[numeroTrois]);
}

/*sauvegarde les parametres de la page config */
void enregistrerConfig(void *ptr) {
  valeurblanc.getValue(&number); // recupere la valeur dans nextion
  sauvegarde.puissanceMaxEclairage[blanc1] = number; // attribut la valeur nextion dans l'arduino
  sauvegarde.puissanceMaxEclairage[blanc2] = sauvegarde.puissanceMaxEclairage[blanc1];
  valeurbleu.getValue(&number);// recupere la valeur dans nextion
  sauvegarde.puissanceMaxEclairage[bleu1] = number;// attribut la valeur nextion dans l'arduino
  sauvegarde.puissanceMaxEclairage[bleu2] = sauvegarde.puissanceMaxEclairage[bleu1];
  effaceminuteEnTexteIn(); // efface memoire
  valeurleverSoleilh.getValue(&number);// recupere la valeur dans nextion
  valeurleverSoleilm.getText(minuteEnTexteIn, maxminuteEnTexte); // recupere la valeur dans nextion
  horraireEnSeconde(number, minuteEnTexteIn); // calcule h*3600+m*60
  sauvegarde.leverSoleil = recupHorraireTemporraire;// attribut la valeur nextion dans l'arduino
  effaceminuteEnTexteIn();// efface memoire
  valeurcoucherSoleilh.getValue(&number);// recupere la valeur dans
  valeurcoucherSoleilm.getText(minuteEnTexteIn, maxminuteEnTexte); // recupere la valeur dans nextion
  horraireEnSeconde(number, minuteEnTexteIn);// calcule h*3600+m*60
  sauvegarde.coucherSoleil = recupHorraireTemporraire;// attribut la valeur nextion dans l'arduino
  valeurdureelevercoucher.getValue(&number);// recupere la valeur dans nextion
  sauvegarde.dureelevercoucher = number * minuteEnSeconde; // attribut la valeur nextion dans l'arduino
  valeuralertetemperaturebasse.getValue(&number);// recupere la valeur dans nextion
  sauvegarde.alertetemperaturebasse = number;// attribut la valeur nextion dans l'arduino
  sauvegarde.alertetemperaturebasse = sauvegarde.alertetemperaturebasse / dix;
  valeuralertetemperaturehaute.getValue(&number);// recupere la valeur dans nextion
  sauvegarde.alertetemperaturehaute = number;// attribut la valeur nextion dans l'arduino
  sauvegarde.alertetemperaturehaute = sauvegarde.alertetemperaturehaute / dix;
  valeurventilationaquarium.getValue(&number);// recupere la valeur dans nextion
  sauvegarde.ventilationaquarium = number;// attribut la valeur nextion dans l'arduino
  sauvegarde.ventilationaquarium = sauvegarde.ventilationaquarium / dix;
  valeurventilationrampe.getValue(&number);// recupere la valeur dans nextion
  sauvegarde.ventilationrampe = number;// attribut la valeur nextion dans l'arduino
  adressesonde.getValue(&number);// recupere la valeur dans le nextion
  if (number == 1) {
    sauvegarde.adresseSondeRampe = 1;// attribut la valeur nextion dans l'arduino
    sauvegarde.adresseSondeBac = 0;// attribut la valeur nextion dans l'arduino
  }
  else {
    sauvegarde.adresseSondeRampe = 0;// attribut la valeur nextion dans l'arduino
    sauvegarde.adresseSondeBac = 1;// attribut la valeur nextion dans l'arduino
  }
  temperature(); // lance la fonction
  effaceminuteEnTexteIn();// efface memoire
  valeurheureNourrissage1h.getValue(&number);// recupere la valeur dans nextion
  valeurheureNourrissage1m.getText(minuteEnTexteIn, maxminuteEnTexte); // recupere la valeur dans nextion
  horraireEnSeconde(number, minuteEnTexteIn);// calcule h*3600+m*60
  sauvegarde.heureNourrissage[numeroUn] = recupHorraireTemporraire;// attribut la valeur nextion dans l'arduino
  effaceminuteEnTexteIn();// efface memoire
  valeurheureNourrissage2h.getValue(&number);// recupere la valeur dans nextion
  valeurheureNourrissage2m.getText(minuteEnTexteIn, maxminuteEnTexte); // recupere la valeur dans nextion
  horraireEnSeconde(number, minuteEnTexteIn);// calcule h*3600+m*60
  sauvegarde.heureNourrissage[numeroDeux] = recupHorraireTemporraire;// attribut la valeur nextion dans l'arduino
  valeurdureeNourrissageMillis.getValue(&number);// recupere la valeur dans nextion
  sauvegarde.dureeNourrissageMillis = number;// attribut la valeur nextion dans l'arduino
  sauvegarde.dureeNourrissageMillis = sauvegarde.dureeNourrissageMillis * 60000ul;
  valeurdureeosmolation.getValue(&number);// recupere la valeur dans nextion
  sauvegarde.dureeOsmolationMillis = number;// attribut la valeur nextion dans l'arduino
  sauvegarde.dureeOsmolationMillis = sauvegarde.dureeOsmolationMillis * mille;
  valeuralerteosmolation.getValue(&number);// recupere la valeur dans nextion
  sauvegarde.compteurOsmolationMax = number; // attribut la valeur nextion dans l'arduino
  valeurconsignePhRac.getValue(&number);// recupere la valeur dans nextion
  sauvegarde.consignePhRac = number;// attribut la valeur nextion dans l'arduino
  valeuralertephbacbas.getValue(&number);// recupere la valeur dans nextion
  sauvegarde.alertephbacbas = number;// attribut la valeur nextion dans l'arduino
  valeuralertephbachaut.getValue(&number);// recupere la valeur dans nextion
  sauvegarde.alertephbachaut = number;// attribut la valeur nextion dans l'arduino
  EEPROM.put(adresseDeSauvegarde, sauvegarde);  // sauvegarde tout les parametres
  majValeursParDefaut(); // recalcule les nouvelles valeurs
  eclairage(); // lance la fonction
  affichePageMenu();  // affiche la page menu
}

/*sauvegarde les parametres de la page wifi */
void enregistrerWifi(void *ptr) {
  effaceBufferTexte();
  valeurssid.getText(bufferTexte, maxbufferTexte); // recupere la valeur dans nextion
  strncpy(identifiantWifi.ssid, bufferTexte, maxSsid); // copie le char
  ecritSurSd(ssidSurSD, identifiantWifi.ssid); // copie sur la SD
  effaceBufferTexte(); // affiche la page configue
  valeurmdp.getText(bufferTexte, maxbufferTexte); // recupere la valeur dans nextion
  strncpy(identifiantWifi.mdp, bufferTexte, maxMdp); // copie le char
  ecritSurSd(mdpSurSD, identifiantWifi.mdp); // copie sur la SD
  executer(ENVOYERidentifiantWifi, true); // envoie les nouveau identifiants de connection
  affichePageConfig(); // affiche la page configue
}

/*sauvegarde les parametres de la page tel */
void enregistrerTel(void *ptr) {
  effaceBufferTexte();
  valeurtel.getText(bufferTexte, maxbufferTexte); // recupere la valeur dans nextion
  strncpy(numero2tel, bufferTexte, maxnumero2tel); // copie le char
  numero2tel[maxnumero2tel] = '\0'; // ajoute le caractere de fin
  ecritSurSd(numero2TelSurSD, numero2tel); // copie sur la SD
  affichePageConfig(); // affiche la page configue
}

/* sauvegarde les parametres de la page brassage */
void enregistrerBrassage(void *ptr) {
  DPRINTF("*************************************** sauvegarde brassage ******************************************");
  valeurbrassage1.getValue(&number); // recupere la valeur dans nextion
  sauvegarde.puissanceMaxBrassage[numeroUn] = number; // attribut la valeur nextion dans l'arduino
  valeurbrassage2.getValue(&number); // recupere la valeur dans nextion
  sauvegarde.puissanceMaxBrassage[numeroDeux] = number; // attribut la valeur nextion dans l'arduino
  valeurbrassage3.getValue(&number); // recupere la valeur dans nextion
  sauvegarde.puissanceMaxBrassage[numeroTrois] = number; // attribut la valeur nextion dans l'arduino
  valeuroscillo1min.getValue(&number); // recupere la valeur dans nextion
  sauvegarde.angle1Oscillo[numeroUn] = number; // attribut la valeur nextion dans l'arduino
  valeuroscillo2min.getValue(&number); // recupere la valeur dans nextion
  sauvegarde.angle1Oscillo[numeroDeux] = number; // attribut la valeur nextion dans l'arduino
  valeuroscillo3min.getValue(&number); // recupere la valeur dans nextion
  sauvegarde.angle1Oscillo[numeroTrois] = number; // attribut la valeur nextion dans l'arduino
  valeuroscillo1max.getValue(&number); // recupere la valeur dans nextion
  sauvegarde.angle2Oscillo[numeroUn] = number; // attribut la valeur nextion dans l'arduino
  valeuroscillo2max.getValue(&number); // recupere la valeur dans nextion
  sauvegarde.angle2Oscillo[numeroDeux] = number; // attribut la valeur nextion dans l'arduino
  valeuroscillo3max.getValue(&number); // recupere la valeur dans nextion
  sauvegarde.angle2Oscillo[numeroTrois] = number; // attribut la valeur nextion dans l'arduino
  valeurmouvementoscillo1.getValue(&number); // recupere la valeur dans nextion
  sauvegarde.delaisMouvementOscilloMillis[numeroUn] = (number * mille); // attribut la valeur nextion dans l'arduino
  valeurmouvementoscillo2.getValue(&number); // recupere la valeur dans nextion
  sauvegarde.delaisMouvementOscilloMillis[numeroDeux] = (number * mille); // attribut la valeur nextion dans l'arduino
  valeurmouvementoscillo3.getValue(&number); // recupere la valeur dans nextion
  sauvegarde.delaisMouvementOscilloMillis[numeroTrois] = (number * mille); // attribut la valeur nextion dans l'arduino
  valeurdureetempete.getValue(&number); // recupere la valeur dans nextion
  sauvegarde.dureeTempeteMillis = (number * minuteEnMillis); // attribut la valeur nextion dans l'arduino
  valeurtempete.getValue(&number); // recupere la valeur dans nextion
  sauvegarde.puissanceTempete = number; // attribut la valeur nextion dans l'arduino
  valeuraccalemienocturne.getValue(&number); // recupere la valeur dans nextion
  sauvegarde.accalmieNocturne = number; // attribut la valeur nextion dans l'arduino
  EEPROM.put(adresseDeSauvegarde, sauvegarde); // sauvegarde tout les parametres
  majValeursParDefaut(); // recalcule les nouvelles valeurs
  oscillateur(); // lance la fonction pour affecter les nouveau parametres
  brassage(); // lance la fonction pour affecter les nouveau parametres
  affichePageConfig(); // affiche la page config
}

/* sauvegarde les parametres de la page horloge */
void enregistrerHorloge(void *ptr) {
  uint8_t adjustHeure, adjustminut, adjustjour, adjustmois, stock;
  uint16_t adjustannee;
  effaceBufferTexte();
  affichageH.getText(bufferTexte, maxbufferTexte); // recupere la valeur dans l'ecran
  adjustHeure = atoi(bufferTexte); // convertie le char en numerique
  effaceBufferTexte();
  affichageM.getText(bufferTexte, maxbufferTexte); // recupere la valeur dans l'ecran;
  adjustminut = atoi(bufferTexte); // convertie le char en numerique
  effaceBufferTexte();
  affichagejour.getText(bufferTexte, maxbufferTexte); // recupere la valeur dans l'ecran
  adjustjour = atoi(bufferTexte); // convertie le char en numerique
  effaceBufferTexte();
  affichagemois.getText(bufferTexte, maxbufferTexte); // recupere la valeur dans l'ecran
  adjustmois = atoi(bufferTexte); // convertie le char en numerique
  effaceBufferTexte();
  affichageannee.getText(bufferTexte, maxbufferTexte); // recupere la valeur dans l'ecran
  stock = atoi(bufferTexte); // convertie le char en numerique
  adjustannee = stock + deuxMille; // ajoute deuxMille
  rtc.adjust(DateTime(adjustannee, adjustmois, adjustjour, adjustHeure, adjustminut, cinquante)); // met a l'heure l'horloge
  effaceBufferTexte();
  affichageHdebutalerte.getText(bufferTexte, maxbufferTexte); // recupere la valeur dans l'ecran
  stock = atoi(bufferTexte); // converti char en numerique
  horraireEnSeconde(stock, 0); // converti en seconde
  sauvegarde.heureDebutAlerte = recupHorraireTemporraire;
  affichageHfinalerte.getText(bufferTexte, maxbufferTexte); // recupere la valeur dans l'ecran
  stock = atoi(bufferTexte); // converti char en numerique
  horraireEnSeconde(stock, 0); // converti en secoinde
  sauvegarde.heureFinAlerte = recupHorraireTemporraire;
  EEPROM.put(adresseDeSauvegarde, sauvegarde); // sauvegarde tout les parametres
  oscillateur(); // lance la fonction pour affecter les nouveau parametres
  brassage(); // lance la fonction pour affecter les nouveau parametres
  eclairage(); // lance la fonction
  affichePageConfig(); // affiche la page config
}

/* affiche heure et duree de la tempete Aleatoire */
void tempeteNextion() {
  TIMEenTEXTE(heureTempeteAleatoire); // convertie numerique en char
  affichageheuretempete.setText(textime); // affichage de l'heure de la tempete Aleatoire
  TIMEenTEXTE(dureeTempeteAleatoire / mille); // convertie la duree de la tempete en char
  affichagedureetempete.setText(textime); // affichage de la durée de la tempete
}

/* affiche l'heure sur l'ecran */
void affichageHeureNextion() {
  TIMEenTEXTE(Time); // convertie numerique en char
  affichageheure.setText(textime); // affiche l'heure
}

/* converti numerique en char */
void numEnChar (uint8_t numeriqueEnChar) {
  const uint8_t maxTampon = 2;
  char tampon[maxTampon + 1];
  memset(tampon, '\0', maxminuteEnTexte); // effacer buffer
  memset(minuteEnTexteOut, '\0', maxminuteEnTexte); // effacer buffer
  if (numeriqueEnChar < dix) { // si inferieur a 10
    strcat(minuteEnTexteOut, "0"); // ajoute 0
  }
  itoa (numeriqueEnChar, tampon, 10); // convertie numerique en char
  DPRINTF("tampon"); DPRINT(tampon); DPRINTLN(); // debug
  strcat(minuteEnTexteOut, tampon); // copie le char
  DPRINTF("minuteEnTexteOut"); DPRINT(minuteEnTexteOut); DPRINTLN(); // debug
}

/* initialise la chaine à vide */
void effaceBufferTexte() {
  memset(bufferTexte, '\0', maxbufferTexte); // effacer buffer
}

/* initialise la chaine à vide */
void effaceminuteEnTexteIn() {
  memset(minuteEnTexteIn, '\0', maxminuteEnTexte); // effacer buffer
}

/* affichage du texte defillant pendant l'alimentation */
void afficherNourrissageEnCours() {
  effaceBufferTexte();
  strncpy_P (bufferTexte, texteNextionNourissage, maxbufferTexte); // recupere "char" en memoire flash et le copie
  bufferTexte[maxbufferTexte] = '\0'; // ajoute le caractere de fin
  messagedefilantalimentationencours.setText(bufferTexte);// envoi char a l'ecran nextion // max 30 caracteres
  texteNourrissage = true;
}

/* arret du texte defillant de l'alimentation */
void afficherNourrissageEnCoursOff() {
  effaceBufferTexte();
  strncpy_P (bufferTexte, vide, maxbufferTexte); // recupere "char" en memoire flash et le copie
  bufferTexte[maxbufferTexte] = '\0'; // ajoute le caractere de fin
  messagedefilantalimentationencours.setText(bufferTexte);// envoi char a l'ecran nextion // max 30 caracteres
  rafraichirBrassage1Nextion();
  rafraichirBrassage2Nextion();
  rafraichirBrassage3Nextion();
}

/* affichage du texte defillant quand trop de declanchement de l'osmolateur  */
void afficherAlerteOsmolation() {
  if (smsAlerteOsmolation == false) {
    texteProgmemAuNextion( messageanomalie2, texteNextionOsmolationOff, pas2Changement2Couleur); // champ, texte, couleur
  }
}

/* arret du texte alerte osmolation */
void afficherAlerteOsmolationOff() {
  texteProgmemAuNextion(messageanomalie2, vide, pas2Changement2Couleur); // champ, texte, couleur
}

/* arret du texte alerte reserve vide */
void afficherAlerteOsmolationOffReserveVide() {
  if (smsAlerteReserve == false) {
    texteProgmemAuNextion(messageanomalie2, texteNextionReserveVide  , pas2Changement2Couleur); // champ, texte, couleur
  }
}

/* affichage du texte defillant quand ph bac trop bas */
void afficherAlertepPhBacBas() {
  if (alertePhBas == false) {
    texteProgmemAuNextion( messageanomalie3, texteNextionPhBas, pas2Changement2Couleur); // champ, texte, couleur
  }
}

/* affichage du texte defillant quand ph bac trop haut*/
void afficherAlertepPhBacHaut() {
  if (alertePhHaut == false) {
    texteProgmemAuNextion( messageanomalie3, texteNextionPhHaut, pas2Changement2Couleur); // champ, texte, couleur
  }
}

/* arret du texte defillant alerte ph */
void afficherAlertepPhBacOff() {
  texteProgmemAuNextion( messageanomalie3, vide, pas2Changement2Couleur); // champ, texte, couleur
  alertePhBas = true;
  alertePhHaut = true;
}

void etalonnageOFF() { // met en OFF l'etalonage
  texteProgmemAuNextion( messageanomalie3, vide, pas2Changement2Couleur); // champ, texte, couleur
  messageetalonnage.setText(bufferTexte);// envoi char a l'ecran nextion
  etalonnageOnOFF = false; // met en OFF
}

void texteProgmemAuNextion(NexText champTexte, const char texteEnProgmem[] PROGMEM, const uint16_t couleurTexte) {
  effaceBufferTexte();
  if (couleurTexte > pas2Changement2Couleur) {
    champTexte.Set_font_color_pco(couleurTexte); // text couleur
  }
  strncpy_P (bufferTexte, texteEnProgmem, maxbufferTexte); // recupere "char" en memoire flash et le copie
  bufferTexte[maxbufferTexte] = '\0'; // ajoute le caractere de fin
  champTexte.setText(bufferTexte);// envoi le texte a l'ecran nextion// envoi le texte a l'ecran nextion
  DPRINT(bufferTexte); DPRINTLN(); // debug
}

void texteAuNextion(NexText champTexte, char* texte, const uint16_t couleurTexte) {
  if (couleurTexte > pas2Changement2Couleur) {
    champTexte.Set_font_color_pco(couleurTexte); // text couleur
  }
  champTexte.setText(texte);// envoi le texte a l'ecran nextion// envoi le texte a l'ecran nextion
  DPRINT(texte); DPRINTLN(); // debug
}
