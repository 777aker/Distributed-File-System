// imports yay
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
//#include <fcntl.h>

#define MAXBUF 8192 // max buffer size

struct Servers {

	char *dfs1ip;
	int dfs1port;
	int dfs1sock;

};

struct Servers handleconf(FILE *fp);
void connectservers(struct Servers servers);
void clientlogic(struct Servers servers);

void clientlogic(struct Servers servers) {
	char buf[MAXBUF];
	// do this forever
	while(1) {
		// clear the buff
		bzero(buf, MAXBUF);
		printf("Command: ");
		fgets(buf, MAXBUF, stdin);
		if(strncmp(buf, "exit", 4) == 0) {
			write(servers.dfs1sock, "END CONNECTION\r\n", strlen("END CONNECTION\r\n"));
			break;
		}
		write(servers.dfs1sock, buf, sizeof(buf));
	}
}

void connectservers(struct Servers servers) {

	// connects to dfs1
	int sockfd, connfd;
	struct sockaddr_in serveraddr, cli;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1) {
		printf("socket creation failed\n");
		exit(0);
	} else {
		printf("Socket created\n");
	}
	bzero(&serveraddr, sizeof(serveraddr));

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(servers.dfs1ip);
	serveraddr.sin_port = htons(servers.dfs1port);

	if(connect(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) != 0) {
		printf("Connecting to DFS1 failed\n");
		exit(0);
	} else {
		printf("Connected to DFS1\n");
	}
	servers.dfs1sock = sockfd;

	// connect to other servers here

	// then call this after all connected
	clientlogic(servers);

	// close all the server sockets
	close(sockfd);
}

int main(int argc, char **argv) {
	FILE *fp; // file pointer
	struct Servers servers; // structure that holds servers info

	// did we get correct number of arguments
	if(argc != 2) {
		printf("Wrong number of arguments. Expected ./dfc <.conf file name>");
		exit(0);
	}

	// try to open conf file from arguments
	if((fp = fopen(argv[1], "r")) == NULL) {
		printf("Couldn't open conf file.");
	}
	// function that handles conf data
	servers = handleconf(fp);
	// close the file
	fclose(fp);
	// heheheeee, it works yay
	/*
	printf("Please ip? {%s}\n", servers.dfs1ip);
	printf("Plz port??? {%d}\n", servers.dfs1port);
	*/

	// connect to servers
	connectservers(servers);	
}

struct Servers handleconf(FILE *fp) {
	char newlinedelim[] = "\n"; // for splitting on newline
	char spacedelim[] = " \r\n"; // for splitting on space
	char colondelim[] = " :\r\n"; // for splitting on colon
	char filedata[MAXBUF]; // file data
	char datacopy[MAXBUF]; // strtok destructive so need to copy data
	char *tokked; //  result of strtok
	// struct that will store server data gotten from file
	struct Servers servers;

	// ------------
	// this section correctly gets the first line of the file
	// and the ip and port for the server based on .conf
	// read the conf file
	fgets(filedata, MAXBUF, fp);
	// copy to 
	strcpy(datacopy, filedata);
	// get the first line of the file
	tokked = strtok(datacopy, newlinedelim);
	// parse the first line
	if(strcmp(strtok(tokked, spacedelim), "Server") == 0) {
		tokked = strtok(NULL, spacedelim);
		servers.dfs1ip = strtok(NULL, colondelim);
		servers.dfs1port = atoi(strtok(NULL, colondelim));
		return servers;
	} else {
		printf(".conf not formatted correctly. Please reference .conf provided.\n");
		exit(0);
	}
	// ------------
}