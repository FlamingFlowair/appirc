#include "channel.h"
#include "err.codes.h"
using namespace ERR;

#include <iostream>
using namespace std;
// include des regex
#include <regex>
#include <iterator>

Channel::Channel(string name, string topic, Client* createur)
	:name(name), topic(topic)
{
	compt=0;
	addClient(createur);
	cout << "Création channel : " << name << endl;
}

unsigned int Channel::getCompt() const {
	return compt;
}

void Channel::setCompt(unsigned int compt) {
	this->compt = compt;
}

const string& Channel::getName() const {
	return name;
}

void Channel::setName(string name) {
	this->name = name;
}

const string & Channel::getTopic() const {
	return topic;
}

void Channel::setTopic(string topic) {
	this->topic = topic;
}

unsigned int Channel::addClient(Client* newclient) {
	if(!isclient(newclient)){
		clientsChan.push_back(newclient);
		if (compt == 0) {
			opChan.push_back(newclient);
		}
		++compt;
	}
	return success;
}

/*
 * Description : supprime un client du chan
 * @oldclient : pointeur sur le client a virer du channel
 * @kicker : pointeur sur l'operateur qui vire le client défaut : NULL
 */
unsigned int Channel::virerClient(string patternOldClient, Client* kicker) {
	unsigned int i=0;
	if (kicker != NULL && isop(kicker) == false) {
		return eNotAutorized;
	}
	list<Client*>::iterator it=clientsChan.begin();
	list<Client*>::iterator fin=clientsChan.end();
	while (it != fin) {
		if (regex_match((*it)->getPseudo(), regex(patternOldClient))) {
			if (kicker != NULL) {
				(*it)->sendData("Vous n'êtes plus dans le channel : "+name+" car "+kicker->getPseudo()+" (operateur du channel) vous a kické du channel");
				if (isop(*it)){
					virerop(*it);
				}
				it=clientsChan.erase(it);
				--compt;
				++i;
			}
			else {
				if (isop(*it)){
					virerop(*it);
				}
				it=clientsChan.erase(it);
				--compt;
				++i;
			}
		}
		else {
			++it;
		}
	}
	if (i == 0) {
		return eNotExist;
	}
	return success;
}

/*
 * Description : Ajoute un operateur au chan si il est déjà client au chan
 * @newop : pointeur sur le client a ajouter à la liste d'operateur
 */
unsigned int Channel::addop(Client* oldop, Client* newop) {
	if  (isop(oldop) == false) {
		return eNotAutorized;
	}
	if (isclient(newop)) {
		opChan.push_back(newop);
		return success;
	}
	else {
		return eNotExist;
	}
}

/*
 * Description : Supprime un operateur de la liste d'operateur
 * @oldclient : pointeur sur le client a virer de la liste du channel
 * @kicker : pointeur sur l'operateur qui vire le client défaut : NULL
 */
unsigned int Channel::virerop(Client* oldop, Client* kicker) {
	list<Client*>::iterator it=opChan.begin();
	list<Client*>::iterator fin=opChan.end();
	while (it != fin) {
		if ((*it)->getFdclient() == oldop->getFdclient()) {
			if (kicker != NULL && isop(kicker)) {
				send(kicker , oldop->getPseudo()+" n'est plus operateur du channel : "+name+" car "+kicker->getPseudo()+"l'a supprimé de la liste d'operateurs.");
				it=opChan.erase(it);
			}
			else if (kicker != NULL && isop(kicker) == false) {
				return eNotAutorized;
			}
			else {
				send(oldop , oldop->getPseudo()+" n'est plus operateur du channel : "+name+" car il renonce à ses droits divins.");
				it=opChan.erase(it);
			}
		}
		else {
			++it;
		}
	}
	return error;
}

/*
 * Description : return true si un client est dans la liste des operateurs du channel
 * @clitotest : pointeur sur le client à tester
 */
bool Channel::isop(Client* clitotest) {
	list<Client*>::iterator it=opChan.begin();
	list<Client*>::iterator fin=opChan.end();
	while (it != fin) {
		if ((*it)->getFdclient() == clitotest->getFdclient()) {
			return true;
		}
		else {
			++it;
		}
	}
	return false;
}

/*
 * Description : return true si un client est dans la liste des clients du channel
 * @clitotest : pointeur sur le client à tester
 */
bool Channel::isclient(Client* clitotest) {
	list<Client*>::iterator it=clientsChan.begin();
	list<Client*>::iterator fin=clientsChan.end();
	while (it != fin) {
		if ((*it)->getFdclient() == clitotest->getFdclient()) {
			return true;
		}
		else {
			++it;
		}
	}
	return false;
}

/*
 * Description : Envoie un message à tous les clients du channel
 * @massage : Message à envoyer
 */
void Channel::send(Client* envoyeur, string message) {
	list<Client*>::iterator it=clientsChan.begin();
	list<Client*>::iterator fin=clientsChan.end();
	while (it != fin) {
		(*it)->sendData(envoyeur->getPseudo()+" : "+message);
		++it;
	}
}
