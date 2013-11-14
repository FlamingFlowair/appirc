/*						 _________________
 *				\|/		/				  \
 *			 /'''''''\ /	Uhhhhhhhh !!  |
 *			 | °  °	 | ''\________________/
 *			 |		 |
 *		nnn	 | `---	 |  nnn
 * ''''''''''''''''''''''''''''
*/

#ifndef SERVEUR_H
#define SERVEUR_H

/// Inclusion des uint32_t, uint16_t ...
#include <stdint.h>

/// Inclusion des éléments de la stl
#include <list>
#include <map>
#include <string>
using namespace std;

/// Inclusion des autre classes de l'IRC
#include "channel.h"
#include "client.h"

class Serveur {
	private:
		/// Nom (resolved by gethostbyname to get the IP adress)
		string hostname;
		/// Port
		unsigned int port;
		/// Name of the serveur
		string nom;
		/// Greeting message
		string messAcc;
		/// File descriptor of the listen socket
		int fdSocket;
		/// Map of channels via ptr
		map<string, Channel*> nomToChannel;
		/// List of client connected to the server via ptr
		list<Client*> clientsServ;

		/// Private constructor (designed pattern singleton)
		Serveur(string hostname="localhost", unsigned int port=42007,  string nom="Anon", string messAcc="Salut bande de pouilleux");
		/// Init sockaddr_in which is ready to bind
		void init_sockaddrin(struct sockaddr_in *name, string hostname, uint16_t port);
		/// Design pattern singleton : adress of the only Serveur instance
		static Serveur* _instance ;
		/// Private method to get the biggest file descriptor opened
		unsigned int getmaxfd();
	public:
		/// Design pattern singleton : return the instance and create it if necessary
		static Serveur* getInstance();
		/// Destructor
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

		/// Methods which correspond to a command
		/// Send a private message between two clients
		unsigned int mp(Client* envoyeur, string pseudo, string message);
		/// Send a mesage to a channel
		unsigned int msgToChannel(string channelName, string msg, Client* envoyeur);
		/// List clients connected to the server, pattern allowed (* will be transformed to .*)
		unsigned int who(string* msgtosend, string pattern="*") const;
		/// List users corresponding to the pattern and connected to a channel corresponding to the pattern
		unsigned int whoChannel(string* msgtosend, string patternChan="*", string patternClient="*") const;
		/// List channels on the server, pattern allowed (* will be transformed to .*)
		unsigned int listerChan(string *msgtosend, string patternChannelName="*") const;
		/// Change the topic of a channel
		unsigned int changerTopic(string channelName, string newTopic, Client *envoyeur, string *reponse);
		/// Kick someone from a channel
		unsigned int kickFromChan(string channelName, string patternPseudo, Client* kicker) ;
		/// Ban the pseudo of somebody from a channel
		unsigned int ban(string *reponse, string patternChan, string patternPseudo, Client * envoyeur);
		/// Give the operator rights to someone.
		unsigned int op(string channelName, string Pseudo, Client* opper) ;
		/// Remove the operator rights to a client
		unsigned int deop(string channelName, string pseudo, Client * deopper);
		/// Join a Client to a Channel
		unsigned int join (Client *cli, string channelname);
		/// Change the nickname of a client
		unsigned int nick(string newpseudo, Client *envoyeur);
		/// Retire a client from a channel (leave)
		unsigned int unjoin (Client *cli, string channelName);
		/// Unban a pseudo from a channel, pattern allowed
		unsigned int unban(string *reponse, string patternChan, string patternPseudo, Client * envoyeur);
		/// List ban peoples from a channel
		unsigned int listerBan(string channelName, string * reponse);

		/// Methods which does not correspond to a command but are used in one of theses.
		/// Add a channel to the serveur
		unsigned int addchannel(Client* createur, string channelname, string topic);
		/// Send a message to every clients on the server
		unsigned int broadcastmsg(string message) const;

		/// Launch the serveur (may contain a forever() ;) )
		int run();
};

#endif // SERVEUR_H
