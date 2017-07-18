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

void handle_bonj(int sock)
{
	char buffer[BUFLEN], cmd[MAXCMD], parametre[MAXPARAM], *f;
	int k, size, status;

	strcpy(buffer, "bonj\0");
    send(sock, buffer, BUFLEN, 0);
    recv(sock, buffer, BUFLEN, 0);
    printf("%s\n", buffer);
    return;
}

void handle_who(int sock)
{
	char buffer[BUFLEN], cmd[MAXCMD], parametre[MAXPARAM], *f;
	int k, size, status;

	printf("Login: ");
	scanf("%s", parametre);
	strcpy(buffer, "who ");
	strcat(buffer, parametre);
	send(sock, buffer, BUFLEN, 0);
	recv(sock, buffer, BUFLEN, 0);
    printf("%s\n", buffer);
    return;
}

void handle_passwd(int sock)
{
	char buffer[BUFLEN], cmd[MAXCMD], parametre[MAXPARAM], *f;
	int k, size, status;

	printf("Password: ");
	scanf("%s", parametre);
	strcpy(buffer, "passwd ");
	strcat(buffer, parametre);
	send(sock, buffer, BUFLEN, 0);
	recv(sock, buffer, BUFLEN, 0);
	printf("%s\n", buffer);
    return;
}

void handle_get(int sock)
{
	char buffer[BUFLEN], cmd[MAXCMD], parametre[MAXPARAM], *f;
	int k, size, status;
	int descripteur;
	int i;


	printf("Entrer le nom de fichier a obtenir: ");
	scanf("%s", parametre);
	strcpy(buffer, "get ");
	strcat(buffer, parametre);
	send(sock, buffer, BUFLEN, 0);
	recv(sock, &size, sizeof(int), 0);
	if(!size)
	{
		printf("Le fichier n'existe pas\n\n");
		return;
	}
	f = malloc(size);
	recv(sock, f, size, 0);
	while(1)
	{
		descripteur = open(parametre, O_CREAT | O_EXCL | O_WRONLY, 0666);
		if(descripteur == -1)
		{
			sprintf(parametre + strlen(parametre), "%d", i);//needed only if same directory is used for both server and client
		}
			else return;
	}
	write(descripteur, f, size, 0);
	close(descripteur);
	strcpy(buffer, "cat ");
	strcat(buffer, parametre);
	system(buffer);
	return;
}

void handle_put(int sock)
{
	int descripteur;
	struct stat obj;

	char buffer[BUFLEN], cmd[MAXCMD], parametre[MAXPARAM], *f;
	int k, size, status;

	printf("Entrer le nom du fichier a envoyer au server: ");
	scanf("%s", parametre);
	descripteur = open(parametre, O_RDONLY);
	if(descripteur == -1)
	{
		printf("Ce fichier n'existe pas!\n\n");
		return;
	}
	strcpy(buffer, "put ");
	strcat(buffer, parametre);
	send(sock, buffer, BUFLEN, 0);
	stat(parametre, &obj);
	size = obj.st_size;
	send(sock, &size, sizeof(int), 0);
	sendfile(sock, descripteur, NULL, size);
	recv(sock, &status, sizeof(int), 0);
	if(status)
		printf("File envoyer avec succes!\n");
	else
		printf("Erreur lors de l'envoie du fichier\n");
	return;
}

void handle_pwd(int sock)
{
	char buffer[BUFLEN], cmd[MAXCMD], parametre[MAXPARAM], *f;
	int k, size, status;

	strcpy(buffer, "pwd");
	send(sock, buffer, BUFLEN, 0);
	recv(sock, buffer, BUFLEN, 0);
	printf("Le chemin sur le server distant est: %s\n", buffer);
	return;
}

void handle_ls(int sock)
{
	char buffer[BUFLEN], cmd[MAXCMD], parametre[MAXPARAM], *f;
	int descripteur;
	int k, size, status;

	strcpy(buffer, "ls");
	send(sock, buffer, BUFLEN, 0);
	recv(sock, &size, sizeof(int), 0);
	f = malloc(size);
	recv(sock, f, size, 0);
	descripteur = creat("/tmp/client.tmp", O_WRONLY);
	write(descripteur, f, size, 0);
	close(descripteur);
	printf("La liste des fichier sur le server distant:\n");
	system("cat /tmp/client.tmp");
}

void handle_cd(int sock)
{
	char buffer[BUFLEN], cmd[MAXCMD], parametre[MAXPARAM], *f;
	int k, size, status;

	strcpy(buffer, "cd ");
	printf("Nouveau chemin: ");
	scanf("%s", buffer + 3);
	send(sock, buffer, BUFLEN, 0);
	recv(sock, &status, sizeof(int), 0);
	if(status)
		printf("Changement de dossier réussi\n");
	else
		printf("Erreur dans le Changement du chemin\n");
	return;
}

void handle_quit(int sock)
{
	char buffer[BUFLEN], cmd[MAXCMD], parametre[MAXPARAM], *f;
	int k, size, status;

	strcpy(buffer, "quit");
	send(sock, buffer, BUFLEN, 0);
	recv(sock, &status, BUFLEN, 0);
	if(status)
    {
		printf("Le server distant a quitter...\n");
		exit(0);
    }
    printf("Erreur le server n'a pas exit\n");
}

int createSock(char *port)
{
	struct sockaddr_in addr;
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == -1)
    {
    	printf("Erreur socket()");
    	exit(1);
	}
	addr.sin_family = AF_INET;
	addr.sin_port = atoi(port);
	addr.sin_addr.s_addr = 0;
	int sock2 = connect(sock,(struct sockaddr*)&addr, sizeof(addr));
	if(sock2 == -1)
	{
		printf("Erreur connect()");
		exit(1);
	}

	return sock;
}

int main(int argc,char *argv[])
{
	int sock = createSock(argv[1]);
	struct stat obj;
	int choix;
	char buffer[BUFLEN], cmd[MAXCMD], parametre[MAXPARAM], *f;
	int k, size, status;
	int descripteur;
	int i = 1;

	while(1)
    {
		printf("Faites votre choix:\n1-bonj\n2-who\n3-passwd\n4-get\n5-put\n6-pwd\n7-ls\n8-cd\n9-quit\n");
		scanf("%d", &choix);
		switch(choix)
		{
			case 1:
				handle_bonj(sock);
				break;
			case 2:
				handle_who(sock);
				break;
			case 3:
				handle_passwd(sock);
				break;
			case 4:
			  	handle_get(sock);
			  	break;
			case 5:
				handle_put(sock);
				break;
			case 6:
				handle_pwd(sock);
				break;
			case 7:
				handle_ls(sock);
				break;
			case 8:
				handle_cd(sock);
				break;
			case 9:
				handle_quit(sock);
				break;
			default:
				break;
		}
	}
}