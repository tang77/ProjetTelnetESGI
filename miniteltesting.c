#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<sys/types.h>
#include	<sys/wait.h>
#include	<signal.h>
#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<errno.h>
#include 	<string.h>
#include	<unistd.h>
#include 	<sys/time.h>
//Fonction pouvant déterminer le maximum entre 2 entier
#define	max(a,b)	((a) > (b) ? (a) : (b))

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

int createSock(ip_version,host,port){
  int sockfd;
  struct sockaddr_in servaddr;
  if(ip_version="ipv6"){
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(port));
    inet_pton(AF_INET, host, &servaddr.sin_addr);
    connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
  }
  if(ip_version="ipv4"){
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
   //Création du set de déscripteurs de fichiers
   fd_set rfds;
   //Création de notre structure pour utiliser time
   struct timeval tv;
   int retval;
   int maxfdp1;
   char sendline[150];
   int n;
 
   /* Regarde dans l'entrée standard si il y a des données entrées */
   for( ; ; ){
     FD_ZERO(&rfds);
     FD_SET(socketfd, &rfds);
     maxfdp1 = max(fileno(stdin),socketfd) + 1;
 
     /* Attend jusqu'à 30 secondes */
     tv.tv_sec = 30;
     tv.tv_usec = 0;
 
     retval = select(maxfdp1, &rfds, NULL, NULL, &tv);
     /* Don't rely on the value of tv now! */
     if (retval == -1){
       perror("select()");
     }
     else if (retval){
       printf("Data is available now.\n");
       /* FD_ISSET(0, &rfds) will be true. */
       shutdown(sockfd,SHUT_WR);
       write(sockfd, sendline,n);
     }
     else {
       printf("No data within five seconds.\n");
     }
   }exit(EXIT_SUCCESS);
}



int main(int argc, char **argv){
  int i;
  char* argzz[argc]; 
    for (i=0; i < argc; i++)
    {
        printf("Argument %ld : %s \n", i+1, argv[i]);
        argzz[i]==argv[i];
        printf("%s",argzz[i]);
    }


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
  transferData(createSock(argIp,argHost,argPort));

  exit(0);
}
