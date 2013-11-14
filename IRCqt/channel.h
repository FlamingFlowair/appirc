#ifndef CHANNEL_H
#define CHANNEL_H

#include "client.h"

#include <list>
#include <string>
using namespace std;

//class Serveur;

class Channel {
	private:
		/// Nom du channel
		string name;
		/// Topic du channel
		string topic;
		/// Liste des clients dans un channels
		list<Client*> clientsChan;
		/// Liste des operateurs
		list<Client*> opChan;
		///Liste des bannis par pseudo (cf protocole)
		list<string> bannis;
		/// Compteur de personnes dans le channel (si 0 destruction)
		unsigned int compt;
	public:
		/// Constructeur / Destructeur
		Channel(string name, string topic, Client* Createur);
		~Channel();
		/// Getteurs / Setteurs
		unsigned int getCompt() const;
		void setCompt(unsigned int compt);
		const string& getName() const;
		void setName(string name);
		const string& getTopic() const;
		void setTopic(string topic);

		// Ajouter un client à la black list
		void addBan(string pseudo);
		//Enlever un client de la black list
		void removeBan(string pseudo);
		/// Ajouter un client à listecli_chan
		unsigned int addClient(Client* newclient);
		/// Virer un client à la listecli_chan
		unsigned int virerClient(string patternOldClient, Client* kicker);
		unsigned int virerClient(Client* oldclient);
		/// Ajouter un client à listeop
		unsigned int addop(Client* oldop, string newop);
		/// Virer un client à la listeop
		unsigned int virerop(Client* oldop, Client* kicker=NULL);
		unsigned int virerop(string pseudo, Client* kicker=NULL);
		/// Vérifie si un client est utilisateur
		bool isop(Client* clitotest);
		/// Vérifie si un utilisateur est un client du channel
		bool isclient(Client* clitotest);
		Client * isclient(string pseudo);
		/// appelée pour commande who sur channel
		unsigned int who(string* msgtosend, string pattern)const;
		///appelée pour commande ban
		unsigned int ban(string *reponse, string pattern, Client * envoyeur, int * nbBannis);
		///appelée pour commande unban
		unsigned int unban(string *reponse, string pattern, Client * envoyeur, int * nbDebannis);
		///appelée pour commande listBan
		void listBan(string *reponse);
		/// Enverras le message à tous les utilisateurs du channel
		void send(string message, unsigned int coderet);

};

#endif // CHANNEL_H
