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

enum {INCONNUE, CREATE_SOCKET, BIND_SOCKET, LISTEN_SOCKET, SELECT_SOCKET};

/********** Design pattern singleton **********/
Serveur* Serveur::_instance=NULL;

Serveur* Serveur::getInstance(){
	if(_instance==0){
		_instance = new Serveur ;
	}
	return _instance;
}
/**********************************************/


/********** Constructeur et Destrcteur **********/
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
/************************************************/


/********** Getteurs et Setteurs **********/
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
/*******************************************/

int Serveur::run()
{
	int nbrclirestant;
	fd_set tmp;
	Ensemble ensemblesolide;
	ensemblesolide.add(fdSocket);
	ensemblesolide.add(STDIN_FILENO);
	for(;;) {
		// On créé un ensemble temporaire pour le donner au select
		tmp=ensemblesolide.getfd_set();
		if ((nbrclirestant=select(ensemblesolide.getmax(), &tmp, NULL, NULL, NULL)) == -1) {
			perror("Erreur select");
			return SELECT_SOCKET;
		}
		// Cas d'un nouveau client
		if (FD_ISSET(fdSocket, &tmp)) {
			// on accepte le client
			int fdClient=accept(fdSocket, NULL, NULL);
			// on ajoutele cli ent à la liste des client et à l'ensemble solide
			ensemblesolide.add(fdClient);
			string pseudo="L33T_80Y";
			stringstream voila;
			voila <<pseudo<<ensemblesolide.getmax();
			Client* nouveauclient=new Client(fdClient, voila.str());
			list<Client*>::iterator it=clientsServ.begin();
			list<Client*>::iterator fin=clientsServ.end();
			while (it!= fin && (*it)->getFdclient() < nouveauclient->getFdclient()) {
				++it;
			}
			clientsServ.insert(it, nouveauclient);
			nouveauclient->sendData(messAcc);
		}
		// cas ou on a quelquechose sur l'entrée standard
		if (FD_ISSET(STDIN_FILENO, &tmp)) {
			string entree;
			cin >> entree;
			if (entree == "quit")
				return 0;
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
	return error;
}

unsigned int Serveur::addchannel(Client* createur, string channelname, string topic) {
	Channel* tmp=new Channel(channelname, topic, createur);
	pair<string, Channel*> mapaire(tmp->getName(), tmp);
	pair< map<string, Channel*>::iterator, bool> paire;
	paire=nomToChannel.insert(mapaire);
	if (paire.second == false) {
		return eChannelnameCollision;
	}
	else {
		return success;
	}
	return error;
}

unsigned int Serveur::join(Client* cli, string channelName) {
	unsigned int retTmp;
	map<string, Channel*>::iterator it;
	it=nomToChannel.find(channelName);
	// Cas ou le channel n'existe pas
	if (it == nomToChannel.end()) {
		switch(retTmp=addchannel(cli, channelName, "Topic vide")){
			case success:
				return eTopicUnset;
			default:
				return retTmp;
		}
	}
	else {
		return nomToChannel[channelName]->addClient(cli);
	}
}

unsigned int Serveur::unjoin(Client* cli, string channelname) {
	unsigned retTmp;
	map<string, Channel*>:: iterator it;
	it=nomToChannel.find(channelname);
	if (it == nomToChannel.end()) {
		return eBadArg;
	}
	else {
		retTmp=nomToChannel[channelname]->virerClient(cli);
		if (nomToChannel[channelname]->getCompt() == 0) {
			nomToChannel.erase(it);
		}
		return retTmp;
	}
	return error;
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
