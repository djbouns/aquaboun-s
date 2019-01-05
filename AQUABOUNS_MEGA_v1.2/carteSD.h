#ifndef  _carteSD_h_
#define _carteSD_h_

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ Librairies $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
#include <arduino.h>

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ Declarations des fonctions dans le .cpp $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
void ecritSurSd(const char * nomDuFichierTxt, char * donneeAEcrireSurSD);
boolean lisSurSd(const char * nomDuFichierTxt, char *donneeLueSurSD, size_t NbMaxCaracteres);
void horodatageEtEcritSurSD (const char Erreur1AAjouter[] PROGMEM, const char* Erreur2AAjouter);

#endif
