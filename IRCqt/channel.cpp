/*						 _________________
 *				\|/		/				  \
 *			 /'''''''\ /	Uhhhhhhhh !!  |
 *			 | °  °	 | ''\________________/
 *			 |		 |
 *		nnn	 | `---	 |  nnn
 * ''''''''''''''''''''''''''''
*/

#include "channel.h"
#include "codes.h"
using namespace ERR;
using namespace RET;

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
	cout<<"Channel::addBan"<<endl;
	list<string>::iterator it=find(bannis.begin(), bannis.end(), pseudo);
	if(it == bannis.end())
		bannis.push_front(pseudo);
}

void Channel::removeBan(string pseudo)
{
	cout<<"Channel::removeBan"<<endl;
	list<string>::iterator it = find(bannis.begin(), bannis.end(), pseudo);
	if(it != bannis.end())
		bannis.erase(it);
}

unsigned int Channel::addClient(Client* newclient) {
	cout<<"Channel::addClient"<<endl;
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
	cout<<"Channel::virerClient(Client*)"<<endl;
	list<Client*>::iterator it=clientsChan.begin();
	list<Client*>::iterator fin=clientsChan.end();
	while (it != fin) {
		if ((*it)->getFdClient() == oldclient->getFdClient()) {
			if(isop(*it))
				virerop(*it, *it);
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
	cout<<"Channel::virerClient(string, Client*)"<<endl;
	unsigned int i=0;
	if (kicker != NULL && isop(kicker) == false) {
		return eNotAutorized;
	}
	list<Client*>::iterator it=clientsChan.begin();
	list<Client*>::iterator fin=clientsChan.end();
	while (it != fin) {
		if (regex_match((*it)->getPseudo(), regex(patternOldClient))) {
			if (kicker != NULL) {
				send(name+"\n"+(*it)->getPseudo()+"\n"+kicker->getPseudo()+"\n", akick);
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
	cout<<"Channel::addop"<<endl;
	Client* newopptr;
	if  (isop(oldop) == false) {
		return eNotAutorized;
	}
	newopptr=isclient(newop);
	if (newopptr != NULL) {
		opChan.push_back(newopptr);
		newopptr->sendRep(aop, oldop->getPseudo()+"\n"+name+"o");
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
	cout<<"Channel::virerop"<<endl;
	list<Client*>::iterator it=opChan.begin();
	list<Client*>::iterator fin=opChan.end();
	/*code de retour 130 aop : changement de droit
	Arg: le nick du client qui a changé les droits, le channel sur lequel les
	droits ont changé, les nouveaux droits
	Droits (pour le moment un seul, bcp plus sur un vrai irc):
	o=>op (si pas de o, alors on est pas op)*/
	while (it != fin) {
		if ((*it)->getFdClient() == oldop->getFdClient()) {
			if (kicker != NULL && isop(kicker)) {
				(*it)->sendRep(aop, kicker->getPseudo()+"\n"+name+"\n");
				it=opChan.erase(it);
				return success;
			}
			else if (kicker != NULL && isop(kicker) == false) {
				return eNotAutorized;
			}
			else {//kicker is NULL
				(*it)->sendRep(aop, oldop->getPseudo()+"\n"+name+"\n");
				it=opChan.erase(it);
				return success;
			}
		}
		else {
			++it;
		}
	}
	return eNotExist;
}
/*
 * Description : Supprime un operateur de la liste d'operateur
 * @oldclient : pointeur sur le client a virer de la liste du channel
 * @kicker : pointeur sur l'operateur qui vire le client défaut : NULL
 */
unsigned int Channel::virerop(string pseudo, Client* kicker) {
	cout<<"Channel::virerop"<<endl;
	list<Client*>::iterator it=opChan.begin();
	list<Client*>::iterator fin=opChan.end();
	/*code de retour 130 aop : changement de droit
	Arg: le nick du client qui a changé les droits, le channel sur lequel les
	droits ont changé, les nouveaux droits
	Droits (pour le moment un seul, bcp plus sur un vrai irc):
	o=>op (si pas de o, alors on est pas op)*/
	while (it != fin) {
		if ((*it)->getPseudo() == pseudo) {
			if (kicker != NULL && isop(kicker)) {
				(*it)->sendRep(aop, kicker->getPseudo()+"\n"+name+"\n");
				it=opChan.erase(it);
				return success;
			}
			else if (kicker != NULL && isop(kicker) == false) {
				return eNotAutorized;
			}
			else {//kicker is NULL
				(*it)->sendRep(aop, pseudo+"\n"+name+"\n");
				it=opChan.erase(it);
				return success;
			}
		}
		else {
			++it;
		}
	}
	return eNotExist;
}

/*
 * Description : return true si un client est dans la liste des operateurs du channel
 * @clitotest : pointeur sur le client à tester
 */
bool Channel::isop(Client* clitotest) {
	list<Client*>::iterator it=opChan.begin();
	list<Client*>::iterator fin=opChan.end();
	while (it != fin) {
		if ((*it)->getFdClient() == clitotest->getFdClient()) {
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
	cout<<"Channel::is client(Client *)"<<endl;
	list<Client*>::iterator it=clientsChan.begin();
	list<Client*>::iterator fin=clientsChan.end();
	while (it != fin) {
		if ((*it)->getFdClient() == clitotest->getFdClient()) {
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
	cout<<"Channel::isclient(string)"<<endl;
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
 */
void Channel::send(string message, unsigned int coderet) {
	cout<<"Channel::send"<<endl;
	list<Client*>::iterator it=clientsChan.begin();
	list<Client*>::iterator fin=clientsChan.end();
		while (it != fin) {
			(*it)->sendRep(coderet, message+"\n");
			++it;
		}
}

unsigned int Channel::who(string* msgtosend, string pattern) const {
	cout<<"Channel::who"<<endl;
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
	cout<<"Channel::ban"<<endl;
	string regpattern;
	size_t place;
	unsigned int retour = eNotExist;
	int memBannis= *nbBannis;
	string nomDuBanni ;
	int flag=0;
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
	*reponse=(*reponse)+"Clients concernés sur le channel "+name+" : \n";
	list<Client*>::const_iterator it=clientsChan.begin();
	list<Client*>::const_iterator itmem;
	list<Client*>::const_iterator fin=clientsChan.end();
	while(it!=fin){
		itmem=it;
		itmem++;
		if (regex_match((*it)->getPseudo(), regex(regpattern))) {
			flag++;
			nomDuBanni=(*it)->getPseudo();
			*reponse=(*reponse)+nomDuBanni+" \n";
			//virer le banni et l'ajouter à la black list
			retour=virerClient(nomDuBanni, envoyeur);
			if(retour == eNotAutorized){
				*reponse =(*reponse)+" Vous n'avez pas de droits sur le channel : "+name+"\n";
				return eNotAutorized;
			}
			/// cette ligne prévient le channel a chaque kick
			/*135: un  ban a été ajouté/enlevé
			Arg: le nom du channel, + pour un ajout, - pour un retrait,la chaine
			qui correspond au ban (un nick ou un motif)*/
			send(name+"\n"+"-"+pattern, aban);
			addBan(nomDuBanni);
			(*nbBannis)++;
		}
		if(flag){
			if(retour == success)
				it=itmem;
			else
				it++;
		}
		else it++;
		flag = 0;
	}
	if (*nbBannis == memBannis) {
		return eNotExist;
	}
	return success;
}

unsigned int Channel::unban(string *reponse, string pattern, Client *envoyeur, int *nbDebannis)
{
	cout<<"Channel::unban"<<endl;
	string regpattern;
	size_t place;
	int memDebannis= *nbDebannis;
	string nomDuBanni ;
	int flag=0;
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
	list<string>::const_iterator it = bannis.begin();
	//mettre un it tmp memoire pour eviter segfault
	list<string>::const_iterator itmem;
	list<string>::const_iterator fin = bannis.end();
	while(it!=fin){
		itmem=it;
		itmem++;
		if (regex_match(*it, regex(regpattern))) {
			flag++;
			*reponse=(*reponse)+(*it)+"\n";
			//enlever le banni de la black list
			if(isop(envoyeur) == false)
				return eNotAutorized;
			removeBan((*it));
			(*nbDebannis)++;
		}
		if(flag)
				it=itmem;
		else
				it++;
		flag = 0;
	}
	if (*nbDebannis == memDebannis) {
		return eNotExist;
	}
	return success;
}

void Channel::listBan(string *reponse)
{
	cout<<"Channel::listBan"<<endl;
	list<string>::const_iterator it=bannis.begin();
	list<string>::const_iterator fin=bannis.end();
	for(; it!=fin; ++it) {
		*reponse=(*reponse)+(*it)+"\n";
	}
}
