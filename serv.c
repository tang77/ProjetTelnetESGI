#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/sendfile.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BUFLEN 100
#define MAXCMD 6
#define MAXPARAM 20

int logged = 0;

void handle_bonj(int sock2)
{
	char buffer[BUFLEN], cmd[MAXCMD], parametre[MAXPARAM];
	strcpy(buffer, "Logger vous avec la commande who !");
	send(sock2, buffer, BUFLEN, 0);
}

void handle_who(int sock2)
{
	char buffer[BUFLEN], cmd[MAXCMD], parametre[MAXPARAM];
	sscanf(buffer, "%s%s", parametre, parametre);
	parametre[MAXPARAM-1] = '\0';

	printf("debug: %s\n", parametre);

	if (!strcmp(parametre, "admin"))
		strcpy(buffer, "Bon login, maintenant utiliser la commande passwd");
	else
		strcpy(buffer, "Mauvais login, utiliser la commande who encore");

	send(sock2, buffer, BUFLEN, 0);
}

void handle_passwd(int sock2)
{
	char buffer[BUFLEN], cmd[MAXCMD], parametre[MAXPARAM];
	sscanf(buffer, "%s%s", parametre, parametre);
	  parametre[19] = '\0';

	  if (!strcmp(parametre, "123456")) {
	    logged = 0;
	    strcpy(buffer, "Bon password, maintenant vous pouvez utiliser les autres commandes");
	  } else
	    strcpy(buffer, "Mauvais password, reloggez vous s'il vous plait!");

	send(sock2, buffer, BUFLEN, 0);
}

void handle_ls(int sock2)
{
  int k, i, taille, len, c;
  struct stat statfichier;
  int descripteur;
	char buffer[BUFLEN], cmd[MAXCMD], parametre[MAXPARAM];

	system("ls > /tmp/serv.tmp");
	i = 0;
	stat("/tmp/serv.tmp", & statfichier);
	taille = statfichier.st_size;
	send(sock2, & taille, sizeof(int), 0);
	descripteur = open("/tmp/serv.tmp", O_RDONLY);
	sendfile(sock2, descripteur, NULL, taille);
}

void handle_get(int sock2)
{
	char buffer[BUFLEN], cmd[MAXCMD], parametre[MAXPARAM];
  int descripteur;
  int k, i, taille, len, c;
  struct stat statfichier;

	sscanf(buffer, "%s%s", parametre, parametre);
	  stat(parametre, & statfichier);
	  descripteur = open(parametre, O_RDONLY);
	  taille = statfichier.st_size;
	  if (descripteur == -1)
	    taille = 0;
	  send(sock2, & taille, sizeof(int), 0);
	  if (taille)
	    sendfile(sock2, descripteur, NULL, taille);
}

void handle_put(int sock2)
{
  int descripteur;
  int k, i, taille, len, c = 0;
	char buffer[BUFLEN], cmd[MAXCMD], parametre[MAXPARAM];

	char * f;
	sscanf(buffer + strlen(cmd), "%s", parametre);
	recv(sock2, & taille, sizeof(int), 0);
	i = 1;
	while (1) {
	descripteur = open(parametre, O_EXCL | O_CREAT | O_WRONLY, 0666);
	if (descripteur == -1) {
	  sprintf(parametre + strlen(parametre), "%d", i);
	} else
	  break;
	}
	f = malloc(taille);
	recv(sock2, f, taille, 0);
	c = write(descripteur, f, taille);
	close(descripteur);
	send(sock2, & c, sizeof(int), 0);
}

void handle_pwd(int sock2)
{
  int k, i, taille, len, c;
	char buffer[BUFLEN], cmd[MAXCMD], parametre[MAXPARAM];

	system("pwd>/tmp/serv.tmp");
	i = 0;
	FILE * f = fopen("/tmp/serv.tmp", "r");
	while (!feof(f))
	buffer[i++] = fgetc(f);
	buffer[i - 1] = '\0';
	fclose(f);
	send(sock2, buffer, BUFLEN, 0);
}

void handle_cd(int sock2, char* buffer)
{
  int k, i, taille, len, c;
	if (chdir(buffer + 3) == 0)
		c = 1;
	else
		c = 0;
	send(sock2, & c, sizeof(int), 0);
}

void handle_quit(int sock2)
{
  int k, i, taille, len, c;

	printf("fin du server..\n");
	i = 1;
	send(sock2, & i, sizeof(int), 0);
	exit(0);
}

int main(int argc, char * argv[]) {
  struct sockaddr_in server, client;
  char buffer[BUFLEN], cmd[MAXCMD], parametre[MAXPARAM];
  struct stat statfichier;
  int sock1, sock2;
  int k, i, taille, len, c;
  int descripteur;
  sock1 = socket(AF_INET, SOCK_STREAM, 0);
  if (sock1 == -1) {
    printf("Erreur dans socket");
    exit(1);
  }
  server.sin_family = AF_INET;
  server.sin_port = atoi(argv[1]);
  server.sin_addr.s_addr = 0;
  k = bind(sock1, (struct sockaddr * ) & server, sizeof(server));
  if (k == -1) {
    printf("Erreur dans bind");
    exit(1);
  }
  k = listen(sock1, 1);
  if (k == -1) {
    printf("Erreur dans listen");
    exit(1);
  }
  len = sizeof(client);
  sock2 = accept(sock1, (struct sockaddr * ) & client, & len);
  i = 1;
  while (1) {
    recv(sock2, buffer, BUFLEN, 0);
    sscanf(buffer, "%s", cmd);

    if (!strcmp(cmd, "bonj")) {
      handle_bonj(sock2);
    } else if (!strcmp(cmd, "who")) {
      handle_who(sock2);
    } else if (!strcmp(cmd, "passwd")) {
      handle_passwd(sock2);
    } else if (!strcmp(cmd, "ls")) {
      handle_ls(sock2);
    } else if (!strcmp(cmd, "get")) {
      handle_get(sock2);
    } else if (!strcmp(cmd, "put")) {
      handle_put(sock2);
    } else if (!strcmp(cmd, "pwd")) {
      handle_pwd(sock2);
    } else if (!strcmp(cmd, "cd")) {
      handle_cd(sock2, buffer);
    } else if (!strcmp(cmd, "bye") || !strcmp(cmd, "quit")) {
      handle_quit(sock2);
    }
  }
  return 0;
}