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
//#include <fcntl.h>

#define MAXBUF 8192 // max buffer size

// structure used for passing servers' info between methods
struct Servers {

	char *dfs1ip;
	int dfs1port;
	int dfs1sock;

	char *dfs2ip;
	int dfs2port;
	int dfs2sock;

	char *dfs3ip;
	int dfs3port;
	int dfs3sock;

	char *dfs4ip;
	int dfs4port;
	int dfs4sock;

};

struct Servers handleconf(FILE *fp); // read's the dfc.conf file and parses info
void connectservers(struct Servers servers); // connect to the 4 servers
void clientlogic(struct Servers servers); // where the client's processing and actions happen
long file_size(char *name); // get a file's size
void get(struct Servers servers, char *filename); // handle client doing get command
void put(struct Servers servers, char *filename); // handle client doing put command
void list(struct Servers servers); // handle client doing list command
int connectserver(char *ip, int port); // helper that connects to one server
void exitcmd(struct Servers servers); // handle client doing exit command
void mkdir(struct Servers servers, char *dirname); // handle mkdir command
void login(struct Servers servers); // handles a user logging in
void logout(struct Servers servers); // handles a user logging out

void get(struct Servers servers, char *filename) {
	//printf("%s\n", filename);
}

void put(struct Servers servers, char *filename) {
	//printf("%s\n", filename);
}

void list(struct Servers servers) {

}

void exitcmd(struct Servers servers) {

}

void mkdir(struct Servers servers, char *dirname) {
	//printf("%s\n", dirname);
}

void login(struct Servers servers) {

}

void logout(struct Servers servers) {

}

void clientlogic(struct Servers servers) {
	char buf[MAXBUF];
	char spacedelim[] = " \r\n"; // for splitting on space
	char *tokked; //  result of strtok
	int optval = 1;
	socklen_t optlen = sizeof(optval);
	// do this forever
	while(1) {
		// clear the buff
		bzero(buf, MAXBUF);
		printf("Command: ");
		fgets(buf, MAXBUF, stdin);
		if(strncmp(buf, "exit", 4) == 0) {
			exitcmd(servers);
			break;
		}
		if(strncmp(buf, "get", 3) == 0) {
			tokked = strtok(buf, spacedelim);
			tokked = strtok(NULL, spacedelim);
			get(servers, tokked);
		}
		if(strncmp(buf, "put", 3) == 0) {
			tokked = strtok(buf, spacedelim);
			tokked = strtok(NULL, spacedelim);
			put(servers, tokked);
		}
		if(strncmp(buf, "list", 4) == 0) {
			list(servers);
		}
		if(strncmp(buf, "mkdir", 5) == 0) {
			tokked = strtok(buf, spacedelim);
			tokked = strtok(NULL, spacedelim);
			mkdir(servers, tokked);
		}
		if(strncmp(buf, "login", 5) == 0) {
			login(servers);
		}
		if(strncmp(buf, "logout", 6) == 0) {
			logout(servers);
		}
		if(servers.dfs1sock != -1) {
			write(servers.dfs1sock, buf, sizeof(buf));
		}
		if(servers.dfs2sock != -1) {
			write(servers.dfs2sock, buf, sizeof(buf));
		}
		if(servers.dfs3sock != -1) {
			write(servers.dfs3sock, buf, sizeof(buf));
		}
		if(servers.dfs4sock != -1) {
			write(servers.dfs4sock, buf, sizeof(buf));
		}
		//printf("dfs1sock check: %d\n", servers.dfs1sock);
	}
}

void connectservers(struct Servers servers) {
	
	// connect to all the servers
	servers.dfs1sock = connectserver(servers.dfs1ip, servers.dfs1port);
	servers.dfs2sock = connectserver(servers.dfs2ip, servers.dfs2port);
	servers.dfs3sock = connectserver(servers.dfs3ip, servers.dfs3port);
	servers.dfs4sock = connectserver(servers.dfs4ip, servers.dfs4port);

	// then call this after all connected
	clientlogic(servers);

	// close all the server sockets
	close(servers.dfs1sock);
	close(servers.dfs2sock);
	close(servers.dfs3sock);
	close(servers.dfs4sock);
}

