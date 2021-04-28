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
#include <openssl/md5.h>
//#include <fcntl.h>

#define MAXBUF 8192 // max buffer size

// structure used for passing servers' info between methods
struct Servers {

	char dfs1ip[15];
	int dfs1port;
	int dfs1sock;

	char dfs2ip[15];
	int dfs2port;
	int dfs2sock;

	char dfs3ip[15];
	int dfs3port;
	int dfs3sock;

	char dfs4ip[15];
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
void makedir(struct Servers servers, char *dirname); // handle mkdir command
void login(struct Servers servers); // handles a user logging in
void logout(struct Servers servers); // handles a user logging out
struct Servers reconnect(struct Servers servers); // try to reconnect to a server
void writeservers(struct Servers servers, char *message); // write message to all servers
void readservers(struct Servers servers); // reads message from each server
int md5sumhash(char *filename); // compute the md5hash of a file
void puthelper(int serv1, int serv2, FILE *fp, int size);

void get(struct Servers servers, char *filename) {
	//printf("%s\n", filename);
	// get file from servers
	// ask each server for a specific fourth until we get one
	// then repeat for each fourth
	// if you none of them provide a fourth then say file incomplete

}

void puthelper(int serv1, int serv2, FILE *fp, int size) {
	int fourth = 0;
	char buf[MAXBUF];
	char c = 0;
	int bufi = 0;

	bzero(buf, MAXBUF);
	read(serv1, buf, strlen(buf));
	if(strcmp(buf, "SUCCESS") != 0) {
		printf("Failed to create file on server side. %s\n", buf);
		return;
	}
	bzero(buf, MAXBUF);
	read(serv2, buf, strlen(buf));
	if(strcmp(buf, "SUCCESS") != 0) {
		printf("Failed to create file on server side. %s\n", buf);
		return;
	}

	while(fourth <= size) {
		if(bufi >= MAXBUF - 1) {
			write(serv1, buf, strlen(buf));
			write(serv2, buf, strlen(buf));
			bufi = 0;
			bzero(buf, MAXBUF);
		}
		c = (char) fgetc(fp);
		buf[bufi] = c;
		bufi++;
		fourth++;
	}
	write(serv1, buf, strlen(buf));
	write(serv2, buf, strlen(buf));

	bzero(buf, MAXBUF);
	strcpy(buf, "EOF\r\n");
	write(serv1, buf, strlen(buf));
	write(serv2, buf, strlen(buf));

	bzero(buf, MAXBUF);
	read(serv1, buf, strlen(buf));
	if(strcmp(buf, "SUCCESS") != 0)
		printf("Failed to write to server. %s\n", buf);
	bzero(buf, MAXBUF);
	read(serv2, buf, strlen(buf));
	if(strcmp(buf, "SUCCESS") != 0)
		printf("Failed to write to server. %s\n", buf);

}

void put(struct Servers servers, char *filename) {
	//printf("%s\n", filename);
	// put files onto servers
	// split file into fourths and send each fourth
	FILE *fp;
	int size;
	int hashval;
	char buf[MAXBUF];

	fp = fopen(filename, "rb");
	if(fp == NULL) {
		printf("Couldn't open file %s cancelling\n", filename);
		return;
	}

	size = file_size(filename);
	size = size / 4;
	//printf("%d\n", size);

	hashval = md5sumhash(filename);
	hashval = hashval % 4;
	//printf("%d hash\n", hashval);

	bzero(buf, MAXBUF);
	strcpy(buf, "put");
	writeservers(servers, buf);
	if(servers.dfs1sock != -1) {
		bzero(buf, MAXBUF);
		read(servers.dfs1sock, buf, strlen(buf));
		if(strcmp(buf, "Ready") != 0)
			printf("Failed to send put to DFS1\n");
	}
	if(servers.dfs2sock != -1) {
		bzero(buf, MAXBUF);
		read(servers.dfs2sock, buf, strlen(buf));
		if(strcmp(buf, "Ready") != 0)
			printf("Failed to send put to DFS2\n");
	}
	if(servers.dfs3sock != -1) {
		bzero(buf, MAXBUF);
		read(servers.dfs3sock, buf, strlen(buf));
		if(strcmp(buf, "Ready") != 0)
			printf("Failed to send put to DFS3\n");
	}
	if(servers.dfs4sock != -1) {
		bzero(buf, MAXBUF);
		read(servers.dfs4sock, buf, strlen(buf));
		if(strcmp(buf, "Ready") != 0)
			printf("Failed to send put to DFS4\n");
	}
	/*
	switch(hashval) {
		case 0:
			bzero(buf, MAXBUF);
			strcpy(buf, filename);
			strcat(buf, ".1");
			write(servers.dfs1sock, buf, strlen(buf));
			write(servers.dfs4sock, buf, strlen(buf));
			puthelper(servers.dfs1sock, servers.dfs4sock, fp, size);

			bzero(buf, MAXBUF);
			strcpy(buf, filename);
			strcat(buf, ".2");
			write(servers.dfs1sock, buf, strlen(buf));
			write(servers.dfs2sock, buf, strlen(buf));
			puthelper(servers.dfs1sock, servers.dfs2sock, fp, size);

			bzero(buf, MAXBUF);
			strcpy(buf, filename);
			strcat(buf, ".3");
			write(servers.dfs2sock, buf, strlen(buf));
			write(servers.dfs3sock, buf, strlen(buf));
			puthelper(servers.dfs2sock, servers.dfs3sock, fp, size);

			bzero(buf, MAXBUF);
			strcpy(buf, filename);
			strcat(buf, ".4");
			write(servers.dfs4sock, buf, strlen(buf));
			write(servers.dfs1sock, buf, strlen(buf));
			puthelper(servers.dfs4sock, servers.dfs1sock, fp, size);
			break;
		case 1:
			bzero(buf, MAXBUF);
			strcpy(buf, filename);
			strcat(buf, ".1");
			write(servers.dfs1sock, buf, strlen(buf));
			write(servers.dfs4sock, buf, strlen(buf));
			puthelper(servers.dfs1sock, servers.dfs4sock, fp, size);

			bzero(buf, MAXBUF);
			strcpy(buf, filename);
			strcat(buf, ".2");
			write(servers.dfs1sock, buf, strlen(buf));
			write(servers.dfs2sock, buf, strlen(buf));
			puthelper(servers.dfs1sock, servers.dfs2sock, fp, size);

			bzero(buf, MAXBUF);
			strcpy(buf, filename);
			strcat(buf, ".3");
			write(servers.dfs2sock, buf, strlen(buf));
			write(servers.dfs3sock, buf, strlen(buf));
			puthelper(servers.dfs2sock, servers.dfs3sock, fp, size);

			bzero(buf, MAXBUF);
			strcpy(buf, filename);
			strcat(buf, ".4");
			write(servers.dfs4sock, buf, strlen(buf));
			write(servers.dfs1sock, buf, strlen(buf));
			puthelper(servers.dfs4sock, servers.dfs1sock, fp, size);
			break;
		case 2:
			bzero(buf, MAXBUF);
			strcpy(buf, filename);
			strcat(buf, ".1");
			write(servers.dfs1sock, buf, strlen(buf));
			write(servers.dfs4sock, buf, strlen(buf));
			puthelper(servers.dfs1sock, servers.dfs4sock, fp, size);

			bzero(buf, MAXBUF);
			strcpy(buf, filename);
			strcat(buf, ".2");
			write(servers.dfs1sock, buf, strlen(buf));
			write(servers.dfs2sock, buf, strlen(buf));
			puthelper(servers.dfs1sock, servers.dfs2sock, fp, size);

			bzero(buf, MAXBUF);
			strcpy(buf, filename);
			strcat(buf, ".3");
			write(servers.dfs2sock, buf, strlen(buf));
			write(servers.dfs3sock, buf, strlen(buf));
			puthelper(servers.dfs2sock, servers.dfs3sock, fp, size);

			bzero(buf, MAXBUF);
			strcpy(buf, filename);
			strcat(buf, ".4");
			write(servers.dfs4sock, buf, strlen(buf));
			write(servers.dfs1sock, buf, strlen(buf));
			puthelper(servers.dfs4sock, servers.dfs1sock, fp, size);
			break;
		case 3:
			bzero(buf, MAXBUF);
			strcpy(buf, filename);
			strcat(buf, ".1");
			write(servers.dfs1sock, buf, strlen(buf));
			write(servers.dfs4sock, buf, strlen(buf));
			puthelper(servers.dfs1sock, servers.dfs4sock, fp, size);

			bzero(buf, MAXBUF);
			strcpy(buf, filename);
			strcat(buf, ".2");
			write(servers.dfs1sock, buf, strlen(buf));
			write(servers.dfs2sock, buf, strlen(buf));
			puthelper(servers.dfs1sock, servers.dfs2sock, fp, size);

			bzero(buf, MAXBUF);
			strcpy(buf, filename);
			strcat(buf, ".3");
			write(servers.dfs2sock, buf, strlen(buf));
			write(servers.dfs3sock, buf, strlen(buf));
			puthelper(servers.dfs2sock, servers.dfs3sock, fp, size);

			bzero(buf, MAXBUF);
			strcpy(buf, filename);
			strcat(buf, ".4");
			write(servers.dfs4sock, buf, strlen(buf));
			write(servers.dfs1sock, buf, strlen(buf));
			puthelper(servers.dfs4sock, servers.dfs1sock, fp, size);
			break;
		default:
			printf("Couldn't get hashvalue cancelling.\n");
	}
	*/
	fclose(fp);
}

void list(struct Servers servers) {
	// get the files in servers and see if
	// have all 4 parts

}

int md5sumhash(char *filename) {
	unsigned char c[MD5_DIGEST_LENGTH];
	int i;
	FILE *fp = fopen(filename, "rb");
	MD5_CTX mdContext;
	int bytes;
	unsigned char data[1024];
	int value = 0;

	if(fp == NULL) {
		printf("Couldn't open %s for md5hash\n", filename);
		return -1;
	}

	MD5_Init(&mdContext);
	while((bytes = fread(data, 1, 1024, fp)) != 0)
		MD5_Update(&mdContext, data, bytes);
	MD5_Final(c, &mdContext);

	for(i = 0; i < MD5_DIGEST_LENGTH; i++)
		value += c[i];
	return value;
}

void exitcmd(struct Servers servers) {
	// send servers exit signal
	char buf[] = "END CONNECTION\r\n";

	printf("Closing connection to servers.\n");
	if(servers.dfs1sock != -1) {
		write(servers.dfs1sock, buf, strlen(buf));
	}
	if(servers.dfs2sock != -1) {
		write(servers.dfs2sock, buf, strlen(buf));
	}
	if(servers.dfs3sock != -1) {
		write(servers.dfs3sock, buf, strlen(buf));
	}
	if(servers.dfs4sock != -1) {
		write(servers.dfs4sock, buf, strlen(buf));
	}

}

void makedir(struct Servers servers, char *dirname) {
	//printf("%s\n", dirname);
	// make a subdirectory in current directory
	char buf[MAXBUF];

	bzero(buf, MAXBUF);
	strcpy(buf, "mkdir ");
	strcat(buf, dirname);
	writeservers(servers, buf);
	bzero(buf, MAXBUF);
	read(servers.dfs1sock, buf, MAXBUF);
	if(strcmp(buf, "SUCCESS") == 0) {
		printf("DFS1 made directory.\n");
	} else {
		printf("DFS1 failed to make directory.\n");
	}
	bzero(buf, MAXBUF);
	read(servers.dfs2sock, buf, MAXBUF);
	if(strcmp(buf, "SUCCESS") == 0) {
		printf("DFS2 made directory.\n");
	} else {
		printf("DFS2 failed to make directory.\n");
	}
	bzero(buf, MAXBUF);
	read(servers.dfs3sock, buf, MAXBUF);
	if(strcmp(buf, "SUCCESS") == 0) {
		printf("DFS3 made directory.\n");
	} else {
		printf("DFS3 failed to make directory.\n");
	}
	bzero(buf, MAXBUF);
	read(servers.dfs4sock, buf, MAXBUF);
	if(strcmp(buf, "SUCCESS") == 0) {
		printf("DFS4 made directory.\n");
	} else {
		printf("DFS4 failed to make directory.\n");
	}
}

void login(struct Servers servers) {
	// login as a user for private drive
	// username and password
	char buf[MAXBUF];
	int sock = -1;
	char username[32];
	char password[32];

	strcpy(buf, "login");
	if(servers.dfs1sock != -1)
		sock = servers.dfs1sock;
	else if(servers.dfs2sock != -1)
		sock = servers.dfs2sock;
	else if(servers.dfs3sock != -1)
		sock = servers.dfs3sock;
	else if(servers.dfs4sock != -1)
		sock = servers.dfs4sock;
	if(sock == -1) {
		printf("No servers available cancelling login.\n");
		return;
	}
	write(sock, buf, strlen(buf));

	printf("Username: ");
	fgets(buf, MAXBUF, stdin);
	// strtok is destructive heheheeeee
	strtok(buf, "\n");
	while(strlen(buf) > 32 || strlen(buf) < 3 || strchr(buf, ':') != NULL) {
		printf("Username too long or too short or contains :.\n Must be 3-32 characters no :. Try agagin\n");
		printf("Username: ");
		bzero(buf, strlen(buf));
		fgets(buf, MAXBUF, stdin);
		strtok(buf, "\n");
	}
	
	strcpy(username, buf);
	write(sock, buf, strlen(buf));
	bzero(buf, strlen(buf));
	read(sock, buf, MAXBUF);
	if(strcmp(buf, "FUSR") == 0) {
		int attempts = 3;
		while(attempts > 0) {
			printf("Enter password: ");
			bzero(buf, strlen(buf));
			fgets(buf, MAXBUF, stdin);
			strtok(buf, "\n");
			strcpy(password, buf);
			//printf("%s\n", buf);
			write(sock, buf, strlen(buf));
			bzero(buf, MAXBUF);
			read(sock, buf, MAXBUF);
			if(strcmp(buf, "SUCCESS") == 0) {
				bzero(buf, strlen(buf));
				strcpy(buf, "logspc ");
				strcat(buf, username);
				strcat(buf, ":");
				strcat(buf, password);
				writeservers(servers, buf);
				printf("Logged in.\n");
				break;
			}
			attempts--;
			printf("Wrong password try again. %d attempts remaining\n", attempts);
		}
	} else {
		printf("User doesn't exist. Create user? (y/n): ");
		bzero(buf, strlen(buf));
		fgets(buf, MAXBUF, stdin);
		strtok(buf, "\n");
		if(strcmp(buf, "y") == 0) {
			printf("Enter password: ");
			bzero(buf, strlen(buf));
			fgets(buf, MAXBUF, stdin);
			strtok(buf, "\n");
			while(strlen(buf) > 32 || strlen(buf) < 3 || strchr(buf, ':') != NULL) {
				printf("Password must be 3-32 characters long and not contain :.\n Try again: ");
				bzero(buf, strlen(buf));
				fgets(buf, MAXBUF, stdin);
				strtok(buf, "\n");
			}
			write(sock, buf, strlen(buf));
			strcpy(password, buf);
			bzero(buf, strlen(buf));
			read(sock, buf, MAXBUF);
			bzero(buf, strlen(buf));
			strcpy(buf, "logspc ");
			strcat(buf, username);
			strcat(buf, ":");
			strcat(buf, password);
			writeservers(servers, buf);
		} else {
			bzero(buf, strlen(buf));
			strcpy(buf, "n");
			//printf("%s\n", buf);
			write(sock, buf, strlen(buf));
			bzero(buf, strlen(buf));
			read(sock, buf, MAXBUF);
		}
	}

}

void logout(struct Servers servers) {
	// logout of the user so they can access shared drive again
	char buf[] = "logout";
	char ret[MAXBUF];

	printf("Logging out of servers.\n");
	if(servers.dfs1sock != -1) {
		write(servers.dfs1sock, buf, strlen(buf));
		bzero(ret, strlen(ret));
		read(servers.dfs1sock, ret, MAXBUF);
		if(strcmp(ret, "1") == 0)
			printf("DFS1 Success.\n");
		else
			printf("DFS1 Failed.\n");
	}
	if(servers.dfs2sock != -1) {
		write(servers.dfs2sock, buf, strlen(buf));
		bzero(ret, strlen(ret));
		read(servers.dfs2sock, ret, MAXBUF);
		if(strcmp(ret, "1") == 0)
			printf("DFS2 Success.\n");
		else
			printf("DFS2 Failed.\n");
	}
	if(servers.dfs3sock != -1) {
		write(servers.dfs3sock, buf, strlen(buf));
		bzero(ret, strlen(ret));
		read(servers.dfs3sock, ret, MAXBUF);
		if(strcmp(ret, "1") == 0)
			printf("DFS3 Success.\n");
		else
			printf("DFS3 Failed.\n");
	}
	if(servers.dfs4sock != -1) {
		write(servers.dfs4sock, buf, strlen(buf));
		bzero(ret, strlen(ret));
		read(servers.dfs4sock, ret, MAXBUF);
		if(strcmp(ret, "1") == 0)
			printf("DFS4 Success.\n");
		else
			printf("DFS4 Failed.\n");
	}
}


// I made this message a little later than I should have
// so some things that should use this don't
// just sends a message to each server
void writeservers(struct Servers servers, char *message) {
	if(servers.dfs1sock != -1)
		write(servers.dfs1sock, message, strlen(message));
	if(servers.dfs2sock != -1)
		write(servers.dfs2sock, message, strlen(message));
	if(servers.dfs3sock != -1)
		write(servers.dfs3sock, message, strlen(message));
	if(servers.dfs4sock != -1)
		write(servers.dfs4sock, message, strlen(message));
}

void readservers(struct Servers servers) {
	char buf[MAXBUF];

	if(servers.dfs1sock != -1) {
		bzero(buf, MAXBUF);
		read(servers.dfs1sock, buf, strlen(buf));
	}
	if(servers.dfs2sock != -1) {
		bzero(buf, MAXBUF);
		read(servers.dfs2sock, buf, strlen(buf));
	}
	if(servers.dfs3sock != -1) {
		bzero(buf, MAXBUF);
		read(servers.dfs3sock, buf, strlen(buf));
	}
	if(servers.dfs4sock != -1) {
		bzero(buf, MAXBUF);
		read(servers.dfs4sock, buf, strlen(buf));
	}
}

struct Servers reconnect(struct Servers servers) {
	// check if still connected to each server
	// for each server not connected to try to reconnect
	char buf[] = "Connected";

