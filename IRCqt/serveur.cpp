#include "serveur.h"

// perror
#include <stdio.h>
#include <errno.h>
// close
#include <unistd.h>
// socket select accept
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>

#include <netdb.h>
// include du exit
#include <stdlib.h>
// include du cout
#include <iostream>
// include des stream string
#include <sstream>
// include des regex
#include <regex>
#include <iterator>

#include "err.codes.h"

using namespace ERR;
using namespace RET;
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
	if (::bind(fdSocket, (struct sockaddr *)&sa, sizeof(struct sockaddr)) == -1){
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
const string& Serveur::getHostname() const {
	return hostname;
}
void Serveur::setHostname(string hostname) {
	this->hostname = hostname;
}
const string& Serveur::getMessageacc() const {
	return messAcc;
}
void Serveur::setMessageacc(string messageacc) {
	this->messAcc = messageacc;
}
const string& Serveur::getNom() const {
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

unsigned int Serveur::getmaxfd()
{
	unsigned int maxtmp=fdSocket;
	list<Client*>::const_iterator it=clientsServ.begin();
	list<Client*>::const_iterator fin=clientsServ.end();
	for(; it != fin; ++it){
		if ((unsigned int)(*it)->getFdClient() > maxtmp)
			maxtmp=(*it)->getFdClient();
	}
	return maxtmp;
}
/*******************************************/

int Serveur::run()
{
	int nbrclirestant;
	fd_set ensemblesolide, tmp;
	FD_SET(fdSocket, &ensemblesolide);
	FD_SET(STDIN_FILENO, &ensemblesolide);
	for(;;) {
		// On créé un ensemble temporaire pour le donner au select
		tmp=ensemblesolide;
		if ((nbrclirestant=select(getmaxfd()+1, &tmp, NULL, NULL, NULL)) == -1) {
			perror("Erreur select");
			return SELECT_SOCKET;
		}
		// Cas d'un nouveau client
		if (FD_ISSET(fdSocket, &tmp)) {
			// on accepte le client
			int fdClient=accept(fdSocket, NULL, NULL);
			// on ajoutele cli ent à la liste des client et à l'ensemble solide
			FD_SET(fdClient, &ensemblesolide);
			string pseudo="L33T_80Y";
			stringstream voila;
			voila << pseudo << getmaxfd();
			Client* nouveauclient=new Client(fdClient, voila.str());
			list<Client*>::iterator it=clientsServ.begin();
			list<Client*>::iterator fin=clientsServ.end();
			while (it!= fin && (*it)->getFdClient() < nouveauclient->getFdClient()) {
				++it;
			}
			clientsServ.insert(it, nouveauclient);
			nouveauclient->sendRep(rwall, messAcc);
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
			if (FD_ISSET((*it)->getFdClient(), &tmp)) {
				(*it)->readCommande();
				(*it)->agir();
			}
			if ((*it)->isAdeconnecter() == true) {
				FD_CLR((*it)->getFdClient(), &ensemblesolide);
				delete (*it);
				it=clientsServ.erase(it);
			}
			else {
				++it;
			}
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
	else {//ici : ajouter verif liste des bannis
		return nomToChannel[channelName]->addClient(cli);
	}
}

unsigned int Serveur::unjoin(Client* cli, string channelName) {
	cout << "Bienvenu dans le unjoin" << endl;
	unsigned int retTmp;
	map<string, Channel*>::iterator it;
	it=nomToChannel.find(channelName);
	if (it == nomToChannel.end()) {
		return eNotExist;
	}
	else {
		cout << "Bon, a priori on trouve le channel" << endl;
		retTmp=nomToChannel[channelName]->virerClient(cli);
		cout << "On a viré le client" << endl;
		nomToChannel[channelName]->send(NULL, channelName+"\n"+cli->getPseudo(),aleave);
		if (nomToChannel[channelName]->getCompt() == 0) {
			nomToChannel.erase(it);
		}
		return retTmp;
	}
	return error;
}

// MP
unsigned int Serveur::mp(Client* envoyeur, string pseudo, string message) {
	list<Client*>::iterator it=clientsServ.begin();
	list<Client*>::iterator fin=clientsServ.end();
	while(it != fin) {
		if ((*it)->getPseudo() == pseudo) {
			(*it)->sendRep(aprivmsg, envoyeur->getPseudo()+"\n"+message+"\n");
			return success;
		}
		else {
			++it;
		}
	}
	return eNotExist;
}

unsigned int Serveur::who(string* msgtosend, string pattern) const {
	string regpattern;
	size_t place;
	// On remplace tous les * en .* pour correspondre aux regex C++
	while ( pattern.length() != 0) {
		if ( (place=pattern.find("*")) != pattern.npos) {
			regpattern+=pattern.substr(0, place)+".*";
			pattern.erase(0, place+1);
		}
		else {
			regpattern+=pattern;
			pattern.erase(0);
		}
	}
	list<Client*>::const_iterator it=clientsServ.begin();
	list<Client*>::const_iterator fin=clientsServ.end();
	for(; it!=fin; ++it) {
		if (regex_match((*it)->getPseudo(), regex(regpattern))) {
			*msgtosend=(*msgtosend)+((*it)->getPseudo())+"\n";
		}
	}
	if (msgtosend->length() == 0) {
		return eNotExist;
	}
	return success;
}

unsigned int Serveur::listerChan(string* msgtosend, string patternChannelName) const {
	string regpattern;
	size_t place;
	// On remplace tous les * en .* pour correspondre aux regex C++
	while ( patternChannelName.length() != 0) {
		if ( (place=patternChannelName.find("*")) != patternChannelName.npos) {
			regpattern+=patternChannelName.substr(0, place)+".*";
			patternChannelName.erase(0, place+1);
		}
		else {
			regpattern+=patternChannelName;
			patternChannelName.erase(0);
		}
	}
	map<string, Channel*>::const_iterator it=nomToChannel.begin();
	map<string, Channel*>::const_iterator fin=nomToChannel.end();
	for(; it!=fin; ++it) {
		if (regex_match(it->first, regex(regpattern))) {
			*msgtosend=(*msgtosend)+it->first+"\n"+(it->second->getTopic())+"\n";
		}
	}
	if (msgtosend->length() == 0) {
		return eNotExist;
	}
	return success;
}

unsigned int Serveur::kickFromChan(string channelName, string patternPseudo, Client* kicker) {
	string regpattern;
	size_t place;
	map<string, Channel*>:: iterator it;
	it=nomToChannel.find(channelName);
	// Cas ou le channel n'existe pas
	if (it == nomToChannel.end()) {
		return eNotExist;
	}
	// On remplace tous les * en .* pour correspondre aux regex C++
	while ( patternPseudo.length() != 0) {
		if ( (place=patternPseudo.find("*")) != patternPseudo.npos) {
			regpattern+=patternPseudo.substr(0, place)+".*";
			patternPseudo.erase(0, place+1);
		}
		else {
			regpattern+=patternPseudo;
			patternPseudo.erase(0);
		}
	}
	return nomToChannel[channelName]->virerClient(regpattern, kicker);
}

unsigned int Serveur::op(string channelName, string pseudo, Client* opper) {
	map<string, Channel*>:: iterator itChannel;
	itChannel=nomToChannel.find(channelName);
	// Cas ou le channel n'existe pas
	if (itChannel == nomToChannel.end()) {
		return eNotExist;
	}
	return nomToChannel[channelName]->addop(opper, pseudo);
}


unsigned int Serveur::msgToChannel(string channelName, string msg, Client* envoyeur)
{
	cout << channelName << " " << msg << " " << envoyeur->getPseudo() << endl;
	map<string, Channel*>::const_iterator itChannel;
	itChannel=nomToChannel.find(channelName);
	// Cas ou le channel n'existe pas
	if (itChannel == nomToChannel.end()) {
		return eNotExist;
	}
	nomToChannel[channelName]->send(envoyeur, msg, apubmsg);
	return success;
}


unsigned int Serveur::whoChannel(string* msgtosend, string patternChan, string patternClient) const {
	string regpattern;
	size_t place;
	cout<<"1";
	// On remplace tous les * en .* pour correspondre aux regex C++
	while ( patternChan.length() != 0) {
		if ( (place=patternChan.find("*")) != patternChan.npos) {
			regpattern+=patternChan.substr(0, place)+".*";
			patternChan.erase(0, place+1);
		}
		else {
			regpattern+=patternChan;
			patternChan.erase(0);
		}
	}
	cout<<"2";
	map<string, Channel*>::const_iterator it=nomToChannel.begin();
	for(; it!=nomToChannel.end(); ++it) {
		if (regex_match(it->first, regex(regpattern))) {
			cout<<"3";
			it->second->who(msgtosend, patternClient);
			cout<<"4";
		}
	}
	cout<<"5";
	if (msgtosend->length() == 0) {
		cout<<"6";
		return eNotExist;
	}
	cout<<"7";
	return success;
}

unsigned int Serveur::changerTopic(string channelName, string newTopic, Client * envoyeur, string * reponse)
{
	map<string, Channel*>:: iterator itChannel;
	itChannel=nomToChannel.find(channelName);
	// Cas ou le channel n'existe pas
	if (itChannel == nomToChannel.end())
		return eNotExist;
	if(newTopic == ""){
		*reponse==nomToChannel[channelName]->getTopic();
		return success;
	}
	if(nomToChannel[channelName]->isop(envoyeur) == false)
		return eNotAutorized;
	nomToChannel[channelName]->setTopic(newTopic);
	//pas de \n ici à la fin puisque il est ajouté dans send()
	nomToChannel[channelName]->send(NULL,channelName+"\n"+newTopic, atopic);
	*reponse="";
	return success;
}


//Par ordre croissant de code commande, ban est la premiere à vérifier le nombre
//d'arguments? A vérifier -->non, les autres commandes avant devront aussi etre modifiées
unsigned int Serveur::ban(string *reponse, string patternChan, string patternPseudo, Client *envoyeur)
{
	string regpattern;
	size_t place;
	// On remplace tous les * en .* pour correspondre aux regex C++
	while ( patternChan.length() != 0) {
		if ( (place=patternChan.find("*")) != patternChan.npos) {
			regpattern+=patternChan.substr(0, place)+".*";
			patternChan.erase(0, place+1);
		}
		else {
			regpattern+=patternChan;
			patternChan.erase(0);
		}
	}
	//recherche sur tous les chan dans le pattern
	int nbBannis = 0;
	map<string, Channel*>::const_iterator it=nomToChannel.begin();
	for(; it!=nomToChannel.end(); ++it) {
		if (regex_match(it->first, regex(regpattern))) {
			//appel de la methode ban du channel
			if(it->second->ban(reponse, patternPseudo, envoyeur, &nbBannis) == eNotAutorized)
				return eNotAutorized;
		}
	}
	if (nbBannis == 0) {
		return eNotExist;
	}
	return success;
}

//Dans op() et deop() : le parcours de toute la liste client serveur, c'est un peu nul
//le mieux ce serait dans addop et virerop de faire un parcours de opChan
unsigned int Serveur::deop(string channelName, string pseudo, Client *deopper)
{
	map<string, Channel*>:: iterator itChannel;
	itChannel=nomToChannel.find(channelName);
	// Cas ou le channel n'existe pas
	if (itChannel == nomToChannel.end()) {
		return eNotExist;
	}
	list<Client*>::iterator it=clientsServ.begin();
	list<Client*>::iterator fin=clientsServ.end();
	while(it != fin) {
		if ((*it)->getPseudo() == pseudo) {
			return nomToChannel[channelName]->virerop((*it) , deopper);
		}
		else {
			++it;
		}
	}
	return eNotExist;
}

unsigned int Serveur::nick(string newpseudo, Client * envoyeur)
{
	list<Client*>::const_iterator it=clientsServ.begin();
	list<Client*>::const_iterator fin=clientsServ.end();
	for(; it != fin; ++it) {
		if((*it)->getPseudo() == newpseudo)
			return eNickCollision;
		}
	string oldpseudo=envoyeur->getPseudo();
	envoyeur->setPseudo(newpseudo);
	//envoyer la bonne nouvelle à tous les channels où est le client? ou a tout le serveur ?
	//allez, tout le serveur
	it=clientsServ.begin();
	for(; it != fin; ++it) {
		(*it)->sendRep(132, oldpseudo+"\n"+newpseudo+"\n");
		}
	return success;
}

unsigned int Serveur::unban(string *reponse, string patternChan, string patternPseudo, Client *envoyeur)
{
	string regpattern;
	size_t place;
	// On remplace tous les * en .* pour correspondre aux regex C++
	while ( patternChan.length() != 0) {
		if ( (place=patternChan.find("*")) != patternChan.npos) {
			regpattern+=patternChan.substr(0, place)+".*";
			patternChan.erase(0, place+1);
		}
		else {
			regpattern+=patternChan;
			patternChan.erase(0);
		}
	}
	//recherche sur tous les chan dans le pattern
	int nbDebannis = 0;
	map<string, Channel*>::const_iterator it=nomToChannel.begin();
	for(; it!=nomToChannel.end(); ++it) {
		if (regex_match(it->first, regex(regpattern))) {
			//appel de la methode unban du channel
			if(it->second->unban(reponse, patternPseudo, envoyeur, &nbDebannis) == eNotAutorized)
				return eNotAutorized;
		}
	}
	if (nbDebannis == 0) {
		return eNotExist;
	}
	return success;
}

unsigned int Serveur::listerBan(string channelName, string *reponse, Client *envoyeur)
{
	map<string, Channel*>:: iterator itChannel;
	itChannel = nomToChannel.find(channelName);
	// Cas ou le channel n'existe pas
	if (itChannel == nomToChannel.end()) {
		return eNotExist;
	}
	nomToChannel[channelName]->listBan(reponse);
	return success;
}

unsigned int Serveur::broadcastmsg(string message) const {
	list<Client*>::const_iterator it=clientsServ.begin();
	list<Client*>::const_iterator fin=clientsServ.end();
	for(; it != fin; ++it) {
		(*it)->sendRep(136, message+"\n");
	}
	return success;
}
