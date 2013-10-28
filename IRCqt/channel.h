#ifndef CHANNEL_H
#define CHANNEL_H

#include "client.h"

#include <list>
#include <string>
using namespace std;

class Serveur;

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
		/// Compteur de personnes dans le channel (si 0 destruction)
		unsigned int compt;
	public:
		/// Constructeur / Destructeur
		Channel(string name, string topic, Client* Createur);

		/// Getteurs / Setteurs
		unsigned int getCompt() const;
		void setCompt(unsigned int compt);
		const string& getName() const;
		void setName(string name);
		const string& getTopic() const;
		void setTopic(string topic);


		/// Ajouter un client à listecli_chan
		unsigned int addClient(Client* newclient);
		/// Virer un client à la listecli_chan
		unsigned int virerClient(string patternOldClient, Client* kicker=NULL);
		/// Ajouter un client à listeop
		unsigned int addop(Client* oldop, Client* newop);
		/// Virer un client à la listeop
		unsigned int virerop(Client* oldop, Client* kicker=NULL);
		/// Vérifie si un client est utilisateur
		bool isop(Client* clitotest);
		/// Vérifie si un utilisateur est un client du channel
		bool isclient(Client* clitotest);

		/// Methodes
		/// Enverras le message à tous les utilisateurs du channel
		void send(Client* envoyeur, string message);

};

#endif // CHANNEL_H
