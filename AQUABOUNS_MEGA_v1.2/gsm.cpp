
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
#include <avr/pgmspace.h>

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ Fichiers $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
#include "gsm.h"
#include "global.h"
#include "pin.h"
#include "debug.h"
#include "autres.h"
#include "affichage.h"
#include "wifi.h"
#include "carteSD.h"

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ Déclarations $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
const uint8_t maxnumero2tel = 15;
char numero2tel [maxnumero2tel + 1];
const uint8_t ctrlZ = 0x1A;
const char * ctrlZString = "\x1A";
const char * escapeString = "\x1B";
const char * ATString = "AT";
const char * OKShortString = "OK";
const char * OKLongString = "\r\nOK\r\n";
const uint8_t maxMessageSize = 100;
char messageGsm[maxMessageSize + 1];
const uint8_t maxNomOperateur = 20;
char nomOperateur[maxNomOperateur + 1];
boolean gsmOn = true;
// message d'alerte
const char smsTemperature[] PROGMEM = " Temperature : ";
const char smsDegree[] PROGMEM = " degree";
const char smsPh[] PROGMEM = " Ph : ";
const char texteSurSD3[] PROGMEM = "reboot Gsm erreur passage mode routage";
const char texteSurSD4[] PROGMEM = "reboot Gsm";
const char texteSurSD5[] PROGMEM = "reboot Gsm erreur passage mode texte";
const char texteSurSD6[] PROGMEM = "reboot Gsm pas de reponse AT";
uint8_t signalGSM = 0;

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ Fonctions $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
boolean getGSMLine() {
  static uint8_t indexMessage = zero;
  boolean incomingMessage = true;
  while (gsm.available() && incomingMessage) {
    int c = gsm.read();
    if (c != -1) {
      switch (c) {
        case '\n':
          messageGsm[indexMessage] = '\0'; // a la fin character NULL pour c-string correct
          indexMessage = zero; // pret pour la suite
          incomingMessage = false;
          break;
        case '\r': // n'est pas pris en compte
          break;
        default:
          if (indexMessage <= maxMessageSize - un) messageGsm[indexMessage++] = (char) c; // sinon ignoré
          break;
      }
    }
  }
  return !incomingMessage;
}

void ecouterGSM() {
  const uint8_t maxstockSignal = 4;
  char stockSignal[maxstockSignal + 1];
  if (getGSMLine()) {
    if (!strncmp(messageGsm, "+COPS: 0,", 9)) { // on compare le début de la chaîne
      char *ptr = strtok(messageGsm, "\"");
      if (ptr) {
        ptr =  strtok(NULL, "\"");
        strncpy(nomOperateur, ptr, maxNomOperateur);
        nomOperateur[maxNomOperateur] = '\0';
        DPRINTF("Operateur : "); DPRINT(nomOperateur); DPRINTLN();
      }
    }
    else if (!strncmp(messageGsm, "+COPS: 0", 8)) { // on compare le début de la chaîne
      DPRINTF("Pas de reponse du module GSM !!!"); DPRINTLN();
    }
    else if (!strncmp(messageGsm, "+CSQ:", 5)) { // on compare le début de la chaîne
      char *ptr = strtok(messageGsm, " ");
      if (ptr) {
        ptr =  strtok(NULL, "\"");
        memset(stockSignal, '\0', maxstockSignal); // effacer buffer
        strncpy(stockSignal, ptr, maxstockSignal);
        stockSignal[maxstockSignal] = '\0';
        signalGSM = atoi (stockSignal);
        DPRINTF("signal GSM : "); DPRINT(signalGSM); DPRINTLN();
      }
    }
  }
}

boolean waitForString(const char * endMarker, unsigned long duration, boolean verbose) {
  int localBufferSize = strlen(endMarker);
  char localBuffer[localBufferSize];
  uint8_t index = 0;
  boolean endMarkerFound = false;
  unsigned long currentTime;
  memset(localBuffer, '\0', localBufferSize); // effacer buffer
  currentTime = millis();
  while (millis() - currentTime <= duration) {
    if (gsm.available() > zero) {
      if (index == localBufferSize) {
        index = zero;
      }
      localBuffer[index] = (uint8_t) gsm.read();
      if (verbose) {
        DPRINT((char) localBuffer[index]);
      }
      endMarkerFound = true;
      for (int i = zero; i < localBufferSize; i++) {
        if (localBuffer[(index + un + i) % localBufferSize] != endMarker[i]) {
          endMarkerFound = false;
          break;
        }
      }
      index++;
    }
    if (endMarkerFound) break;
  }
  return endMarkerFound;
}

