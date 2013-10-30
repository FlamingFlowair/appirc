#include "client.h"
#include "err.codes.h"
#include "serveur.h"

#include <unistd.h>
#include <iostream>
#include <errno.h>
#include <stdio.h>

using namespace std;
using namespace ERR;

Client::Client(int socket, string pseudo) :fdSocket(socket), pseudo(pseudo)
{
	adeconnecter=false;
	cout << "Construction client " << pseudo << endl;
}

/// Le pseudo ne s'affiche pas, wtf ?
Client::~Client()
{
	sendData("Vous avez été déconnecté du serveur");
	close(fdSocket);
	cout << "Client détruit " << pseudo << endl;
}

void Client::readCommande()
{
	uint16_t tailleTrame;
	string argstmp;
	char buffer[4096]={0}; // il semblerait que l'optimisation fasse en sorte d'utiliser le même buffer d'ou l'initialisation
	int nblu;
	nblu=read(fdSocket, &tailleTrame, sizeof(uint16_t));
	if (nblu < 0) {
		perror("Erreur lecture taille trame.");
		adeconnecter=true;
		return;
	}
	nblu=read(fdSocket, &idCmd, sizeof(uint16_t));
	if (nblu < 0) {
		perror("Erreur lecture identifiant commande.");
		adeconnecter=true;
		return;
	}
	nblu=read(fdSocket, &codeCmd_ctos, sizeof(uint8_t));
	if (nblu < 0) {
		perror("Erreur lecture code commande.");
		adeconnecter=true;
		return;
	}
	nblu=read(fdSocket, buffer, tailleTrame-3);
	if (nblu < 0) {
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
	uint16_t tailleTrame=aenvoyer.size()+3;
	if (write(fdSocket, &tailleTrame, sizeof(uint16_t)) == -1) {
		perror("Perror_sendMsg write client");
	}
	if (write(fdSocket, &idCmd, sizeof(uint16_t)) == -1) {
		perror("Perror_sendMsg write idcmd");
	}
	if (write(fdSocket, &coderetour, sizeof(uint8_t)) == -1) {
		perror("Perror_sendMsg write client");
	}
	if (write(fdSocket, aenvoyer.c_str(), aenvoyer.size()) == -1) {
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
int Client::getFdclient() const {
	return fdSocket;
}
void Client::setFdclient(int fdclient) {
	this->fdSocket = fdclient;
}
const string& Client::getPseudo() const {
	return pseudo;
}
void Client::setPseudo(string pseudo) {
	this->pseudo = pseudo;
}

void Client::sendData(string aenvoyer, uint8_t codeCmd_stoc)
{
	uint16_t tailleTrame=aenvoyer.size()+3;
	if (write(fdSocket, &tailleTrame, sizeof(uint16_t)) == -1) {
		perror("Perror_sendMsg write client");
	}
	if (write(fdSocket, &idCmd, sizeof(uint16_t)) == -1) {
		perror("Perror_sendMsg write idcmd");
	}
	if (write(fdSocket, &codeCmd_stoc, sizeof(uint8_t)) == -1) {
		perror("Perror_sendMsg write client");
	}
	if (write(fdSocket, aenvoyer.c_str(), aenvoyer.size()) == -1) {
		perror("Perror_sendMsg write client");
	}
}



/*************************************/

void Client::agir()
{
	Serveur* srv=Serveur::getInstance();
	/// ANALYSE DE CHAINE COMMANDE
	/// Commande join
	switch (codeCmd_ctos) {
		case 1:
			switch (srv->mp(this, argsCmd[0], argsCmd[1])) {
				case success:
					sendRep(success);
					break;
				case eNotExist:
					sendRep(eNotExist, argsCmd[0]+"n'est pas un client du serveur.");
				default:
					sendRep(error, "Erreur inconnue");
					break;
			}
			break;
		case 2:
			switch (srv->msgToChannel(argsCmd[0], argsCmd[1], this)) {
				case success:
					sendRep(success);
					break;
				case eNotExist:
					sendRep(eNotExist, argsCmd[0]+"n'est pas un channel du serveur.");
				default:
					sendRep(error, "Erreur inconnue");
					break;
			}
			break;
		case 3: {
			string reponse;
			switch (srv->who(&reponse, argsCmd[0])) {
				case success:
					sendRep(success, reponse);
					break;
				case eNotExist:
					sendRep(eNotExist, "Aucun client ne correspond a : "+argsCmd[0]);
					break;
				default:
					sendRep(error, "Erreur inconnue");
					break;
			}
			break;
			}
		case 4: {
					string reponse;
					switch (srv->whoChannel(&reponse, argsCmd[0])) {
						case success:
							sendRep(success, reponse);
							break;
						case eNotExist:
							sendRep(eNotExist, "Aucun channel ne correspond a : "+argsCmd[0]);
							break;
						default:
							sendRep(error, "Erreur inconnue");
							break;
					}
					break;
				}
		case 5: {
			string reponse;
			switch (srv->listerChan(&reponse, argsCmd[0])) {
				case success:
					sendRep(success, reponse);
					break;
				case eNotExist:
					sendRep(eNotExist, "Aucun channel ne correspond a : "+argsCmd[0]);
					break;
				default:
					sendRep(error, "Erreur inconnue");
					break;
			}
			break;
			}
		case 7:
			switch (srv->kickFromChan(argsCmd[0], argsCmd[1], this)) {
				case success:
					sendRep(success);
					break;
				case eNotExist:
					sendRep(eNotExist, "Aucun channel avec ce nom ou aucun client de ce nom dans le channel");
					break;
				default:
					sendRep(error, "Erreur inconnue");
					break;
			}
			break;
		case 9:
			switch (srv->op(argsCmd[0], argsCmd[1], this)) {
				case success:
					sendRep(success);
					break;
				case eNotExist:
					sendRep(eNotExist, "Aucun channel avec ce nom ou aucun client de ce nom dans le channel");
					break;
				case eNotAutorized:
					sendRep(eNotAutorized, "Vous n'avez pas les droits pour rendre op quelqu'un dans ce channel");
					break;
				default:
					sendRep(error, "Erreur inconnue");
					break;
			}
			break;
		case 21:
			switch (srv->join(this, argsCmd[0])) {
				case success:
					sendRep(success, "Vous êtes désormais dans le channel");
					break;
				case eTopicUnset:
					sendRep(success, "Un channel a été créé, utilisez la commande topic pour définir le topic");
					break;
				default:
					sendRep(error, "Erreur inconnue");
					break;
			}
			break;
		case 23:
			switch (srv->unjoin(this, argsCmd[0])) {
				case success:
					sendRep(success, "Vous avez quitté le channel"+argsCmd[0]);
					break;
				case eNotExist:
					sendRep(eNotExist, "Le channel "+argsCmd[0]+" n'existe pas");
					break;
				default:
					sendRep(error, "Erreur inconnue");
					break;
			}
			break;
	}
}

/************************************/
