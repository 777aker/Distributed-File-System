// imports yay
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAXBUF 8192 // max IO buffer between client and server
#define LISTENQ 1024 // second argument to listen()

struct User {
	char *username;
	char *password;
};

int open_listenfd(int port); // handle some connection stuff
void logic(int connfd); // actual logic of the server or at least the start of it
void *thread(void *vargp); // handle the threading stuff
void get(int con, char *buf, struct User user); // handle get command
void put(int con, char *buf, struct User user); // handle put command
void list(int con, struct User user); // handle list command
void mkdir(int con, char *buf, struct User user); // handle mkdir command
struct User login(int con, char *buf); // handle login command

char root[5];

void get(int con, char *buf, struct User user) {

}

void put(int con, char *buf, struct User user) {

}

void list(int con, struct User user) {

}

void mkdir(int con, char *buf, struct User user) {

}

struct User login(int con, char *buf) {

}

void logic(int connfd) {
	size_t n;
	char buf[MAXBUF];
	int optval = 1;
	socklen_t optlen = sizeof(optval);
	struct User user;
	user.username = "";
	user.password = "";

	bzero(buf, MAXBUF);
	n = read(connfd, buf, MAXBUF);
	printf("server received:\n{%s}\n", buf);
	while(strcmp(buf, "END CONNECTION\r\n") != 0) {
		if(strcmp(buf, "") == 0) {
			printf("Recieved empty buf assuming dead client closing connection\n");
			return;
		}
		bzero(buf, MAXBUF);
		n = read(connfd, buf, MAXBUF);
		//printf("server received while:\n{%s}\n", buf);
		if(strncmp(buf, "get", 3) == 0) {
			get(connfd, buf, user);
		} else if(strncmp(buf, "put", 3) == 0) {
			put(connfd, buf, user);
		} else if(strncmp(buf, "list", 4) == 0) {
			list(connfd, user);
		} else if(strncmp(buf, "mkdir", 5) == 0) {
			mkdir(connfd, buf, user);
		} else if(strncmp(buf, "login", 5) == 0) {
			user = login(connfd, buf);
		} else if(strncmp(buf, "logout", 6) == 0) {
			user.username = "";
			user.password = "";
			bzero(buf, MAXBUF);
			strcpy(buf, "1");
			write(connfd, buf, sizeof(buf));
		}
	}
	if(strcmp(buf, "END CONNECTION\r\n") == 0)
		printf("Recieved END CONNECTION closing connection\n");
}

// opening the listenfd
int open_listenfd(int port) {
	int listenfd, optval=1;
	struct sockaddr_in serveraddr;
	socklen_t optlen = sizeof(optval);

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
	// set to keep alive
	if(setsockopt(listenfd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0) {
		printf("setsockopt error");
		close(listenfd);
		return -1;
	}
	// this didn't work bc these don't exist, oh well
	// probably look into it in office hours
	/*
	optval = 2;
	if(setsockopt(listenfd, SOL_TCP, TCP_KEEPCNT, &optval, optlen) < 0) {
		printf("setsockopt error");
		close(listendfd);
		return -1;
	}
	optval = 15;
	if(setsockopt(listenfd, SOL_TCP, TCP_KEEPCNT, &optval, optlen) < 0) {
		printf("setsockopt error");
		close(listendfd);
		return -1;
	}
	optval = 15;
	if(setsockopt(listenfd, SOL_TCP, TCP_KEEPCNT, &optval, optlen) < 0) {
		printf("setsockopt error");
		close(listendfd);
		return -1;
	}
	*/

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
	strcpy(root, argv[1]);
	strcat(root, "/");
	printf("root %s\n", root);

	listenfd = open_listenfd(port);
	if(listenfd == -1) {
		printf("Error creating listenfd\n");
		exit(0);
	}
	while(1) {
		connfdp = malloc(sizeof(int));
		*connfdp = accept(listenfd, (struct sockaddr*)&clientaddr, &clientlen);
		pthread_create(&tid, NULL, thread, connfdp);
	}
}