/*GsmPrintlnAndWaitATCommand exécute une commmand AT en ajoutant à la fin CR LF
  il vérifie si endMarker est recu du GSM pour la durée Max , boolean si le marqueur a été trouvé
  avec l'option verbeuse, les info du GSM sont imprimées dans le serial */
boolean gsmPrintlnAndWaitATCommand(const char * command, const char * endMarker, unsigned long duration, boolean verbose) {
  gsm.println(command);
  return waitForString(endMarker, duration, verbose);
}

boolean gsmPrintAndWaitATCommand(const char * command, const char * endMarker, unsigned long duration, boolean verbose) {
  gsm.print(command);
  return waitForString(endMarker, duration, verbose);
}

/* envoie les commande AT pour envoyer SMS */
boolean envoyerSMS(const char * noDeTel, const char * messageAEnvoyer) {
  boolean SMSenvoye = false;
  if (gsmOn ) {
    gsmPrintATCommand("AT+CMGS=\"");
    gsmPrintATCommand(noDeTel);
    if (gsmPrintlnAndWaitATCommand("\"", ">", mille, false)) {
      gsmPrintlnATCommand(messageAEnvoyer);
      SMSenvoye = gsmPrintlnAndWaitATCommand(ctrlZString, OKShortString, mille, false);
    } else {
      gsmPrintlnATCommand(escapeString);
      gsmOn = false;
      reboot(hardResetGSM);
      horodatageEtEcritSurSD (texteSurSD4, "");
    }
    return SMSenvoye;
  }
}

void etatGSM() {
  if (gsmOn == false) { // si GSM OFF
    DPRINTF("GSM OFF"); DPRINTLN();
    if (gsmPrintlnAndWaitATCommand(ATString, OKLongString, mille, true)) {// on verifie si le module repond
      if (gsmPrintlnAndWaitATCommand("AT+CMGF=1", OKLongString, mille, true)) { // on passe les SMS en mode texte
        if (gsmPrintlnAndWaitATCommand("AT+CNMI=1,0,0,0,0", OKLongString, mille, true)) { // on passe en mode routage des SMS vers le terminal
          gsmOn = !gsmOn;
          envoyerSMS(numero2tel, messageGsm);
        }
        else { // si erreur passage en mode routage des SMS vers le terminal
          reboot(hardResetGSM); // on reboot
          horodatageEtEcritSurSD (texteSurSD3, "");
        }
      }
      else { // si erreur passage en mode SMS en mode texte
        reboot(hardResetGSM); // on reboot
        horodatageEtEcritSurSD (texteSurSD5, "");
      }
    }
    else { // si pas de reponse du module
      reboot(hardResetGSM); // on reboot
      horodatageEtEcritSurSD (texteSurSD6, "");
    }
  }
  else {
    DPRINTF("GSM ON"); DPRINTLN();
  }
}

/* complete le SMS et demande l'envoi */
void completerMessageAlerte(const char debutMessage[] PROGMEM, const char milieuMessage[] PROGMEM) {
  effaceMessageGsm();
  effaceBufferTexte();
  strncpy_P (messageGsm, debutMessage, maxMessageSize); // ajoute le debut du message
  messageGsm[maxMessageSize] = '\0';
  strncpy_P (bufferTexte, milieuMessage, maxbufferTexte); // recupere "char" en memoire flash et le copie
  bufferTexte[maxbufferTexte] = '\0';
  if (strlen(bufferTexte) + strlen(messageGsm ) < maxMessageSize ) {
    strcat(messageGsm, bufferTexte); // ajoute le char recupere au debut du message
    envoyerSMS(numero2tel, messageGsm); // demande l'envoye du SMS
    DPRINTF("SMS autre: "); DPRINT(messageGsm); DPRINTF(" au :"); DPRINT(numero2tel);  DPRINTLN(); // debug
    horodatageEtEcritSurSD (vide, messageGsm);
  }
  else {
    DPRINTF("probleme de place memoire dans completerMessageAlerte");  DPRINTLN(); // debug
  }
}

