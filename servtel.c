#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include<unistd.h> 
#include <signal.h>

#define BUFLEN 1024
#define MAXCMD 255
#define MAXLOGPWD 32

#define CMD    0xFF
#define BONJ   0x41
#define WHO    0x42
#define PASSWD 0x43

void usage(){
  printf("Usage: minitel [OPTIONS] [HOST [PORT]] [INPUT DATA]\n");
}

void handle_signal(int signal);

void handle_signal(int signal) {
	const char *signal_name;
    	sigset_t pending;

    	// Find out which signal we're handling
    	switch (signal) {
        	case SIGINT:
            		printf("Caught SIGINT, exiting now\n");
            		exit(0);
			break;
        	default:
            		fprintf(stderr, "Caught wrong signal: %d\n", signal);
            		return;
			break;
    	}	
}

void sigCases(){

        struct sigaction sa;
        // Setup the sighub handler
        sa.sa_handler = &handle_signal;

        // Restart the system call, if at all possible
        sa.sa_flags = SA_RESTART;

        // Block every signal during the handler
        sigfillset(&sa.sa_mask);
        if (sigaction(SIGINT, &sa, NULL) == -1) {
                perror("Error: cannot handle SIGINT"); // Should not happen
        }
}

int createSock(char* ip_version,char* host,char* port){
	
        // Création de la structure d'adresse serveur
        struct sockaddr_in servaddr;
	// Création des descripteurs de fichiers
        int listenfd;
        
	if(strcmp(ip_version, "ipv6")==0){
		// creation de la socket TCP(SOCK_STREAM) sur IPv4(AF_INET)
        	// recuperation du descripteur de socket listenfd
        	listenfd=socket(AF_INET,SOCK_STREAM,0);

        	// mise a zero de la structure servaddr
        	bzero(&servaddr,sizeof(servaddr));

        	// famille d'adresses : IPv4
        	servaddr.sin_family=AF_INET;
        	// adresse IP d'ecoute du serveur : INADDR_ANY (adresse wildcard)
        	// c'est-a-dire que le serveur ecoutera sur toutes ses interfaces
        	// on la range en ordre réseau avec htonl
        	inet_pton(AF_INET, host, &servaddr.sin_addr);
        	// port d'ecoute du serveur : l'argument n°1
        	// on le convertit en entier avec atoi
        	// puis on le range en ordre reseau avec htons
        	servaddr.sin_port=htons(atoi(port));
	
        	// on lie la structure servaddr a la socket listenfd
        	// pour definir le port et l'adresse d'ecoute du serveur
        	if (bind(listenfd,(struct sockaddr *) &servaddr,sizeof(servaddr))<0)
        	{
                	perror("Erreur de bind");
                	exit(-1);
        	}

        	// on passe la socket en mode passif : elle devient capable d'attendre une connexion
        	listen(listenfd,10);
	}
	if(strcmp(ip_version,"ipv4")==0){
                // creation de la socket TCP(SOCK_STREAM) sur IPv4(AF_INET)
                // recuperation du descripteur de socket listenfd
                listenfd=socket(AF_INET,SOCK_STREAM,0);

                // mise a zero de la structure servaddr
                bzero(&servaddr,sizeof(servaddr));

                // famille d'adresses : IPv4
                servaddr.sin_family=AF_INET;
                // adresse IP d'ecoute du serveur : INADDR_ANY (adresse wildcard)
                // c'est-a-dire que le serveur ecoutera sur toutes ses interfaces
                // on la range en ordre réseau avec htonl
        	inet_pton(AF_INET, host, &servaddr.sin_addr);
                // port d'ecoute du serveur : l'argument n°1
                // on le convertit en entier avec atoi
                // puis on le range en ordre reseau avec htons
                servaddr.sin_port=htons(atoi(port));

                // on lie la structure servaddr a la socket listenfd
                // pour definir le port et l'adresse d'ecoute du serveur
                if (bind(listenfd,(struct sockaddr *)& servaddr,sizeof(servaddr))<0)
                {
                        perror("Erreur de bind");
                        exit(-1);
                }

                // on passe la socket en mode passif : elle devient capable d'attendre une connexion
                listen(listenfd,10);
	}
	return listenfd;

}