	if(servers.dfs1sock == -1) {
		printf("Not connected to DFS1 attempting connect.\n");
		servers.dfs1sock = connectserver(servers.dfs1ip, servers.dfs1port);
		write(servers.dfs1sock, buf, strlen(buf));
	}
	if(servers.dfs2sock == -1) {
		printf("Not connected to DFS2 attempting connect.\n");
		servers.dfs2sock = connectserver(servers.dfs2ip, servers.dfs2port);
		write(servers.dfs2sock, buf, strlen(buf));
	}
	if(servers.dfs3sock == -1) {
		printf("Not connected to DFS3 attempting connect.\n");
		servers.dfs3sock = connectserver(servers.dfs3ip, servers.dfs3port);
		write(servers.dfs3sock, buf, strlen(buf));
	}
	if(servers.dfs4sock == -1) {
		printf("Not connected to DFS4 attempting connect.\n");
		servers.dfs4sock = connectserver(servers.dfs4ip, servers.dfs4port);
		write(servers.dfs4sock, buf, strlen(buf));
	}
	return servers;
}

void clientlogic(struct Servers servers) {
	char buf[MAXBUF];
	char spacedelim[] = " \r\n"; // for splitting on space
	char *tokked; //  result of strtok
	int optval = 1;
	socklen_t optlen = sizeof(optval);

	// preliminary connected message
	strcpy(buf, "Connected");
	if(servers.dfs1sock != -1)
		write(servers.dfs1sock, buf, strlen(buf));
	if(servers.dfs2sock != -1)
		write(servers.dfs2sock, buf, strlen(buf));
	if(servers.dfs3sock != -1)
		write(servers.dfs3sock, buf, strlen(buf));
	if(servers.dfs4sock != -1)
		write(servers.dfs4sock, buf, strlen(buf));
	// do this forever
	while(1) {
		//printf("%s:%d\n", servers.dfs1ip, servers.dfs1port);

		// clear the buff
		bzero(buf, MAXBUF);
		printf("--------------\n");
		printf("Command: ");
		fgets(buf, MAXBUF, stdin);

		if(strncmp(buf, "exit", 4) == 0) {
			exitcmd(servers);
			break;
		} else if(strncmp(buf, "get", 3) == 0) {
			tokked = strtok(buf, spacedelim);
			tokked = strtok(NULL, spacedelim);
			if(strncmp(tokked, "users/", 6) == 0)
				printf("Users is protected directory.\n");
			else
				get(servers, tokked);
		} else if(strncmp(buf, "put", 3) == 0) {
			tokked = strtok(buf, spacedelim);
			tokked = strtok(NULL, spacedelim);
			if(strncmp(tokked, "users/", 6) == 0)
				printf("Users is protected directory.\n");
			else
				put(servers, tokked);
		} else if(strncmp(buf, "list", 4) == 0) {
			list(servers);
		} else if(strncmp(buf, "mkdir", 5) == 0) {
			tokked = strtok(buf, spacedelim);
			tokked = strtok(NULL, spacedelim);
			if(tokked == NULL)
				printf("Invalid directory cancelling.\n");
			else if(strcmp(tokked, "users") == 0)
				printf("Protected directory can't make.\n");
			else
				makedir(servers, tokked);
		} else if(strncmp(buf, "login", 5) == 0) {
			login(servers);
		} else if(strncmp(buf, "logout", 6) == 0) {
			logout(servers);
		} else if(strncmp(buf, "connect", 6) == 0) {
			servers = reconnect(servers);
		} else {
			printf("Command not recognized. Available commands are:\n");
			printf("exit, get, put, list, mkdir, login, logout, connect\n");
		}
		// this is for testing get rid of when done
		/*
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
		*/
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
		strcpy(servers.dfs1ip, strtok(NULL, colondelim));
		//servers.dfs1ip = strtok(NULL, colondelim);
		servers.dfs1port = atoi(strtok(NULL, colondelim));
	} else {
		printf(".conf not formatted correctly. Please reference .conf provided.\n");
		exit(0);
	}
	//printf("read %s:%d\n", servers.dfs1ip, servers.dfs1port);
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
		strcpy(servers.dfs2ip, strtok(NULL, colondelim));
		//servers.dfs2ip = strtok(NULL, colondelim);
		servers.dfs2port = atoi(strtok(NULL, colondelim));
		//printf("yo\n");
	} else {
		printf(".conf not formatted correctly. Please reference .conf provided.\n");
		exit(0);
	}
	//printf("read %s:%d\n", servers.dfs2ip, servers.dfs2port);
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
		strcpy(servers.dfs3ip, strtok(NULL, colondelim));
		//servers.dfs3ip = strtok(NULL, colondelim);
		servers.dfs3port = atoi(strtok(NULL, colondelim));
	} else {
		printf(".conf not formatted correctly. Please reference .conf provided.\n");
		exit(0);
	}
	//printf("read %s:%d\n", servers.dfs3ip, servers.dfs3port);
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
		strcpy(servers.dfs4ip, strtok(NULL, colondelim));
		//servers.dfs4ip = strtok(NULL, colondelim);
		servers.dfs4port = atoi(strtok(NULL, colondelim));
	} else {
		printf(".conf not formatted correctly. Please reference .conf provided.\n");
		exit(0);
	}
	//printf("read %s:%d\n", servers.dfs4ip, servers.dfs4port);
	return servers;
}