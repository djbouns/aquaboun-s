
/********************************************************************************************************************************************************************************************************
****                                                                                      AQUABOUNS REEF v1.2                                                                                        ****
****                                                                         Automate pour la gestion d'aquarium recifal                                                                             ****
****                                                                                       Créé par Djbouns                                                                                          ****
****                                                                Avec l'aide du forum https://forum.arduino.cc pour le codage                                                                     ****
****                                                                                     Un grand merci a J.M.L                                                                                      ****
****                                                                                           05/01/19                                                                                           ****
********************************************************************************************************************************************************************************************************/

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

#include "ESP8266WiFi.h"
#include <Servo.h>
#include <EEPROM.h>
#define arduino Serial  // là où est connecté l'autre Arduino
#include "FS.h" // pour le SPIFFS
#include "debug.h"

const char * page2 = "/aquabouns.html";
const char * page1 = "/admin.html";
const char * page3 = "/favicon.ico";

const byte led = D1;
const byte maxURL = 35;
char urlRequest[maxURL + 1];
const byte maxHTTPLine = 100;
char httpLine[maxHTTPLine + 1];
const uint16_t HTTPPort = 80;
WiFiServer server(HTTPPort);
const uint8_t maxSsid = 30;// prévois la place
const uint8_t maxMdp = 30;// prévois la place
boolean envoyerStatutDeConnection = false ;
boolean recevoirIdentifiantDeConnection = false ;
uint32_t compteurDemandeIdentifiant;
float PH;
const uint8_t maxAdresseIP = 30;
uint16_t delaisEntreDemandeIdentifiant = 10000; // si pas recu, envoie une demande d'identifiant a l'arduino toute les 10 seconde

// LES COMMANDES DOIVENT TOUTES AVOIR LA MEME TAILLE
const size_t tailleCommande = 4; // 4 caractères
enum typeCommande_t : byte {ENVOYERsauvegarde = 0, ENVOYERvariable, ENVOYERidentifiantWifi, ENVOYERconnectionWifi,
                            RECEVOIRsauvegarde, RECEVOIRvariable, RECEVOIRidentifiantWifi, RECEVOIRconnectionWifi, AUCUNE = 255
                           }; // dans le même ordre que les commandes
const char *  lesCommandes[] = {"$E1$", "$E2$", "$E3$", "$E4$", "$R1$", "$R2$", "$R3$", "$R4$"};
const size_t nbCommandes = sizeof(lesCommandes) / sizeof(lesCommandes[0]);

struct __attribute__ ((packed)) paramS_t {// force l'ordre des champs
  // page menu
  bool tempeteAleatoireOn;
  bool nourrissageON[2];
  uint8_t remonteDelay;
  uint8_t eclairageOnOffPwm[4];
  uint8_t brassageOnOffPwm[3];
  // page config
  uint8_t puissanceMaxEclairage[4];
  uint32_t leverSoleil;
  uint32_t coucherSoleil;
  uint16_t dureelevercoucher;
  float alertetemperaturebasse;
  float alertetemperaturehaute;
  float ventilationaquarium;
  uint8_t ventilationrampe;
  bool adresseSondeRampe;
  bool adresseSondeBac;
  uint32_t heureNourrissage[2];
  uint32_t dureeNourrissageMillis;
  uint16_t dureeOsmolationMillis ;
  uint8_t compteurOsmolationMax;
  uint16_t consignePhRac;
  uint16_t alertephbacbas;
  uint16_t alertephbachaut;
  // page brassage
  uint8_t puissanceMaxBrassage[3];
  uint8_t angle1Oscillo[3];
  uint8_t angle2Oscillo[3];
  uint16_t delaisMouvementOscilloMillis[3];
  uint32_t dureeTempeteMillis;
  uint8_t puissanceTempete;
  uint8_t accalmieNocturne;
  // horloge
  uint32_t heureDebutAlerte;
  uint32_t heureFinAlerte;
} sauvegarde;
const size_t tailleparamS_t = sizeof(paramS_t);

