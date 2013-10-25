#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

#define PORT 42007
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
    
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

void init_sockaddrin(struct sockaddr_in* name, char* hostname, uint16_t port) {
	struct hostent* hostinfo;
	name->sin_family=AF_INET;
	name->sin_port=htons(port);
	if ((hostinfo=gethostbyname(hostname)) == NULL) {
		fprintf(stderr, "Host inconnu : *%s*\n", hostname);
		perror("GethostbynamE : ");
		exit(1);
	}
	name->sin_addr=*(struct in_addr*)hostinfo->h_addr;
}

int client(void)
{
	int i=0;//no de client, désactivé pour l'instant.
	int j;
	int idcmd=0, codecmd=0;
	int erreur = 0;
    SOCKET sock;
    SOCKADDR_IN sin;
    if(!erreur)
    {
        /* Création de la socket */
        sock = socket(AF_INET, SOCK_STREAM, 0);
        /* Configuration de la connexion */
		init_sockaddrin(&sin, "localhost", PORT);
        /* Si le client arrive à se connecter */
        if(connect(sock, (SOCKADDR*)&sin, sizeof(sin)) != SOCKET_ERROR){
            printf("Connexion du client %d à %s sur le port %d\n", i, inet_ntoa(sin.sin_addr), htons(sin.sin_port)); 
            char bufferE[4096], bufferS[4096], buffer8[4096];
            uint16_t nblu;
            //while(strcmp(bufferS, "/quit") != 0)
                fgets(bufferE, sizeof(bufferE), stdin);
                nblu=strlen(bufferE);
                if(nblu>0) {
					nblu+=3;
                //else if (nblu==0){
                //    printf("Client, saisissez une commande (/quit pour quitter l'appli) : \n");
                //        scanf("%s", bufferS);
					//sprintf(bufferS, "%.2d%.2d%d%s", nblu, 0, 0, bufferE);
					write(sock, &nblu, sizeof(uint16_t));
					write(sock, &idcmd, sizeof(uint16_t));
					write(sock, &codecmd, sizeof(uint8_t));
					write(sock, bufferE, strlen(bufferE));
					//fgets(bufferE, sizeof(bufferE), stdin);
					for(j=0; j<4096; ++j) {
						buffer8[j]='\0';
					}
					while((read(sock, buffer8, sizeof(buffer8)))>0) {
						printf("Sortie du serveur : %s\n", buffer8);
						for(j=0; j<4096; ++j) {
							buffer8[j]='\0';
						}
					}
					close(sock);
				}
                else perror("SOCKET_ERROR : read client");
        }
        else
            printf("Client %d : Impossible de se connecter\n", i);        
        /* On ferme la socket précédemment ouverte */
        close(sock);
        //printf("Pressez une touche pour continuer...\n");
        //getchar();
    }
  
    return 0;
}

int main(void){
	/*int i;
	for(i=0;i<10;i++)
		client(i);*/
	client();
	return 0;
}
