#include "client.h"
#include "err.codes.h"
#include "serveur.h"

#include <unistd.h>
#include <iostream>
#include <errno.h>
#include <stdio.h>

using namespace std;
using namespace ERR;
using namespace RET;

Client::Client(int socket, string pseudo) :fdClient(socket), pseudo(pseudo)
{
	adeconnecter=false;
	cout << "Construction client " << pseudo << endl;
}

Client::~Client()
{
	close(fdClient);
	cout << "Client détruit " << pseudo << endl;
}

int Client::getNbArg() const
{
	return argsCmd.size();
}

void Client::readCommande()
{
	argsCmd.clear();
	uint16_t tailleTrame;
	string argstmp;
	char buffer[4096]={0}; // il semblerait que l'optimisation fasse en sorte d'utiliser le même buffer d'ou l'initialisation
	int nblu;
	nblu=read(fdClient, &tailleTrame, sizeof(uint16_t));
	if (nblu == 0 ) {
		adeconnecter=true;
		return;
	}
	else if (nblu <= 0) {
		perror("Erreur lecture taille trame :");
		adeconnecter=true;
		return;
	}
	nblu=read(fdClient, &idCmd, sizeof(uint16_t));
	if (nblu <= 0) {
		perror("Erreur lecture identifiant commande :");
		adeconnecter=true;
		return;
	}
	nblu=read(fdClient, &codeCmd_ctos, sizeof(uint8_t));
	if (nblu <= 0) {
		perror("Erreur lecture code commande :");
		adeconnecter=true;
		return;
	}
	nblu=read(fdClient, buffer, tailleTrame-3);
	if (nblu <= 0) {
		adeconnecter=true;
		return;
	}
	else {
		argstmp=buffer;
	}
	vector<string>::iterator it=argsCmd.begin();
	int i=0;
	while (argstmp.length() != 0) {
		argsCmd.insert(it, argstmp.substr(0, argstmp.find("\n")));
		argstmp.erase(0, argsCmd[i].length()+1);
		++it;
		++i;
	}
}

uint8_t Client::getCodecmd() const {
	return codeCmd_ctos;
}

void Client::setCodecmd(uint8_t codeCmd) {
	this->codeCmd_ctos = codeCmd;
}

uint16_t Client::getIdcmd() const {
	return idCmd;
}

void Client::setIdcmd(uint16_t idCmd) {
	this->idCmd= idCmd;
}

void Client::sendRep(uint8_t coderetour, string aenvoyer)
{
	aenvoyer+="\n";
	uint16_t tailleTrame=aenvoyer.size()+3;
	if (write(fdClient, &tailleTrame, sizeof(uint16_t)) == -1) {
		perror("Perror_sendMsg write client");
	}
	if (write(fdClient, &idCmd, sizeof(uint16_t)) == -1) {
		perror("Perror_sendMsg write idcmd");
	}
	if (write(fdClient, &coderetour, sizeof(uint8_t)) == -1) {
		perror("Perror_sendMsg write client");
	}
	if (write(fdClient, aenvoyer.c_str(), aenvoyer.size()) == -1) {
		perror("Perror_sendMsg write client");
	}
}

const vector<string>& Client::getArgsCmd() const {
	return argsCmd;
}

bool Client::isAdeconnecter() const {
	return adeconnecter;
}
void Client::setAdeconnecter(bool adeconnecter) {
	this->adeconnecter = adeconnecter;
}
int Client::getFdClient() const {
	return fdClient;
}
void Client::setFdclient(int fdclient) {
	this->fdClient = fdclient;
}
const string& Client::getPseudo() const {
	return pseudo;
}
void Client::setPseudo(string pseudo) {
	this->pseudo = pseudo;
}




/*************************************/