struct __attribute__ ((packed)) valeursActuel_t { // structure des valeurs actuels
  uint8_t Heure;
  uint8_t minut;
  uint8_t seconde;
  float temperatureBac; // pour la temperature du bac
  uint8_t temperatureRampe; // pour la temperature de la rampe
  uint8_t pwmEclairage[4];
  uint8_t pwmBrassage[3];
  uint16_t phBac;
  uint16_t phRac;
  uint8_t nbrBrassageEnON;
} variable;
const size_t taillevaleursActuel_t = sizeof(valeursActuel_t);

struct __attribute__ ((packed)) identifiantDeConnection_t { // structure identifiant de connection
  char ssid[31]; // pour le SSID prevois la place + carractere de fin
  char mdp[31]; // pour le MDP prevois la place + carractere de fin
} identifiantWifi;
const size_t tailleidentifiantDeConnection_t = sizeof(identifiantDeConnection_t);

struct __attribute__ ((packed)) etatconnection_t { // structure de connection
  boolean connecter;
  boolean presenceModule;
  uint8_t puissanceSignal;
  char adresseIP [31];// prevois la place + carractere de fin
} connectionWifi;
const size_t tailleetatconnection_t = sizeof(etatconnection_t);

void envoyerVariables(WiFiClient &client) { // envoie les variable sur la page wew
  client.print(F("HTTP/1.1 200 OK\r\n"
            "Content-Type: text/xml\r\n"
            "Connection: keep-alive\r\n\r\n"
            "<?xml version = \"1.0\" ?>"));
  client.print(F("<variables><TimeH>"));
  if (variable.Heure < 10) {
    client.print(F("0"));
  }
  client.print(variable.Heure);
  client.print(F("</TimeH><TimeM>"));
  if (variable.minut < 10) {
    client.print(F("0"));
  }
  client.print(variable.minut);
  client.print(F("</TimeM><TimeS>"));
  if (variable.seconde < 10) {
    client.print(F("0"));
  }
  client.print(variable.seconde);
  client.print(F("</TimeS><Tbac>"));
  client.print(variable.temperatureBac);
  client.print(F("</Tbac><Trampe>"));
  client.print(variable.temperatureRampe);
  client.print(F("</Trampe><Pblanc>"));
  client.print((variable.pwmEclairage[0] + variable.pwmEclairage[1]) / 2);
  client.print(F("</Pblanc><Pbleu>"));
  client.print((variable.pwmEclairage[2] + variable.pwmEclairage[3]) / 2);
  client.print(F("</Pbleu><Pbrass>"));
  client.print((variable.pwmBrassage[0] + variable.pwmBrassage[1] + variable.pwmBrassage[2]) / variable.nbrBrassageEnON);
  client.print(F("</Pbrass><Phbac>"));
  PH = variable.phBac;
  client.print(PH / 100);
  client.print(F("</Phbac><Phrac>"));
  PH = variable.phRac;
  client.print(PH / 100);
  client.print(F("</Phrac></variables>"));
}

void recupererPuissanceSignal() {
  int8_t dbSignal;
  dbSignal = WiFi.RSSI(); // recupere la puissance du signal
  dbSignal = (dbSignal - (dbSignal * 2)); // pour resultat en positif
  connectionWifi.puissanceSignal = dbSignal; // copie la variable
}

void connectionAuWifi() {
  connectionWifi.connecter = false; // statut "non connecter au reseau wifi"
  DPRINTLN();  DPRINTF("Connecting to "); DPRINTLN(identifiantWifi.ssid);
  WiFi.begin(identifiantWifi.ssid, identifiantWifi.mdp);
  DPRINTLN();
  while (WiFi.status() != WL_CONNECTED) {
    ecouter();
  }
  connectionWifi.connecter = true; // statut "connecter au reseau wifi"
  DPRINTLN();  DPRINTLNF("WiFi connected");
  DPRINTF("puissance du signal :");  DPRINTLN(WiFi.RSSI());
  recupererPuissanceSignal();
  server.begin(); // demmare le serveur
  memset(connectionWifi.adresseIP, '\0', maxAdresseIP); // effacer buffer
  sprintf (connectionWifi.adresseIP, "Connecte, IP : %d.%d.%d.%d", WiFi.localIP () [0], WiFi.localIP () [1], WiFi.localIP () [2], WiFi.localIP () [3]); // recupere l'adresse IP
  DPRINTLN(connectionWifi.adresseIP);
  executer(ENVOYERconnectionWifi, true); // envoie les identifiants de connection
}

