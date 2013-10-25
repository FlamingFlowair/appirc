#include "channel.h"
#include "err.codes.h"

using namespace ERR;

Channel::Channel(string name, string topic, Client* createur)
	:name(name), topic(topic)
{
	compt=0;
	addClient(createur);
}

unsigned int Channel::getCompt() const {
	return compt;
}

void Channel::setCompt(unsigned int compt) {
	this->compt = compt;
}

string Channel::getName() const {
	return name;
}

void Channel::setName(string name) {
	this->name = name;
}

string Channel::getTopic() const {
	return topic;
}

void Channel::setTopic(string topic) {
	this->topic = topic;
}

void Channel::addClient(Client* newclient) {
	if(!isclient(newclient)){
		clientsChan.push_back(newclient);
		if (compt == 0) {
			opChan.push_back(newclient);
		}
		++compt;
	}
	newclient->sendRep(0, "Vous êtes maintenant client du channel "+name+".");
}

/*
 * Description : supprime un client du chan
 * @oldclient : pointeur sur le client a virer du channel
 * @kicker : pointeur sur l'operateur qui vire le client défaut : NULL
 */
unsigned int Channel::virerClient(Client* oldclient, Client* kicker) {
	list<Client*>::iterator it=clientsChan.begin();
	list<Client*>::iterator fin=clientsChan.end();
	while (it != fin) {
		if ((*it)->getFdclient() == oldclient->getFdclient()) {
			if (kicker != NULL && isop(kicker) == true) {
				oldclient->sendData("Vous n'êtes plus dans le channel : "+name+" car "+kicker->getPseudo()+" (operateur du channel) vous a kické du channel");
				--compt;
				it=clientsChan.erase(it);
				return compt;
			}
			else if (kicker != NULL && isop(kicker) == false) {
				kicker->sendRep(eNotAutorized, "Vous n'êtes pas operateur du channel, kick impossible");
				return compt;
			}
			else {
				oldclient->sendRep(success, "Vous n'êtes plus dans le channel : "+name+".");
				--compt;
				it=clientsChan.erase(it);
				return compt;
			}
		}
		else {
			++it;
		}
	}
	return compt;
}

/*
 * Description : Ajoute un operateur au chan si il est déjà client au chan
 * @newop : pointeur sur le client a ajouter à la liste d'operateur
 */
void Channel::addop(Client* oldop, Client* newop) {
	if  (isop(oldop) == false) {
		oldop->sendRep(eNotAutorized, "Hé mec t'est pas opérateur kes tu fé, t'as pas les droits");
		return;
	}
	if (isclient(newop)) {
		opChan.push_back(newop);
	}
	else {
		oldop->sendRep(eNotExist, newop->getPseudo()+"doit être client du channel pour que cela fonctionne.");
	}
}

/*
 * Description : Supprime un operateur de la liste d'operateur
 * @oldclient : pointeur sur le client a virer de la liste du channel
 * @kicker : pointeur sur l'operateur qui vire le client défaut : NULL
 */
void Channel::virerop(Client* oldop, Client* kicker) {
	list<Client*>::iterator it=opChan.begin();
	list<Client*>::iterator fin=opChan.end();
	while (it != fin) {
		if ((*it)->getFdclient() == oldop->getFdclient()) {
			if (kicker != NULL && isop(kicker)) {
				send(kicker , oldop->getPseudo()+" n'est plus operateur du channel : "+name+" car "+kicker->getPseudo()+"l'a supprimé de la liste d'operateurs.");
				it=opChan.erase(it);
			}
			else if (kicker != NULL && isop(kicker) == false) {
				kicker->sendRep(eNotAutorized, "Vous n'êtes pas operateur du channel, kick d'operateur impossible");
				return;
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
