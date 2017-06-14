#include    <sys/socket.h>
#include    <netinet/in.h>
#include    <arpa/inet.h>
#include    <sys/types.h>
#include    <sys/wait.h>
#include    <signal.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <errno.h>
#include    <string.h>
#include    <unistd.h>
#include    <sys/time.h>
//Fonction pouvant déterminer le maximum entre 2 entier
#define max(a,b)    ((a) > (b) ? (a) : (b))
#define BUFLEN 1024

void str_cli(FILE *, int);

void usage(){
  printf("Usage: minitel [OPTIONS] [HOST [PORT]] [INPUT DATA]\n");
}
void help(){
  printf("Usage: minitel [OPTIONS] [HOST [PORT]] [INPUT DATA]\n");
  printf("Login to remote system HOST (optionally, on service port PORT)\n\n");
  printf("General options :\n\n");
  printf("-4, --ipv4                 use only IPv4\n");
  printf("-6, --ipv6                 use only IPv6\n");
  printf("-d, --debug                turn on debugging\n");
  printf("-?, --help                 give this help list\n");
  printf("    --usage                give a short usage message\n");
  printf("-V, --version              print program version\n");
}

int createSock(char* ip_version,char* host,char* port){
  int sockfd;
  struct sockaddr_in servaddr;
  if(strcmp(ip_version, "ipv6")==0){
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(port));
    inet_pton(AF_INET, host, &servaddr.sin_addr);
    connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
  }
  if(strcmp(ip_version,"ipv4")==0){
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(port));
    inet_pton(AF_INET, host, &servaddr.sin_addr);
    connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)); 
  }
  return sockfd;
}

void transferData(int socketfd){
    //Initialisationd des variables
    unsigned char buf[BUFLEN + 1];
    //Création du set de déscripteurs de fichiers
    fd_set rfds;
    //Création de notre structure pour utiliser time
    struct timeval tv;
    int retval;
    int maxfdp1;
    char sendline[150];
    int n;

    //init buffer
    memset(buf, 0, sizeof(buf));

    /* Regarde dans l'entrée standard si il y a des données entrées */
    for( ; ; ){
        fd_set rfds;
        FD_ZERO(&rfds);
        if (socketfd != 0)
            FD_SET(socketfd, &rfds);
        FD_SET(0, &rfds);

        //maxfdp1 = max(fileno(stdin),socketfd) + 1;

        /* Attend jusqu'à 30 secondes */
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        retval = select(socketfd+1, &rfds, (fd_set *) 0, (fd_set *) 0, &tv);
        /* Don't rely on the value of tv now! */
        if (retval == -1){
            perror("select()");
        }
        else if (socketfd != 0 && FD_ISSET(socketfd, &rfds)) {
            int rv;
            if ((rv = recv(socketfd , buf , 1 , 0)) < 0)
                return 1;
            else if (rv == 0) {
                printf("Connection closed by the remote end\n\r");
                return 0;
            }
            else
            {
                printf("%s", buf);
                //cleaning buf
                memset(buf, 0, sizeof(buf));
            }
        }
        else if (FD_ISSET(0, &rfds))
        {
            if(fgets(buf, sizeof(buf), stdin) == NULL)
                return 1;

            //write in socket
            write(socketfd, buf, strlen(buf));
            
            //show cmd sended
            printf("> Envoi: %s\n", buf);
            
            //flush stdout
            fflush(stdout);

            //cleaning buf
            memset(buf, 0, sizeof(buf));
        }
        else {
            printf("No data within five seconds.\n");
        }
    }
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv){
    
    if (argc < 2 || argc > 3) {
        printf("Usage: %s IP PORT\n", argv[0]);
        return 1;
    }
    
    int sock = createSock("ipv4", argv[1], argv[2]);
    if(sock == -1)
    {
        perror("createSock() Failed!");
        return 1;
    }
    transferData(sock);

    /*if(argv[1]=="--help\0"||argv[1]=="-h"){
      help();
      exit(-1);
    }
    else if(argv[1]=="--version\0"||argv[1]=="-v"){
      printf("Minitel Version 0.0.1\n");
      printf("Authors : SUEUR Tanguy / VERNHET Anthony");
    }
    else if{
      usage();
      exit(-1);
    }
    else{

    }
  
  if (strcmp(argv[argc],"stdin")!=0){
    fp=fopen(argv[argc],"r");
  }
  else{
    fp=stdin;
  }*/

  exit(0);
}
