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

void Channel::addBan(string pseudo)
{
	list<string>::iterator it=find(bannis.begin(), bannis.end(), pseudo);
	if(it == bannis.end())
		bannis.push_front(pseudo);
}

void Channel::removeBan(string pseudo)
{
	list<string>::iterator it = find(bannis.begin(), bannis.end(), pseudo);
	if(it != bannis.end())
		bannis.erase(it);
}

unsigned int Channel::addClient(Client* newclient) {
	if(!isclient(newclient)){
		if(bannis.end() == find(bannis.begin(), bannis.end(), newclient->getPseudo()))
			clientsChan.push_back(newclient);
		else return eNotAutorized; //cas d'un black listé
		if (compt == 0) {
			opChan.push_back(newclient);
		}
		++compt;
	}
	return success;
}

//premiere fonction virerClient()
//est-ce que il ne faudrait pas le virer aussi de list op? Si.
unsigned int Channel::virerClient(Client* oldclient) {
	list<Client*>::iterator it=clientsChan.begin();
	list<Client*>::iterator fin=clientsChan.end();
	while (it != fin) {
		if ((*it)->getFdclient() == oldclient->getFdclient()) {
			if(isop(*it))
				virerop(*it,NULL);
			else return eNotAutorized;
			--compt;
			it=clientsChan.erase(it);
			return success;
		}
		else {
			++it;
		}
	}
	return error;
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
unsigned int Channel::addop(Client* oldop, string newop) {
	Client* newopptr;
	if  (isop(oldop) == false) {
		return eNotAutorized;
	}
	if (isclient(newop)) {
		opChan.push_back(newopptr);
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
 * Description : return true si un client est dans la liste des clients du channel
 * @clitotest : string : pseudo du client à tester
 */
Client* Channel::isclient(string pseudo) {
	list<Client*>::iterator it=clientsChan.begin();
	list<Client*>::iterator fin=clientsChan.end();
	while (it != fin) {
		if ((*it)->getPseudo() == pseudo) {
			return (*it);
		}
		else {
			++it;
		}
	}
	return NULL;
}

/*
 * Description : Envoie un message à tous les clients du channel
 * @message : Message à envoyer
 * Dans le cas ou envoyeur vaut NULL : message général du serveur au channel
 */
void Channel::send(Client* envoyeur, string message) {
	list<Client*>::iterator it=clientsChan.begin();
	list<Client*>::iterator fin=clientsChan.end();
	if(envoyeur != NULL)
		while (it != fin) {
			(*it)->sendData(envoyeur->getPseudo()+" : "+message);
			++it;
		}
	else
		while (it != fin) {
			(*it)->sendData("Serveur : "+message);
			++it;
		}
}

unsigned int Channel::who(string* msgtosend, string pattern) const {
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
	//Cette ligne ajoute simplement le nom du channel avant la liste des nicks
	//histoire de s'y retrouver.
	*msgtosend=(*msgtosend)+name+" : \n";
	list<Client*>::const_iterator it=clientsChan.begin();
	list<Client*>::const_iterator fin=clientsChan.end();
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

unsigned int Channel::ban(string *reponse, string pattern, Client * envoyeur, int *nbBannis)
{
	string regpattern;
	size_t place;
	int memBannis= *nbBannis;
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
	//Cette ligne ajoute simplement le nom du channel avant la liste des nicks
	//histoire de s'y retrouver.
	*reponse=(*reponse)+"Bannis de "+name+" : \n";
	list<Client*>::const_iterator it=clientsChan.begin();
	list<Client*>::const_iterator fin=clientsChan.end();
	for(; it!=fin; ++it) {
		if (regex_match((*it)->getPseudo(), regex(regpattern))) {
			*reponse=(*reponse)+((*it)->getPseudo())+"\n";
			//virer le banni et l'ajouter à la black list
			if(virerClient((*it)->getPseudo(), envoyeur) == eNotAutorized){
				*reponse =(*reponse)+"Vous n'avez pas de droits sur le channel : "+name+"\n";
				return eNotAutorized;
			}
			addBan((*it)->getPseudo());
			*nbBannis++;
		}
	}
	if (*nbBannis == memBannis) {
		return eNotExist;
	}
	return success;
}

unsigned int Channel::unban(string *reponse, string pattern, Client *envoyeur, int *nbDebannis)
{
	string regpattern;
	size_t place;
	int memDebannis= *nbDebannis;
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
	//Cette ligne ajoute simplement le nom du channel avant la liste des nicks
	//histoire de s'y retrouver.
	*reponse=(*reponse)+"Débannis de "+name+" : \n";
	list<Client*>::const_iterator it=clientsChan.begin();
	list<Client*>::const_iterator fin=clientsChan.end();
	for(; it!=fin; ++it) {
		if (regex_match((*it)->getPseudo(), regex(regpattern))) {
			*reponse=(*reponse)+((*it)->getPseudo())+"\n";
			//enlever le banni de la black list
			if(isop(envoyeur) == false)
				return eNotAutorized;
			removeBan((*it)->getPseudo());
			*nbDebannis++;
		}
	}
	if (*nbDebannis == memDebannis) {
		return eNotExist;
	}
	return success;
}

void Channel::listBan(string *reponse)
{
	list<string>::const_iterator it=bannis.begin();
	list<string>::const_iterator fin=bannis.end();
	for(; it!=fin; ++it) {
		*reponse=(*reponse)+(*it)+"\n";
	}
}