// this connects to servers given ip and port
int connectserver(char *ip, int port) {
	int sockfd, connfd;
	struct sockaddr_in serveraddr;
	int optval = 1;
	socklen_t optlen = sizeof(optval);

	//printf("%s:%d\n", ip, port);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1) {
		printf("Socket creation failed exiting.\n");
		exit(0);
	}
	bzero(&serveraddr, sizeof(serveraddr));

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(ip);
	serveraddr.sin_port = htons(port);

	if(connect(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) != 0) {
		printf("Connecting to %s:%d failed. Continuing\n", ip, port);
		return -1;
	} else {
		printf("Connected to %s:%d\n", ip, port);
	}
	// set to keep alive and the various KEEPALIVE options
	if(setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0) {
		printf("setsockopt error %s:%d\n", ip, port);
		close(sockfd);
		return -1;
	}
	// this didn't work bc these don't exist, oh well
	// probably look into it in office hours
	/*
	optval = 2;
	if(setsockopt(sockfd, SOL_TCP, TCP_KEEPCNT, &optval, optlen) < 0) {
		printf("setsockopt error %s:%d\n", ip, port);
		close(sockfd);
		return -1;
	}
	optval = 15;
	if(setsockopt(sockfd, SOL_TCP, TCP_KEEPCNT, &optval, optlen) < 0) {
		printf("setsockopt error %s:%d\n", ip, port);
		close(sockfd);
		return -1;
	}
	optval = 15;
	if(setsockopt(sockfd, SOL_TCP, TCP_KEEPINTVL, &optval, optlen) < 0) {
		printf("setsockopt error %s:%d\n", ip, port);
		close(sockfd);
		return -1;
	}
	*/

	// return the socket
	return sockfd;
}

// get file's size
long file_size(char *name) {
	FILE *fp = fopen(name, "rb");

	long size = -1;
	if(fp) {
		fseek(fp, 0, SEEK_END);
		size = ftell(fp)+1;
		fclose(fp);
	}
	return size;
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
		exit(0);
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

	// this is a pretty bad way of doing this since I'm doing the exact
	// same thing 4 times for each server but that's the most straightforward
	// wow, I didn't know that fgets stops on \n and now I'm slightly upset
	// this would've been way easier from the start had I know that
	// and wouldn't have had to do this in such a nonsense way but oh well
	// it works so moving on
	// easy way
	// read the conf file
	fgets(filedata, MAXBUF, fp);
	// --------------
	// copy to 
	strcpy(datacopy, filedata);
	// get the first line of the file
	tokked = strtok(datacopy, newlinedelim);
	// parse the first line
	if(strcmp(strtok(tokked, spacedelim), "Server") == 0) {
		tokked = strtok(NULL, spacedelim);
		servers.dfs1ip = strtok(NULL, colondelim);
		servers.dfs1port = atoi(strtok(NULL, colondelim));
	} else {
		printf(".conf not formatted correctly. Please reference .conf provided.\n");
		exit(0);
	}
	printf("read %s:%d\n", servers.dfs1ip, servers.dfs1port);
	// --------------
	// copy to 
	fgets(filedata, MAXBUF, fp);
	bzero(datacopy, MAXBUF);
	strcpy(datacopy, filedata);
	//printf("yo\n");
	// get the second line of the file
	tokked = strtok(datacopy, newlinedelim);
	//printf("yo %s\n", tokked);
	//tokked = strtok(NULL, newlinedelim);
	//printf("yo %s\n", tokked);
	// parse the first line
	if(strcmp(strtok(tokked, spacedelim), "Server") == 0) {
		//printf("yo\n");
		tokked = strtok(NULL, spacedelim);
		servers.dfs2ip = strtok(NULL, colondelim);
		servers.dfs2port = atoi(strtok(NULL, colondelim));
		//printf("yo\n");
	} else {
		printf(".conf not formatted correctly. Please reference .conf provided.\n");
		exit(0);
	}
	printf("read %s:%d\n", servers.dfs2ip, servers.dfs2port);
	// --------------
	// copy to
	fgets(filedata, MAXBUF, fp);
	bzero(datacopy, MAXBUF);
	strcpy(datacopy, filedata);
	// get the first line of the file
	tokked = strtok(datacopy, newlinedelim);
	//tokked = strtok(NULL, newlinedelim);
	//tokked = strtok(NULL, newlinedelim);
	// parse the first line
	if(strcmp(strtok(tokked, spacedelim), "Server") == 0) {
		tokked = strtok(NULL, spacedelim);
		servers.dfs3ip = strtok(NULL, colondelim);
		servers.dfs3port = atoi(strtok(NULL, colondelim));
	} else {
		printf(".conf not formatted correctly. Please reference .conf provided.\n");
		exit(0);
	}
	printf("read %s:%d\n", servers.dfs3ip, servers.dfs3port);
	// --------------
	// copy to
	fgets(filedata, MAXBUF, fp);
	bzero(datacopy, MAXBUF);
	strcpy(datacopy, filedata);
	// get the first line of the file
	tokked = strtok(datacopy, newlinedelim);
	//tokked = strtok(NULL, newlinedelim);
	//tokked = strtok(NULL, newlinedelim);
	//tokked = strtok(NULL, newlinedelim);
	// parse the first line
	if(strcmp(strtok(tokked, spacedelim), "Server") == 0) {
		tokked = strtok(NULL, spacedelim);
		servers.dfs4ip = strtok(NULL, colondelim);
		servers.dfs4port = atoi(strtok(NULL, colondelim));
	} else {
		printf(".conf not formatted correctly. Please reference .conf provided.\n");
		exit(0);
	}
	printf("read %s:%d\n", servers.dfs4ip, servers.dfs4port);
	return servers;
}