/* complete le SMS PH et demande l'envoi */
void completerMessageAlertePh(const char debutMessage[] PROGMEM) {
  effaceMessageGsm();
  effaceBufferTexte();
  strncpy_P (messageGsm, debutMessage, maxMessageSize);// ajoute le debut du message
  messageGsm[maxMessageSize] = '\0';
  strncpy_P (bufferTexte, smsPh, maxbufferTexte); // recupere "char" en memoire flash et le copie
  bufferTexte[maxbufferTexte] = '\0';
  if (strlen(bufferTexte) + strlen(messageGsm ) < maxMessageSize ) {
    strcat(messageGsm, bufferTexte);// ajoute le char recupere au debut du message
    effaceBufferTexte();
    float calculPh = ((float)variable.phBac / cent); // met la mesure ph au bon format
    dtostrf (calculPh, 5, 2, bufferTexte); // convertie la mesure PH en char
    if (strlen(bufferTexte) + strlen(messageGsm ) < maxMessageSize ) {
      strcat(messageGsm, bufferTexte); // ajoute la mesure PH au SMS
      envoyerSMS(numero2tel, messageGsm); // demande l'envoye du SMS
      DPRINTF("SMS ph : "); DPRINT(messageGsm); DPRINTF(" au :"); DPRINT(numero2tel);  DPRINTLN(); // debug
      horodatageEtEcritSurSD (vide, messageGsm);
    }
    else {
      DPRINTF("probleme de place memoire dans completerMessageAlertePh niveau 2");  DPRINTLN(); // debug
    }
  }
  else {
    DPRINTF("probleme de place memoire dans completerMessageAlertePh niveau 1");  DPRINTLN(); // debug
  }
}

/* complete le SMS temperature et demande l'envoi */
void completerMessageAlerteTemperature(const char debutMessage[] PROGMEM) {
  effaceMessageGsm();
  effaceBufferTexte();
  strncpy_P (messageGsm, debutMessage, maxMessageSize); // ajoute le debut du message
  messageGsm[maxMessageSize] = '\0';
  strncpy_P (bufferTexte, smsTemperature, maxbufferTexte); // recupere "char" en memoire flash et le copie
  bufferTexte[maxbufferTexte] = '\0';
  if (strlen(bufferTexte) + strlen(messageGsm ) < maxMessageSize ) {
    strcat(messageGsm, bufferTexte); // ajoute le char recupere au debut du message
    effaceBufferTexte();
    dtostrf (variable.temperatureBac, 5, 2, bufferTexte); // convertie la temperature en char
    if (strlen(bufferTexte) + strlen(messageGsm ) < maxMessageSize ) {
      strcat(messageGsm, bufferTexte); // ajoute la temperature au SMS
      effaceBufferTexte();
      strncpy_P (bufferTexte, smsDegree, maxbufferTexte); // recupere "char" en memoire flash et le copie
      bufferTexte[maxbufferTexte] = '\0';
      if (strlen(bufferTexte) + strlen(messageGsm) < maxMessageSize ) {
        strcat(messageGsm, bufferTexte); // ajoute le char recupere au debut du message
        envoyerSMS(numero2tel, messageGsm); // demande l'envoye du SMS
        alerteTemperature = !alerteTemperature;
        DPRINTF("SMS temperature : "); DPRINT(messageGsm); DPRINTF(" au :"); DPRINT(numero2tel);  DPRINTLN(); // debug
        horodatageEtEcritSurSD (vide, messageGsm);
      }
      else {
        DPRINTF("probleme de place memoire dans completerMessageAlerteTemperature niveau 3");  DPRINTLN(); // debug
      }
    }
    else {
      DPRINTF("probleme de place memoire dans completerMessageAlerteTemperature niveau 2");  DPRINTLN(); // debug
    }
  }
  else {
    DPRINTF("probleme de place memoire dans completerMessageAlerteTemperature niveau 1");  DPRINTLN(); // debug
  }
}

void effaceMessageGsm() {
  memset(messageGsm, '\0', maxMessageSize); // effacer buffer
}

boolean nomOperateurRecu() {
  boolean recu = false;
  if (strcmp (nomOperateur, bufferTexte) != zero) {
    recu = true;
    DPRINTF("operateur recu avant fin du timer : ");  DPRINTLN(nomOperateur); // debug
  }
  return recu;
}

boolean signalGSMRecu() {
  boolean recu = false;
  if (signalGSM != zero) {
    DPRINTF("puissance signal recu avant fin du timer : ");  DPRINTLN(signalGSM); // debug
    recu = true;
  }
  return recu;
}
