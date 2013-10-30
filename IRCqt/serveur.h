#ifndef SERVEUR_H
#define SERVEUR_H

/// déclaration des uint32_t, uint16_t ...
#include <stdint.h>

#include <list>
#include <map>
#include <string>
using namespace std;

#include "channel.h"
#include "client.h"

class Serveur {
	private:
		/// Hostname
		string hostname;
		/// Port
		unsigned int port;
		/// Nom du serveur
		string nom;
		/// Message d'acceuil du serveur
		string messAcc;
		/// File descriptor du socket d'écoute
		int fdSocket;
		/// Map des channels
		map<string, Channel*> nomToChannel;
		/// Liste de client sur le serveur
		list<Client*> clientsServ;

		/// Constructeur privé : designe pattern singleton
		Serveur(string hostname="localhost", unsigned int port=42007,  string nom="Anon", string messAcc="Salut bande de pouilleux");
		/// init le sockaddr_in pour init le serveur
		void init_sockaddrin(struct sockaddr_in *name, string hostname, uint16_t port);

	public:
		/// Adresse du serveur, instanciation et destruction
		static Serveur* _instance ;
		static Serveur* getInstance();
		~Serveur();

		/// Getteurs et Setteurs
		const string& getHostname() const;
		void setHostname(string hostname);
		const string& getMessageacc() const;
		void setMessageacc(string messAcc);
		const string & getNom() const;
		void setNom(string nom);
		unsigned int getPort() const;
		void setPort(unsigned int port);
		int getSocketecoute() const;
		void setSocketecoute(int fdSocket);

		/// Methodes
		/// Ajoute un channel
		unsigned int addchannel(Client* createur, string channelname, string topic);
		/// Ajoute une personne à la liste des clients d'un channel
		unsigned int join (Client *cli, string channelname);
		/// Enleve une personne de la liste des clients d'un channel
		unsigned int unjoin (Client *cli, string channelName);

		/// Envoi un message à un client ou a un channel
		unsigned int mp(Client* envoyeur, string pseudo, string message);
		/// who ?
		unsigned int who(string* msgtosend, string pattern="*") const;
		/// list
		unsigned int listerChan(string *msgtosend, string patternChannelName="*") const;
		///kick
		unsigned int kickFromChan(string channelName, string patternPseudo, Client* kicker) ;
		/// Rend operateur un client du channel
		unsigned int op(string channelName, string Pseudo, Client* opper) ;

		//msg to channel
		unsigned int msgToChannel(string channelName, string msg, Client* envoyeur);

		/// Gere le serveur
		int run();
};

#endif // SERVEUR_H
