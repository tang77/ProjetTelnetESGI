#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h> 
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

void handle_signal(int);
void handle_bonj_msg(int);
void handle_who_msg(int);
void handle_passwd_msg(int);

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

int
read_from_client (int filedes)
{
  char buf[BUFLEN];
  int nbytes;

  nbytes = read (filedes, buf, 1);
  if (nbytes < 0)
    {
      perror ("read");
      exit (EXIT_FAILURE);
    }
  else if (nbytes == 0)
    return -1;
  else
    {
      switch(buf[0])
        {
            /*
            case CMD:
                //do CMD
                break;
            */
            case BONJ:
                printf("Client sent [BONJ] message\n");
                handle_bonj_msg(filedes);
                break;
            case WHO:
                printf("Client sent [WHO] message\n");
                handle_who_msg(filedes);
                break;
            case PASSWD:
                printf("Client sent [PASSWD] message\n");
                handle_passwd_msg(filedes);
                break;
            default:
                break;
        }
    }
}

int watchFD(int listenfd){
  fd_set active_fd_set, read_fd_set;
  int i;
  struct sockaddr_in clientname;
  size_t size;
  //Création de notre structure pour utiliser time
    struct timeval tv;

  FD_ZERO (&active_fd_set);
  FD_SET (listenfd, &active_fd_set);

    /* Attend jusqu'à 30 secondes */
    tv.tv_sec = 5;
    tv.tv_usec = 0;

  while (1)
    {
      /* Block until input arrives on one or more active sockets. */
      read_fd_set = active_fd_set;
      if (select (FD_SETSIZE, &read_fd_set, (fd_set *) 0, (fd_set *) 0, &tv) < 0)
        {
          perror ("select");
          exit (EXIT_FAILURE);
        }

      /* Service all the sockets with input pending. */
      for (i = 0; i < FD_SETSIZE; ++i)
        if (FD_ISSET (i, &read_fd_set))
          {
            if (i == listenfd)
              {
                /* Connection request on original socket. */
                int new;
                size = sizeof (clientname);
                new = accept (listenfd,
                              (struct sockaddr *) &clientname,
                              &size);
                if (new < 0)
                  {
                    perror ("accept");
                    exit (EXIT_FAILURE);
                  }
                fprintf (stderr,
                         "Server: connect from host %s, port %hd.\n",
                         inet_ntoa (clientname.sin_addr),
                         ntohs (clientname.sin_port));
                FD_SET (new, &active_fd_set);
              }
            else
              {
                /* Data arriving on an already-connected socket. */
                if (read_from_client (i) < 0)
                  {
                    close (i);
                    FD_CLR (i, &active_fd_set);
                  }
              }
          }
    }

}

void handle_bonj_msg(int sock){
    char tmp[MAXCMD];
    tmp[0]=WHO;
    if(send(sock,tmp,1,0)<0)
        exit(1);
}


void handle_who_msg(int sock){
    char buf[MAXLOGPWD];
    int rv = read(sock, buf, sizeof(buf));
    if(rv<=0)
        exit(1);

    if(strlen(buf)<=0)
        return;

    printf("[WHO] Client sent login: %s\n", buf);

    if(strcmp(buf, "admin") == 0)
    {
        char tmp[MAXCMD];
        tmp[0]=PASSWD;

        if(send(sock,tmp,sizeof(tmp),0)<=0)
            exit(1);    
    }
    else
    {
        handle_bonj_msg(sock);
    }
}

void handle_passwd_msg(int sock){
//debug
    printf("HANDLE PASSWD");
    char tmp[MAXCMD];
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