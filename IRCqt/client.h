/*						 _________________
 *				\|/		/				  \
 *			 /'''''''\ /	Uhhhhhhhh !!  |
 *			 | °  °	 | ''\________________/
 *			 |		 |
 *		nnn	 | `---	 |  nnn
 * ''''''''''''''''''''''''''''
*/

#ifndef CLIENT_H
#define CLIENT_H

#include <stdint.h>
#include <string>
#include <vector>
using namespace std;

class Client {
	private:
		/// File descriptor du socket en entrée
		int fdClient;
		/// String pseudo, auto attribué par le client
		string pseudo;
		/// si ce flag est à  true, il faut virer le client
		bool adeconnecter;
		/// Vector which contains all the arguments of the last read command
		vector<string> argsCmd;
		/// Id of the command, inc on each command by the prgmclient
		uint16_t idCmd;
		/// Code command from client to server correspond to the
		uint8_t codeCmd_ctos;
	public:
		/// Constructeur / Destructeur
		Client(int socket, string pseudo);
		~Client();

		/// Getteurs et setteurs
		int getFdClient() const;
		void setFdclient(int fdClient);
		const string& getPseudo() const;
		void setPseudo(string pseudo);
		bool isAdeconnecter() const;
		void setAdeconnecter(bool adeconnecter);
		const vector<string>& getArgsCmd() const;
		uint16_t getIdcmd() const;
		void setIdcmd(uint16_t idCmd);
		uint8_t getCodecmd() const;
		void setCodecmd(uint8_t codeCmd_ctos);

		/// Methodes
		/// Lit la trame envoyée par le client
		void readCommande();
		/// Agis en fonction des données envoyées par le client
		void agir();
		/// Effectue les envois vers le programme client
		void sendRep(uint8_t coderetour, string aenvoyer="");
};

#endif // CLIENT_H
