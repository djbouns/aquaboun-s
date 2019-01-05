
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
#include "SdFat.h"

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ Fichiers $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
#include "global.h"
#include "debug.h"
#include "affichage.h"
#include "wifi.h"
#include "carteSD.h"

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ Déclarations $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
SdFat SD;
File myFile;
const char* ssidSurSD = "ssid.txt";
const char* mdpSurSD = "mdp.txt";
const char* numero2TelSurSD = "numero2tel.txt";
const char* erreursSurSD = "erreur.txt";

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ Fonctions $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
/* écrit sur la carte SD */
void ecritSurSd(const char * nomDuFichierTxt, char * donneeAEcrireSurSD) {
  if (strcmp (nomDuFichierTxt, erreursSurSD) != 0) { // si il ne s'agit pas du fichier erreur
    DPRINTF("Pas le fichier erreur.txt = on supprime l'ancien fichier : "); DPRINTLN(nomDuFichierTxt); DPRINTLN();
    SD.remove(nomDuFichierTxt); // suprime l'ancien fichier
    myFile = SD.open(nomDuFichierTxt, O_WRITE | O_CREAT);
    DPRINTF("Ouverture du fichier : "); DPRINTLN(nomDuFichierTxt);
  }
  else {
    DPRINTF("fichier erreur.txt"); DPRINTLN();
    myFile = SD.open(nomDuFichierTxt, O_WRITE | O_CREAT | O_AT_END);
    DPRINTF("Ouverture du fichier : "); DPRINTLN(nomDuFichierTxt);
    myFile.println(); // on passe a la ligne suivante
  }
  if (myFile) {
    myFile.print(donneeAEcrireSurSD); // ecrit dans le fichier
    DPRINTF("Ecriture dans le fichier : "); DPRINTLN(nomDuFichierTxt);
    DPRINT(donneeAEcrireSurSD);
    DPRINTF(", a bien été écrit dans le fichier "); DPRINTLN(nomDuFichierTxt); DPRINTLN();
  } else {
    DPRINTF("Erreur d'écriture dans le fichier "); DPRINTLN();
  }
  myFile.close(); // ferme le fichier
}

/* lis sur la carte SD */
boolean lisSurSd(const char * nomDuFichierTxt, char *donneeLueSurSD, size_t indexEcritureMAX) {
  boolean lectureOK = false; // retourne l'etat de lecture
  DPRINTF("Ouverture de : "); DPRINTLN(nomDuFichierTxt);//debug
  myFile = SD.open(nomDuFichierTxt, O_READ); // ouvre le fichier
  if (myFile) {
    uint8_t i = zero;
    donneeLueSurSD[0] = '\0'; // on initialise la chaine à vide
    while (myFile.available()) { // on lit l'intégralité du fichier, à concurrence de l'espace dispo dans notre buffer
      if (i < indexEcritureMAX) {
        donneeLueSurSD[i] = myFile.read(); // lit le fichier
        i++;
      } else {
        donneeLueSurSD[indexEcritureMAX] = '\0'; // ajoute caractere de fin
        break;
      }
    }
    donneeLueSurSD[i] = '\0';  // il n'y a plus rien à lire dans le fichier, on termine
    lectureOK = true; // la lecture c'est bien passé
    myFile.close(); // ferme le fichier
    effaceBufferTexte(); // effac e le buffer
    strncpy_P (bufferTexte, texteNextion26, maxbufferTexte); // recupere "char" en memoire flash et le copie
    bufferTexte[maxbufferTexte] = '\0'; // ajoute le caractere de fin
    if (strlen(bufferTexte) + strlen(nomDuFichierTxt) < maxbufferTexte) {
      strcat(bufferTexte, nomDuFichierTxt); // ajoute a la chaine de caractere
      if (strlen(bufferTexte) + strlen(nomDuFichierTxt) < maxbufferTexte) {
        strcat(bufferTexte, ", OK"); // ajoute a la chaine de caractere
        if (pageActuelNextion == demarrage) { // si l'ecran est sur la page de demarrage
          texte4.Set_font_color_pco(vert); // texte couleur vert
          texte4.setText(bufferTexte);// envoi char a l'ecran nextion// envoi char a l'ecran nextion
          delay(mille);
        }
        else {
          effaceBufferTexte();
          strncpy_P (bufferTexte, texteNextion27, maxbufferTexte); // recupere "char" en memoire flash et le copie
          bufferTexte[maxbufferTexte] = '\0'; // ajoute le caractere de fin
          strcat(bufferTexte, nomDuFichierTxt); // ajoute a la chaine de caractere
          if (pageActuelNextion == demarrage) { // si l'ecran est sur la page de demarrage
            texte4.Set_font_color_pco(rouge); // text couleur rouge
            texte4.setText(bufferTexte);// envoi char a l'ecran nextion// envoi char a l'ecran nextion
          }
          DPRINT(bufferTexte); DPRINTLN(); // debug
          delay(mille);
        }
        DPRINT(bufferTexte); DPRINTLN(); // debug
      }
      else {
        DPRINTF("probleme de place memoire dans lisSurSd niveau 2");  DPRINTLN(); // debug
      }
    }
    else {
      DPRINTF("probleme de place memoire dans lisSurSd niveau 1");  DPRINTLN(); // debug
    }
  }
  return lectureOK;
}

void horodatageEtEcritSurSD (const char Erreur1AAjouter[] PROGMEM, const char* Erreur2AAjouter) {
  uint8_t maxstock = 50;
  char stock [maxstock + 1];
  effaceBufferTexte();
  itoa (jour, stock, 10);
  strcat(bufferTexte, stock);
  strcat(bufferTexte, "/");
  itoa (mois, stock, 10);
  strcat(bufferTexte, stock);
  strcat(bufferTexte, "/");
  itoa (annee, stock, 10);
  strcat(bufferTexte, stock);
  strcat(bufferTexte, " a ");
  itoa (variable.Heure, stock, 10);
  strcat(bufferTexte, stock);
  strcat(bufferTexte, "H");
  itoa (variable.minut, stock, 10);
  strcat(bufferTexte, stock);
  strcat(bufferTexte, "s");
  itoa (variable.seconde, stock, 10);
  strcat(bufferTexte, stock);
  strcat(bufferTexte, " = ");
  strncpy_P (stock, Erreur1AAjouter, maxstock); // recupere "char" en memoire flash et le copie
  stock[maxstock] = '\0';
  strcat(bufferTexte, stock);
  strcat(bufferTexte, Erreur2AAjouter);
  ecritSurSd(erreursSurSD, bufferTexte);
}