boolean envoyerData(uint8_t * d, size_t taille ) {
  size_t somme = arduino.write((uint8_t *) d, taille); // envoi des données
  DPRINTF("somme : "); DPRINT(somme); DPRINTF(" // taille : "); DPRINT(taille); DPRINTLN();
  return (somme == taille); // retourne vrai si le bon nombre d'octet a été envoyé
}

boolean recevoirData(uint8_t * d, size_t taille) {
  size_t somme = arduino.readBytes(d, taille); // reception des données
  DPRINTF("somme : "); DPRINT(somme); DPRINTF(" // taille : "); DPRINT(taille); DPRINTLN();
  return (somme == taille); // retourne vrai si le bon nombre d'octet a été reçu avant le timeout
}

void ecouter() {
  typeCommande_t commandeDistante = ecouterCommandeDistante();
  if (commandeDistante != AUCUNE) executer(commandeDistante, false);
}

void executer(typeCommande_t uneCommande, boolean commandeLocale) {
  switch (uneCommande) {

    case ENVOYERsauvegarde:
      if (commandeLocale) {
        arduino.write((uint8_t *) lesCommandes[RECEVOIRsauvegarde], tailleCommande);
      }
      if (!envoyerData((uint8_t *) &sauvegarde, tailleparamS_t )) {
        DPRINTLN("Erreur envoi params");
      }
      else {
        DPRINTLN("params envoyer");
      }
      break;

    case ENVOYERvariable:
      if (commandeLocale) {
        arduino.write((uint8_t *) lesCommandes[RECEVOIRvariable], tailleCommande);
      }
      if (!envoyerData((uint8_t *) &variable, taillevaleursActuel_t )) {
        DPRINTLN("Erreur envoi variables");
      }
      else {
        DPRINTLN("variables envoyer");
      }
      break;

    case ENVOYERidentifiantWifi:
      if (commandeLocale) {
        arduino.write((uint8_t *) lesCommandes[RECEVOIRidentifiantWifi], tailleCommande);
      }
      if (!envoyerData((uint8_t *) &identifiantWifi, tailleidentifiantDeConnection_t )) {
        DPRINTLN("Erreur envoi identifiantWifi");
      }
      else {
        DPRINTLN("identifiantWifi envoyer");
      }
      break;

    case ENVOYERconnectionWifi:
      if (commandeLocale) {
        arduino.write((uint8_t *) lesCommandes[RECEVOIRconnectionWifi], tailleCommande);
      }
      if (connectionWifi.connecter == false) {
        connectionWifi.puissanceSignal = 0;
        memset(connectionWifi.adresseIP, '\0', maxAdresseIP); // effacer buffer
      }
      if (!envoyerData((uint8_t *) &connectionWifi, tailleetatconnection_t )) {
        DPRINTLN("Erreur envoi connectionWifi");
      }
      else {
        DPRINT("connectionWifi.connecter : "); DPRINTLN(connectionWifi.connecter);
        DPRINT("connectionWifi.presenceModule : "); DPRINTLN(connectionWifi.presenceModule);
        DPRINT("connectionWifi.puissanceSignal : "); DPRINTLN(connectionWifi.puissanceSignal);
        DPRINT("connectionWifi.adresseIP : "); DPRINTLN(connectionWifi.adresseIP);
        DPRINTLN("connectionWifi envoyer");
      }
      break;

    case  RECEVOIRsauvegarde:
      if (commandeLocale) {
        arduino.write((uint8_t *) lesCommandes[ENVOYERsauvegarde], tailleCommande);
      }
      if (!recevoirData((uint8_t *) &sauvegarde, tailleparamS_t)) {
        DPRINTLN("Erreur reception data1");
      }
      else {
        DPRINTLN("params recu et sauvegarder");
      }
      break;

    case  RECEVOIRvariable:
      if (commandeLocale) {
        arduino.write((uint8_t *) lesCommandes[ENVOYERvariable], tailleCommande);
      }
      if (!recevoirData((uint8_t *) &variable, taillevaleursActuel_t)) {
        DPRINTLN("Erreur reception variables");
      }
      else {
        DPRINTLN("variables recu");
      }

      break;

    case  RECEVOIRidentifiantWifi:
      if (commandeLocale) {
        arduino.write((uint8_t *) lesCommandes[ENVOYERidentifiantWifi], tailleCommande);
      }
      WiFi.disconnect(true);
      if (!recevoirData((uint8_t *) &identifiantWifi, tailleidentifiantDeConnection_t)) {
        DPRINTLN("Erreur reception identifiantWifi");
      }
      else {
        recevoirIdentifiantDeConnection = true;
        DPRINTLN("identifiantWifi recu");
        DPRINT(" ssid : ");
        DPRINTLN(identifiantWifi.ssid);
        DPRINT(" mdp : ");
        DPRINTLN(identifiantWifi.mdp);
        connectionAuWifi();
      }
      break;

    case  RECEVOIRconnectionWifi:
      if (commandeLocale) {
        arduino.write((uint8_t *) lesCommandes[ENVOYERconnectionWifi], tailleCommande);
      }
      if (!recevoirData((uint8_t *) &connectionWifi, tailleetatconnection_t)) {
        DPRINTLN("Erreur reception connectionWifi");
      }
      else {
        DPRINTLN("connectionWifi recu");
      }
      break;

    case  AUCUNE:
      break;
  }
}

