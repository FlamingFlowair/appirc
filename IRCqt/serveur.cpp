#include "serveur.h"

/// perror
#include <stdio.h>
#include <errno.h>
/// close
#include <unistd.h>
/// socket select accept
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <netdb.h>
/// exit
#include <stdlib.h>
// include du cout
#include <iostream>

#include "err.codes.h"
#include "ensemble.h"
#include <sstream>

using namespace ERR;
using namespace std;

enum {INCONNUE, CREATE_SOCKET, BIND_SOCKET, LISTEN_SOCKET};
/**************************/

Serveur* Serveur::_instance=NULL;

Serveur* Serveur::getInstance(){
	if(_instance==0){
		_instance = new Serveur ;
	}
	return _instance;
}

/**************************/

Serveur::Serveur(string hostname, unsigned int port, string nom, string messageacc) :hostname(hostname), port(port), nom(nom), messAcc(messageacc) {
	struct sockaddr_in sa;
	// On crée le socket d'écoute
	if ((fdSocket=socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("Ouverture port d'écoute");
		exit(CREATE_SOCKET);
	}
	// On l'initialise
	init_sockaddrin(&sa, hostname, port);
	// On le bind
	if (bind(fdSocket, (struct sockaddr *)&sa, sizeof(struct sockaddr)) == -1){
		perror("Erreur bind socket écoute");
		exit(BIND_SOCKET);
	}
	// On écoute sur le socket
	if (listen(fdSocket, 12) == -1) {
		perror("Erreur listen");
		exit(LISTEN_SOCKET);
	}
}

Serveur::~Serveur() {
	list<Client*>::iterator it=clientsServ.begin();
	list<Client*>::iterator end=clientsServ.end();
	while (it != end) {
		delete (*it);
		it=clientsServ.erase(it);
	}
	close(fdSocket);
}

void Serveur::init_sockaddrin(struct sockaddr_in* name, string hostname, uint16_t port) {
	struct hostent* hostinfo;
	name->sin_family=AF_INET;
	name->sin_port=htons(port);
	if ((hostinfo=gethostbyname(hostname.c_str())) == NULL) {
		cout << "Host inconnu : *" << hostname << "*" << endl;
		perror("GethostbynamE : ");
		exit(1);
	}
	name->sin_addr=*(struct in_addr*)hostinfo->h_addr;
}

string Serveur::getHostname() const {
	return hostname;
}

void Serveur::setHostname(string hostname) {
	this->hostname = hostname;
}

string Serveur::getMessageacc() const {
	return messAcc;
}

void Serveur::setMessageacc(string messageacc) {
	this->messAcc = messageacc;
}

string Serveur::getNom() const {
	return nom;
}

void Serveur::setNom(string nom) {
	this->nom = nom;
}

unsigned int Serveur::getPort() const {
	return port;
}

void Serveur::setPort(unsigned int port) {
	this->port = port;
}

int Serveur::getSocketecoute() const {
	return fdSocket;
}

void Serveur::setSocketecoute(int socketecoute) {
	this->fdSocket = socketecoute;
}

int Serveur::run()
{
	int nbrclirestant;
	fd_set tmp;
	Ensemble ensemblesolide;
	ensemblesolide.add(fdSocket);
	for(;;) {
		// On créé un ensemble temporaire pour le donner au select
		tmp=ensemblesolide.getfd_set();
		if ((nbrclirestant=select(ensemblesolide.getmax(), &tmp, NULL, NULL, NULL)) == -1) {
			perror("Erreur select");
			return -1;
		}
		// Cas d'un nouveau client
		if (FD_ISSET(fdSocket, &tmp)) {
			// on accepte le client
			int fdtmp=accept(fdSocket, NULL, NULL);
			// on ajoutele cli ent à la liste des client et à l'ensemble solide
			ensemblesolide.add(fdtmp);
			string pseudo="L33T_80Y";
			stringstream voila;
			voila <<pseudo<<ensemblesolide.getmax();
			Client* nouveauclient=new Client(fdtmp, voila.str());
			clientsServ.push_back(nouveauclient);
			nouveauclient->sendData(messAcc);
		}
		// creation des deux iterateurs de parcours
		list<Client*>::iterator it=clientsServ.begin();
		list<Client*>::iterator fin=clientsServ.end();
		while(it != fin){
			/* On parcours tous les clients pour savoir si il y a qqch a faire
			  * Lorsqu'il y a quelquechose à faire, on dit au client de lire la commande
			  * puis d'agir
			  */
			if (FD_ISSET((*it)->getFdclient(), &tmp)) {
				(*it)->readCommande();
				(*it)->agir();
			}
			if ((*it)->isAdeconnecter() == true) {
				ensemblesolide.clr((*it)->getFdclient());
				delete (*it);
				it=clientsServ.erase(it);
			}
			else
				++it;
		}
	}
	return 0;
}

map<string, Channel*>::iterator Serveur::addchannel(Client* createur, string channelname, string topic) {
	pair<string, Channel*> mapaire(channelname, new Channel(channelname, topic, createur));
	pair< map<string, Channel*>::iterator, bool> paire;
	paire=nomToChannel.insert(mapaire);
	if (paire.second == false) {
		createur->sendRep(eBadArg, "Un channel possède déjà le nom : "+channelname+" nom, veuillez en choisir un autre");
	}
	else {
		createur->sendRep(success, "Le channel à été créé avec succès.");
	}
	return paire.first;

	/// Mieux mais c++ 2011
	///	mapchannel.emplace(channelname, Channel(channelname, topic, createur));
	///		createur->sendtoprgmclient("Il existe déjà un channel avec le nom : "+channelname);
}

void Serveur::join(Client* cli, string channelName) {
	map<string, Channel*>:: iterator it;
	it=nomToChannel.find(channelName);
	if (it == nomToChannel.end()) {
		it=addchannel(cli, channelName, "Topic vide");
	}
	else {
		nomToChannel[channelName]->addClient(cli);
	}
}

void Serveur::unjoin(Client* cli, string channelname) {
	map<string, Channel*>:: iterator it;
	it=nomToChannel.find(channelname);
	if (it == nomToChannel.end()) {
		cli->sendRep(eBadArg, "Déconnexion impossible : le channel n'existe pas.");
	}
	else {
		if (nomToChannel[channelname]->virerClient(cli) == 0) {
			nomToChannel.erase(it);
		}
	}
}

// MP

void Serveur::sendmsgbynom(Client* envoyeur, string pseudo, string message) {
	list<Client*>::iterator it;
	list<Client*>::iterator fin=clientsServ.end();
	while(it != fin) {
		if ((*it)->getPseudo() == pseudo) {
			(*it)->sendData("MP - "+envoyeur->getPseudo()+" : "+message);
			return;
		}
		else {
			++it;
		}
	}
}
