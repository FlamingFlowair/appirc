#include "serveur.h"

int main(void) {
	Serveur* Monserveur=Serveur::getInstance();
	Monserveur->run();
	delete Monserveur;
	return 0;
}