typeCommande_t ecouterCommandeDistante() {
  static uint8_t sentinelle[tailleCommande]; // pour recevoir les données
  static uint8_t indice = 0; // la position dans le buffer circulaire
  typeCommande_t commandeRecue = AUCUNE;
  if (arduino.available()) {
    if (indice == tailleCommande) indice = 0;
    sentinelle[indice] = (uint8_t) arduino.read();// on regarde si on a reçu une des commandes
    for (byte c = 0; c < nbCommandes; c++) {
      commandeRecue = (typeCommande_t) c; // petit hack car on a aligné l'enum pour qu'il commence à 0
      for (byte i = 0; i < tailleCommande; i++) {
        if (sentinelle[(indice + 1 + i) % tailleCommande] != lesCommandes[c][i]) {
          commandeRecue = AUCUNE;
          break;
        }
      }
      if (commandeRecue != AUCUNE) break;
    }
    if (commandeRecue != AUCUNE) {// on se reprépare pour la prochaine fois
      memset(sentinelle, 0, tailleCommande); // On efface la sentinelle pour la prochaine fois (http://www.cplusplus.com/reference/cstring/memset/)
      indice = 0;
    } else indice++;
  }
  return commandeRecue;
}

boolean testRequeteWeb() {
  boolean requeteHTTPRecue = false;
  byte indexMessage = 0;
  char * ptrGET, *ptrEspace;
  WiFiClient client = server.available();
  if (!client) return requeteHTTPRecue; // pas de client connecté
  boolean currentLineIsBlank = true;
  while (client.connected()) {
    if (client.available()) {
      char c = client.read();
      if (c == '\n' && currentLineIsBlank) { // une requête HTTP se termine par une ligne vide
        executer(RECEVOIRvariable, true);
        // ON GENERE LA PAGE WEB
        if (strcmp(urlRequest, "/favicon.ico")) { // si ce n'est pas pour le favicon
          requeteHTTPRecue = true;

          // on regarde si on a une requete qui continet /reqEtatBouton ( sinon on renvoie toute la page)
          if (strstr(urlRequest, "/reqEtatVariables")) { // http://www.cplusplus.com/reference/cstring/strstr/?kw=strstr
            // on va lire l'état du bouton et on renvoie l'information correctement
            envoyerVariables(client);
          }
          else if (!strncmp(urlRequest, "/admin", 6)) {  // on vérifie quelle page est demandée en se limitant au 6 premier caracter
            if (SPIFFS.exists(page2)) {
              // On envoie un en tête de réponse HTTP standard de type HTML
              client.println("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: keep-alive\r\n");
              File pageWeb = SPIFFS.open(page2, "r");
              client.write(pageWeb);
              pageWeb.close();
            } else {
              DPRINTLN(F("Erreur chargement page ADMIN"));
            }
          }
          else { // on envoie la page web par défaut
            if (SPIFFS.exists(page1)) {
              // On envoie un en tête de réponse HTTP standard de type HTML
              client.println("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: keep-alive\r\n");
              File pageWeb = SPIFFS.open(page1, "r");
              client.write(pageWeb);
              pageWeb.close();
            } else {
              DPRINTLN(F("Erreur chargement page VARIABLE"));
            }
          }
        }
        else {
          if (SPIFFS.exists(page3)) {
            // On envoie un en tête de réponse HTTP standard de type HTML
            File pageWeb = SPIFFS.open(page3, "r");
            client.write(pageWeb);
            pageWeb.close();
          } else {
            DPRINTLN(F("Erreur chargement FAVICONE"));
          }
        }
        break; // on sort du while et termine la requête
      } // fin de génération de la réponse HTTP

      if (c == '\n') {
        currentLineIsBlank = true;
        httpLine[indexMessage] = '\0'; // on termine la ligne correctement (c-string)
        indexMessage = 0; // on se reprépre pour la prochaine ligne
        if (ptrGET = strstr(httpLine, "GET")) {
          // c'est la requête GET, la ligne continent "GET /URL HTTP/1.1", on extrait l'URL
          ptrEspace = strstr(ptrGET + 4, " ");
          *ptrEspace = '\0';
          strncpy(urlRequest, ptrGET + 4, maxURL);
          urlRequest[maxURL] = '\0'; // par précaution si URL trop longue
        }
      } else if (c != '\r') {
        currentLineIsBlank = false;
        if (indexMessage <= maxHTTPLine - 1) {
          httpLine[indexMessage++] =  c; // sinon on ignore le reste de la ligne
        }
      }
    } // end if available
  } // end while
  delay(10);
  client.stop(); // termine la connexion
  return requeteHTTPRecue;
}

