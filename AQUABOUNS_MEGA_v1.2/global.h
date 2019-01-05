#ifndef global_h_
#define global_h_

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ Librairies $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
#include <Servo.h>
#include "DallasTemperature.h"
#include <OneWire.h>
#include "RTClib.h"
#include "SdFat.h"
#include "Nextion.h"

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ Fichiers $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
#include "eeprom.h"

/* dans .ino */
extern uint32_t roulementPrecedent2;
extern const char texteNextion2[] PROGMEM;
extern const char texteNextion25[] PROGMEM;
extern const char texteNextion26[] PROGMEM;
extern const char texteNextion27[] PROGMEM;
extern const char texteNextion32[] PROGMEM;

/* dans affichage.cpp */
extern boolean onOffalimentation, etalonnageOnOFF, initialisationPhDemarrage, mesurePhDemarrage;
extern const char vide[] PROGMEM;
extern char bufferTexte[];
extern const uint8_t dix, cent, maxbufferTexte, zero, un, deux, trois, quatre, pas2Changement2Couleur;
enum pageNextion : uint8_t {menu, parametre, brass, horloge , wifi, tel, info, standby, demarrage};
extern const uint16_t mille, deuxMille;
extern uint32_t number, numero_page, recupHorraireTemporraire;
extern pageNextion pageActuelNextion;
extern NexPage pageMenu;
extern NexButton onoffpomperemonte;
extern NexButton onoffalimentation1;
extern NexButton onoffalimentation2;
extern NexButton onoffpwmbrassage1;
extern NexButton onoffpwmbrassage2;
extern NexButton onoffpwmbrassage3;
extern NexButton onoffpwmblanc;
extern NexButton onoffpwmbleu;
extern NexButton onofftempete;
extern NexButton onofftempetealeatoire;
extern NexScrolltext messagedefilantalimentationencours;
extern NexText messageanomalie;
extern NexText messageetalonnage;
extern NexVariable variableetatboutons;
extern NexText affichageheurealimentation1;
extern NexText affichageheurealimentation2;
extern NexButton boutonmenuversconfig;
extern NexButton boutonalimentationmanuel;
extern NexButton boutonconfigverstel;
extern NexButton boutonconfigverswifi;
extern NexButton boutonconfigversmenu;
extern NexButton boutonconfigversinfo;
extern NexButton boutonwifiversconfig;
extern NexButton boutontelversconfig;
extern NexButton boutoninfoversconfig;
extern NexButton boutonenregistrertelversconfig;
extern NexButton boutonenregistrerwifiversconfig;
extern NexButton boutonenregistrerconfigversmenu;
extern NexButton boutonbrassageversconfig;
extern NexButton boutonenregistrerbrassageversconfig;
extern NexButton boutonconfigversbrassage;
extern NexButton boutonconfigvershorloge;
extern NexButton boutonhorlogeversconfig;
extern NexButton boutonenregistrerhorlogeversconfig;
extern NexButton boutonomoinsoscillo1min;
extern NexButton boutonplusoscillo1min;
extern NexButton boutonomoinsoscillo1max;
extern NexButton boutonplusoscillo1max;
extern NexButton boutonomoinsoscillo2min;
extern NexButton boutonplusoscillo2min;
extern NexButton boutonomoinsoscillo2max;
extern NexButton boutonplusoscillo2max;
extern NexButton boutonomoinsoscillo3min;
extern NexButton boutonplusoscillo3min;
extern NexButton boutonomoinsoscillo3max;
extern NexButton boutonplusoscillo3max;
extern NexButton boutonversstandby;
extern NexButton boutonetalonnage;
extern NexPage pagestandby;
extern NexText texte1;
extern NexText texte2;
extern NexText texte3;
extern NexText texte4;
extern NexText texte5;
extern NexText texte6;
extern const uint16_t vert, orange, rouge, batterie0;
extern uint16_t mesureNiveauBatterie;

