#ifndef CLIENT_H
#define CLIENT_H

#include <stdint.h>
#include <string>
#include <vector>
using namespace std;

class Client {
	private:
		/// File descriptor du socket en entrée
		int fdSocket;
		/// String pseudo, auto attribué par le client
		string pseudo;
		/// si ce flag est à  true, il faut virer le client
		bool adeconnecter;
		/// String chainedecommande, récupérée par le read du client
		/// Utilisé par agir() qui l'analyse et agit en conséquence
		vector<string> argsCmd; // Tous les messages recus sont des commandes : la normalisation est faite par le prgmclient
		uint16_t idCmd;
		uint8_t codeCmd_ctos;
	public:
		/// Constructeur / Destructeur
		Client(int socket, string pseudo);
		~Client();

		/// Getteurs et setteurs
		int getNbArg()const;
		bool isAdeconnecter() const;
		void setAdeconnecter(bool adeconnecter);
		const string& getChainecommande() const;
		void setChainecommande(string chainecommande);
		int getFdclient() const;
		void setFdclient(int fdSocket);
		const string& getPseudo() const;
		void setPseudo(string pseudo);
		uint8_t getCodecmd() const;
		void setCodecmd(uint8_t codeCmd_ctos);
		uint16_t getIdcmd() const;
		void setIdcmd(uint16_t idCmd);

		/// Methodes
		/// Methode appelée par le serveur à la sortie du select
		void readCommande();
		/// Analyse chainecommande et agit en conséquence
		void agir();
		/// Envoi une chaine de caractère à un prgmclient
		void sendRep (uint8_t coderetour, string aenvoyer="");
		const vector<string> & getArgsCmd () const;
};

#endif // CLIENT_H