void printHTTPServerInfo() {
  DPRINTF("Site web http://"); DPRINT(WiFi.localIP());
  if (HTTPPort != 80) {
    DPRINTF(":");
    DPRINT(HTTPPort);
  }
  DPRINTLN();
}

void setup() {
  arduino.begin(115200);
  arduino.setTimeout(500); // temps max de réceptoin d'une structure en ms
  DPRINTLN("\n\nTest SPIFFS\n");

  // on démarre le SPIFSS
  if (!SPIFFS.begin()) {
    DPRINTLN("erreur SPIFFS");
    while (true); // on ne va pas plus loin
  }
  delay(10);
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);
  connectionWifi.connecter = false; // initialise le statut "non connecter au wifi"
  connectionWifi.presenceModule = true; // initialise le statut "presence module"

  // Connect to WiFi network
  DPRINTLN();
  DPRINTLN("en attente des identifiant de connection ...");
  while (recevoirIdentifiantDeConnection == false) {
    if ((compteurDemandeIdentifiant + delaisEntreDemandeIdentifiant) < millis()) {
      executer(RECEVOIRidentifiantWifi, true); // fait une demande pour recevoir les identifiants de connection
      compteurDemandeIdentifiant = millis();
    }
    ecouter();
  }
  connectionAuWifi();

  server.begin();
  printHTTPServerInfo();

  /* Active le watchdog de l'arduino
    Si plus de remise a zero du compteur "reboot" (fait dans le loop) au bout de 8 secondes = bug de l'arduino = reboot automatique ! */
  wdt_enable(WDTO_8S);
}

void loop() {
  ecouter();
  if (WiFi.status() == WL_CONNECTED) { // si toujours connecter au reseau wifi
    digitalWrite(led, HIGH);
    connectionWifi.connecter = true;
    if (envoyerStatutDeConnection) {
      envoyerStatutDeConnection = !envoyerStatutDeConnection;
      executer(ENVOYERconnectionWifi, true); // envoie les identifiants de connection
    }
    testRequeteWeb();
  }
  else {
    connectionWifi.connecter = false;
    digitalWrite(led, LOW);
    if ((compteurDemandeIdentifiant + delaisEntreDemandeIdentifiant) < millis()) {
      compteurDemandeIdentifiant = millis();
      executer(ENVOYERconnectionWifi, true); // envoie les identifiants de connection
    }
  }
  wdt_reset(); // indique que le loop est OK, pas de bug, remise a zero du compteur "reboot" du watchdog
}
