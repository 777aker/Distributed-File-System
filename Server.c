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

#define MAXBUF 8192 // max IO buffer between client and server
#define LISTENQ 1024 // second argument to listen()

struct user {
	char username[32];
	char password[32];
};

int open_listenfd(int port); // handle some connection stuff
void logic(int connfd); // actual logic of the server or at least the start of it
void *thread(void *vargp); // handle the threading stuff

void logic(int connfd) {
	size_t n;
	char buf[MAXBUF];

	bzero(buf, MAXBUF);
	n = read(connfd, buf, MAXBUF);
	printf("server received:\n{%s}\n", buf);
	while(strcmp(buf, "END CONNECTION\r\n") != 0) {
		bzero(buf, MAXBUF);
		n = read(connfd, buf, MAXBUF);
		printf("server received while:\n{%s}\n", buf);
	}
	printf("Quitting\n");
}

// opening the listenfd
int open_listenfd(int port) {
	int listenfd, optval=1;
	struct sockaddr_in serveraddr;

	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -1;

	if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
			(const void *)&optval, sizeof(int)) < 0)
		return -1;

	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons((unsigned short) port);
	if(bind(listenfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0)
		return -1;

	if(listen(listenfd, LISTENQ) < 0)
		return -1;
	return listenfd;
}

// thread logic/multiple connection handling
void *thread(void *vargp) {
	int connfd = *((int *)vargp);
	pthread_detach(pthread_self());
	free(vargp);
	logic(connfd);
	close(connfd);
	return NULL;
}

int main(int argc, char **argv) {
	
	// creating the threads and accepting connections
	int listenfd, *connfdp, port, clientlen=sizeof(struct sockaddr_in);
	struct sockaddr_in clientaddr;
	pthread_t tid;

	if(argc < 3) {
		printf("Wrong number of arguments expected at least dfs DFS# port#");
		exit(0);
	}

	port = atoi(argv[2]);

	listenfd = open_listenfd(port);
	while(1) {
		connfdp = malloc(sizeof(int));
		*connfdp = accept(listenfd, (struct sockaddr*)&clientaddr, &clientlen);
		pthread_create(&tid, NULL, thread, connfdp);
	}
}