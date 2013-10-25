#include "serveur.h"

int main(int argc, char **argv) {
	Serveur* Monserveur=Serveur::getInstance();
	Monserveur->run();
	delete Monserveur;
	return 0;
}