int watchFD(int listenfd){
	int connfd,port;
	socklen_t len;
        struct sockaddr_in cliaddr;
	char adresseIP[16];
	//Initialisation des variables
    	unsigned char buf[BUFLEN + 1];
	//Création du set de déscripteurs de fichiers
	fd_set rfds;
	//Création de notre structure pour utiliser time
	struct timeval tv;
	int retval;
  	time_t tempo;
  	//init buffer
  	memset(buf, 0, sizeof(buf));
	sigCases();
	for(;;) 
	{
		len=sizeof(cliaddr);
		// mise en attente de connexion
		// on recuperera dans cliaddr les coordonnees (adresse et port du client)
	    	// accept retourne un descripteur de socket connectée avec lequel on lire et ecrira dans la socket
	    	connfd=accept(listenfd,(struct sockaddr*)&cliaddr,&len);
	    	// On recupere dans port le port de connexion du client
	    	// on le passe en ordre machine avec ntohs
	    	port=ntohs(cliaddr.sin_port);
	    	// on convertit l'adresse IP stockee en numerique dans cliaddr
	    	// en decimale pointee
	    	inet_ntop(AF_INET,&cliaddr.sin_addr,adresseIP,sizeof(adresseIP));
	    	// on affiche les coordonnees du client
	    	printf("connexion de %s, port %d\n",adresseIP,port);
	    	// on met dans tempo le nombre de secondes ecoulees depuis le 01/01/1970
	    	tempo=time(NULL);
	    	// on tranforme tempo en chaine de caracteres buff contenant la date et l'heure
	    	snprintf(buf,sizeof(buf),"%.24s\n",ctime(&tempo));
	    	// on ecrit dans la socket la chaine buff
	    	write(connfd,buf,strlen(buf));
            
            	fd_set rfds;
            	FD_ZERO(&rfds);
            	if (connfd != 0)
              		FD_SET(connfd, &rfds);
           	FD_SET(0, &rfds);
            
            	retval = select(connfd+1, &rfds, (fd_set *) 0, (fd_set *) 0, &tv);
            	if (retval == -1){
              		perror("select()");
            	} 
            	else if (connfd != 0 && FD_ISSET(connfd, &rfds)){
             		int rv;
              		if ((rv = recv(connfd , buf, 1, 0)) < 0)
              		{
                		return 1;
              		}
              		else if (rv == 0)
              		{
                		printf("Connection closed by the remote end\n\r");
                		return 0;
              		}
			switch(buf[0])
              		{
                		case CMD:
                      			//do CMD
                      			break;
                  		case BONJ:
                      			printf("Client sent [BONJ] message\n");
                      			handle_bonj_msg(connfd);
                      			break;
                  		case WHO:
                      			printf("Client sent [WHO] message\n");
                      			handle_who_msg(connfd);
                      			break;
                  		case PASSWD:
                     			printf("Client sent [PASSWD] message\n");
                      			handle_passwd_msg(connfd);
                      			break;
		   
                  		default:
                      			printf("%s\n", buf);
                      			//cleaning buf
                      			memset(buf, 0, sizeof(buf));
              		}	 
		}
		else if (FD_ISSET(0, &rfds))
        	{
            		if(fgets(buf, sizeof(buf), stdin) == NULL)
                		return 1;

            		//write in socket
            		write(connfd, buf, strlen(buf));

            		//show cmd sended
            		printf("> Envoi: %s\n", buf);

            		//flush stdout
            		fflush(stdout);

            		//cleaning buf
            		memset(buf, 0, sizeof(buf));
       		}
        	else {
            		//printf("No data within five seconds.\n");
            		continue;
        	}
    	}
    	exit(EXIT_SUCCESS);
	
 }

void handle_bonj_msg(int sock){
	//debug
	printf("HANDLE BONJ");
	char tmp[MAXLOGPWD];
	tmp[0]=WHO;
	if(send(sock,tmp,sizeof(tmp),0)<=0)
 		exit(1);
}


void handle_who_msg(int sock){
	//debug
	printf("HANDLE WHO");
	char tmp[MAXLOGPWD];
	tmp[0]=PASSWD;
	if(send(sock,tmp,sizeof(tmp),0)<=0)
		exit(1);
}

void handle_passwd_msg(int sock){
	//debug
	printf("HANDLE PASSWD");
	char tmp[MAXLOGPWD];
	tmp[0]=CMD;
	if(send(sock,tmp,sizeof(tmp),0)<=0)
		exit(1);
	
}

int main(int argc, char **argv)
{ 
  	// s'il n'y a pas 2 arguments sur la ligne de commande, on sort
	if (argc < 2 || argc > 3) {
        	usage();
        	return 1;
        }
	int sock = createSock("ipv4",argv[1],argv[2]);
	if(sock == -1)
    	{
        	perror("createSock() Failed!");
        	return 1;
    	}
	
	watchFD(sock);
}