void Client::agir()
{
	Serveur* srv=Serveur::getInstance();
	/// ANALYSE DE CHAINE COMMANDE
	/// Commande join
	switch (codeCmd_ctos) {
		//code retour spontané 129 aprivmsg : envoi d'un message privé
		//Arg: le nick du client emetteur, le message
		case 1:
			if (getNbArg() < 2) {
				sendRep(eMissingArg);
			}
			else {
				switch (srv->mp(this, argsCmd[0], argsCmd[1])) {
					case success:
						sendRep(success);
						break;
					case eNotExist:
						sendRep(eNotExist);//, argsCmd[0]+"n'est pas un client du serveur.");
					default:
						sendRep(error);//, "Erreur inconnue");
						break;
				}
			}
			break;
		//code retour spontané 128 apubmsg : envoi d'un message par un client à un channel.
		//Args: le nom du channel, le client émetteur, le message.
		case 2:
			if (getNbArg() < 2) {
				sendRep(eMissingArg);
			}
			else {
				switch (srv->msgToChannel(argsCmd[0], argsCmd[1], this)) {
					case success:
						sendRep(success);
						break;
					case eNotExist:
						sendRep(eNotExist);//, argsCmd[0]+"n'est pas un channel du serveur.");
						break;
					default:
						sendRep(error);//, "Erreur inconnue");
						break;
				}
			}
			break;
		//pas de message spontané du serveur pour who
		case 3: {
				if (getNbArg() < 1) {
					sendRep(eMissingArg);
				}
				else {
					string reponse;
					switch (srv->who(&reponse, argsCmd[0])) {
						case success:
							sendRep(success, reponse);
							break;
						case eNotExist:
							sendRep(eNotExist);//, "Aucun client ne correspond à : "+argsCmd[0]);
							break;
						default:
							sendRep(error);//, "Erreur inconnue");
							break;
					}
				}
			}
			break;
		//pas de message spontané du serveur pour who
		case 4: {
				if (getNbArg() < 2) {
					sendRep(eMissingArg);
				}
				else {
					string reponse;
					switch (srv->whoChannel(&reponse, argsCmd[0], argsCmd[1])) {
						case success:
							sendRep(success);//, reponse);
							break;
						case eNotExist:
							sendRep(eNotExist);//, "Aucun channel ne correspond à : "+argsCmd[0]);
							break;
						default:
							sendRep(error);//, "Erreur inconnue");
							break;
					}
				}
			}
			break;
		//pas de message spontané du serveur pour listerChan
		case 5: {
				if (argsCmd[0] == "") {
					argsCmd[0]="*";
					//sendRep(eMissingArg);
				}
				string reponse;
				switch (srv->listerChan(&reponse, argsCmd[0])) {
					case success:
						sendRep(success, reponse);
						break;
					case eNotExist:
						sendRep(eNotExist);//, "Aucun channel ne correspond à : "+argsCmd[0]);
						break;
					default:
						sendRep(error);//, "Erreur inconnue");
						break;
				}
			}
			break;
		//code retour spontané 131 atopic : changement de topic
		//arg: le channel sur lequel le notic a changé, le nouveau topic
		case 6: {
				if (getNbArg() < 2) {
					sendRep(eMissingArg);
				}
				else {
					string reponse;
					switch(srv->changerTopic(argsCmd[0], argsCmd[1], this, &reponse)){
						case success:
							sendRep(success, reponse);
							break;
						case eNotExist:
							sendRep(eNotExist);//, "Aucun channel ne correspond à : "+argsCmd[0]);
							break;
						case eNotAutorized:
							sendRep(eNotAutorized);//, "Vous n'avez pas les droits pour changer le topic du channel");
							break;
						default:
							sendRep(error);//, "Erreur inconnue");
							break;
					}
				}
			}
			break;
		//code retour 134 akick: un utilisateur a été kické
		//Arg: le nom du channel, le nick de l'utilisateur kické, le nick du kickeur
		case 7:
			if (getNbArg() < 2) {
				sendRep(eMissingArg);
			}
			else {
				switch (srv->kickFromChan(argsCmd[0], argsCmd[1], this)) {
					case success:
						sendRep(success);
						break;
					case eNotExist:
						sendRep(eNotExist);//, "Aucun channel avec ce nom ou aucun client de ce nom dans le channel");
						break;
					default:
						sendRep(error);//, "Erreur inconnue");
						break;
				}
			}
			break;
		//code retour 135 aban: un  ban a été ajouté/enlevé
		//Arg: le nom du channel, + pour un ajout, - pour un retrait,la chaine
		//qui correspond au ban (un nick ou un motif)
		case 8: {
				if (getNbArg() < 2) {
					sendRep(eMissingArg);
				}
				else {
					string reponse;
					switch (srv->ban(&reponse, argsCmd[0], argsCmd[1], this)) {
						case success:
							sendRep(success, reponse);
							break;
						case eNotExist:
							sendRep(eNotExist);//, "Aucun client et/ou channel ne correspond à : "+argsCmd[0]+" "+argsCmd[1]);
							break;
						case eMissingArg:
							sendRep(eMissingArg);//, "Erreur, mauvais nombre d'arguments");
							break;
						case eNotAutorized:
							sendRep(eNotAutorized, reponse+"Vous n'avez pas les droits pour bannir, opération incomplète (Avez vous les droits sur TOUS les channels donnés?)");
							break;
						default:
							sendRep(error, reponse+"Erreur inconnue");
							break;
					}
				}
			}
			break;
		/*code retour 130 aop: changement de droit
		Arg: le nick du client qui a changé les droits, le channel sur lequel les
		droits ont changé, les nouveaux droits
		Droits (pour le moment un seul, bcp plus sur un vrai irc):
		o=>op (si pas de o, alors on est pas op)*/
		case 9:
			if (getNbArg() < 2) {
				sendRep(eMissingArg);
			}
			else {
				switch (srv->op(argsCmd[0], argsCmd[1], this)) {
					case success:
						sendRep(success);
						break;
					case eNotExist:
						sendRep(eNotExist);//, "Aucun channel avec ce nom ou aucun client de ce nom dans le channel");
						break;
					case eNotAutorized:
						sendRep(eNotAutorized);//, "Vous n'avez pas les droits pour rendre op quelqu'un dans ce channel");
						break;
					default:
						sendRep(error);//, "Erreur inconnue");
						break;
				}
			}
			break;
		/*code retour 130 aop: changement de droit
		Arg: le nick du client qui a changé les droits, le channel sur lequel les
		droits ont changé, les nouveaux droits
		Droits (pour le moment un seul, bcp plus sur un vrai irc):
		o=>op (si pas de o, alors on est pas op)*/
		case 20:
			if (getNbArg() < 2) {
				sendRep(eMissingArg);
			}
			else {
				switch (srv->deop(argsCmd[0], argsCmd[1], this)) {
					case success:
						sendRep(success);
						break;
					case eNotExist:
						sendRep(eNotExist);//, "Aucun channel avec ce nom ou aucun client de ce nom op dans le channel");
						break;
					case eNotAutorized:
						sendRep(eNotAutorized);//, "Vous n'avez pas les droits pour deop quelqu'un dans ce channel");
						break;
					case eMissingArg:
						sendRep(eMissingArg);//, "Erreur, mauvais nombre d'arguments");
						break;
					default:
						sendRep(error);//, "Erreur inconnue");
						break;
				}
			}
			break;
		//code de retour 137 ajoin : join (un nouveau client a rejoint le channel)
		//Arg: le channel, le nick du client.
		case 21:
			if (getNbArg() < 1) {
				sendRep(eMissingArg);
			}
				else {
				switch (srv->join(this, argsCmd[0])) {
					case success:
						sendRep(success);//, "Vous êtes désormais dans le channel");
						break;
					case eTopicUnset:
						sendRep(success);//, "Un channel a été créé, utilisez la commande topic pour définir le topic");
						break;
					default:
						sendRep(error);//, "Erreur inconnue");
						break;
				}
			}
			break;
		//code de retour 132 anick : changement de nick
		//Arg: l'ancien et le nouveau nick du client
		case 22:
			if (getNbArg() < 1) {
				sendRep(eMissingArg);
			}
			else {
				switch (srv->nick(argsCmd[0],this)) {
					case success:
						sendRep(success);//, "Votre pseudo est désormais : "+argsCmd[0]);
						break;
					case eMissingArg:
						sendRep(eMissingArg);//, "Argument manquant : nouveau pseudo");
						break;
					case eBadArg:
						sendRep(eBadArg);//, "Trop d'arguments, votre login doit etre une suite sans espace de chifres et de lettres");
						break;
					case eNickCollision:
						sendRep(eNickCollision);//, "Un autre client est déjà connecté avec ce pseudo");
						break;
					default:
						sendRep(error);//, "Erreur inconnue");
						break;
				}
			}
			break;
		//code de retour 133 aleave : un utilisateur a quitté le channel
		//Arg: le nom du channel, le nick de l'utilisateur
		case 23:
			cout << "Bienvenu dans le cas 23" << endl;
			if (getNbArg() < 1) {
				sendRep(eMissingArg);
			}
			else {
				switch (srv->unjoin(this, argsCmd[0])) {
					case success:
						sendRep(success);//, "Vous avez quitté le channel"+argsCmd[0]);
						break;
					case eNotExist:
						sendRep(eNotExist);//, "Le channel "+argsCmd[0]+" n'existe pas");
						break;
					default:
						sendRep(error);//, "Erreur inconnue");
						break;
				}
			}
			break;
		//code retour 135 aban: un  ban a été ajouté/enlevé
		//Arg: le nom du channel, + pour un ajout, - pour un retrait,la chaine
		//qui correspond au ban (un nick ou un motif)
		case 24: {
			if(getNbArg() < 2)
				sendRep(eMissingArg);
			else {
				string reponse;
				switch (srv->unban(&reponse, argsCmd[0], argsCmd[1], this)) {
					case success:
						sendRep(success, reponse);
						break;
					case eNotExist:
						sendRep(eNotExist);//, "Aucun client et/ou channel ne correspond à : "+argsCmd[0]+" "+argsCmd[1]);
						break;
					case eMissingArg:
						sendRep(eMissingArg);//, "Erreur, mauvais nombre d'arguments");
						break;
					case eNotAutorized:
						sendRep(eNotAutorized, reponse+"Vous n'avez pas les droits pour débannir, opération incomplète (Avez vous les droits sur TOUS les channels donnés?)");
						break;
					default:
						sendRep(error, reponse+"Erreur inconnue");
						break;
					}
				}
			}
			break;
		//pas de message spontané du serveur pour listerBan
		case 25: {
			if(getNbArg() < 1)
				sendRep(eMissingArg);
			else {
				string reponse;
				switch (srv->listerBan(argsCmd[0], &reponse, this)) {
					case success:
						sendRep(success, reponse);
						break;
					case eNotExist:
						sendRep(eNotExist);//, argsCmd[0]+"n'est pas un channel du serveur.");
					case eMissingArg:
						sendRep(eMissingArg);//, "Argument manquant : nom du channel");
						break;
					case eBadArg:
						sendRep(eBadArg);//, "Trop d'arguments, entrez le nom d'un seul channel");
						break;
					default:
						sendRep(error, reponse+"Erreur inconnue");
						break;
				}
			}
		}
		break;
	}
}

/************************************/