/* autres.cpp */
extern boolean nourrissage1ON, nourrissage2ON, texteNourrissage, alarmeOnOff;
extern const uint16_t adresseMotClef, adresseDeSauvegarde;
extern const uint32_t motClef;
extern uint32_t dureeNourrissageMillis, heureNourrissage[];
enum {numeroUn, numeroDeux, numeroTrois}; // utiliser pour brassage, oscillo, nourrissage ...

/* dans brassage.cpp */
enum {Arret, Actif, Pwm, Delay}; // utiliser pour brassage, eclairage, pompe de remonté
extern boolean MajEcranTempeteEnCours, nourissageEnCours, tempeteManuelEnCours, tempeteAleatoireEnCours;
extern uint8_t pwmBrassage[], puissanceMaxBrassage[], puissanceMinBrassage[], accalmieNocturne, puissanceTempete;
extern uint32_t dureeTempeteMillis, heureTempeteAleatoire, dureeTempeteAleatoire;
extern float pourCentEnPwm;

/* dans eclairage */
enum {bleu1, bleu2, blanc1, blanc2};
extern uint8_t pwmEclairage[], puissanceMaxEclairage[];
extern uint16_t dureelevercoucher;
extern uint32_t leverSoleil, coucherSoleil, debutleverSoleil[], finleverSoleil[], debutcoucherSoleil[], fincoucherSoleil[];

/* dans floteurs.cpp */
extern boolean smsAlerteOsmolation, smsAlerteGodet, smsAlerteSecurite, smsAlerteReserve;

/* dans gsm.cpp */
extern boolean gsmOn, envoyerSMS(const char * noDeTel, const char * messageAEnvoyer);
extern boolean gsmPrintlnAndWaitATCommand(const char * command, const char * endMarker, uint32_t duration, boolean verbose);
extern char numero2tel[], messageGsm[], buffer[], nomOperateur[];
extern const char * ATString, * OKShortString, * OKLongString;
extern const uint8_t maxnumero2tel, maxMessageSize;

extern const uint8_t maxNomOperateur;
extern uint8_t signalGSM ;

/* dans horloge.cpp */
extern char textime[], minuteEnTexteOut[], minuteEnTexteIn[];
extern const uint8_t minuteEnSeconde, maxminuteEnTexte;
extern uint8_t Heure, minut, jour, mois;
extern uint16_t M, S, annee;
extern uint32_t Time, H, tempsPrecedentNourrissage, tempsPrecedentTempete, tempsPrecedentTempeteAleatoire;

/* dans oscillo.cpp */
extern Servo oscillo[];
extern boolean oscillo1On, oscillo2On, oscillo3On;
extern uint8_t angle1Oscillo[], angle2Oscillo[], positionOscillo[];
extern uint16_t delaisMouvementOscilloMillis[];
extern uint32_t tempsPrecedentOscillo[];

/* dans sd.cpp */
extern SdFat SD;
extern File myFile;
extern const char* ssidSurSD;
extern const char* mdpSurSD;
extern const char* numero2TelSurSD;
extern const char* erreursSurSD;

/* dans temperature.cpp */
extern RTC_DS3231 rtc;
extern OneWire oneWire;
extern DallasTemperature sensors;
extern DeviceAddress sondeBac, sondeRampe;
extern boolean alerteTemperature, alertePhBas, alertePhHaut, adresseSondeRampe, adresseSondeBac, tempeteAleatoireOn, etatVentilationBac, etatVentilationRampe;
extern const char alerte[] PROGMEM, finAlerte[] PROGMEM;
extern const uint8_t cent;
extern uint8_t temperatureRampe, ventilationrampe;
extern float moyennePhBac, moyennePhRac;
extern float temperatureBac, ventilationaquarium, alertetemperaturehaute, alertetemperaturebasse;

/* dans wifi.cpp */
enum typeCommande_t : uint8_t {ENVOYERsauvegarde = 0, ENVOYERvariable, ENVOYERidentifiantWifi, ENVOYERconnectionWifi,
                               RECEVOIRsauvegarde, RECEVOIRvariable, RECEVOIRidentifiantWifi, RECEVOIRconnectionWifi, AUCUNE = 255
                              }; // dans le même ordre que les commandes
extern const uint8_t maxSsid, maxMdp;
extern boolean reponseModuleWifi;

#endif
