// imports yay
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <dirent.h>

#define MAXBUF 8192 // max IO buffer between client and server
#define LISTENQ 1024 // second argument to listen()

// stores username stuff
struct User {
	char username[32];
	char password[32];
};

int open_listenfd(int port); // handle some connection stuff
void logic(int connfd); // actual logic of the server or at least the start of it
void *thread(void *vargp); // handle the threading stuff
void get(int con, struct User user, char *filename); // handle get command
void put(int con, struct User user); // handle put command
void list(int con, struct User user, char *directory); // handle list command
void makedir(int con, char *buf, struct User user); // handle mkdir command
struct User login(int con); // handle login command
struct User logspc(int con, char *buf); // a special login to get around all the password stuff
// users can't access this command bc it's special
void test(); // I got tired of doing printf("some random statement")
void testm(char message[]); // again, printf got annoyting bc \n uhg
void puthelper(int con, struct User user); // put needed a helper so I don't copy so much

char root[5]; // root folder we are in
// ie DFS1/, DFS2/, ... 

void get(int con, struct User user, char *filename) {


}

// the list command
void list(int con, struct User user, char *directory) {
	struct dirent *de;
	DIR *dr;
	char dir[MAXBUF];
	char buf[MAXBUF];

	// ok, the directory we want to put it in
	// is slightly different from what they sent
	// like each dfs has has it's DFS# folder and
	// then users data if your logged in
	strcpy(dir, root);
	if(strlen(user.username) != 0) {
		strcat(dir, "users/");
		strcat(dir, user.username);
		strcat(dir, "/");
	}
	strcat(dir, directory);

	// open the directory and send to client 
	// whether we were able to open or not
	dr = opendir(dir);
	//printf("dir: %s\n", directory);
	if(dr == NULL) {
		bzero(buf, MAXBUF);
		strcpy(buf, "failed to open ");
		strcat(buf, dir);
		write(con, buf, strlen(buf));
		return;
	}
	bzero(buf, MAXBUF);
	strcpy(buf, "opened");
	printf("opened %s\n", dir);
	write(con, buf, strlen(buf));
	read(con, buf, MAXBUF);

	// write each directory name to the client
	while((de = readdir(dr)) != NULL) {
		bzero(buf, MAXBUF);
		strcpy(buf, de->d_name);
		if(strcmp(buf, ".") != 0 && strcmp(buf, "..") != 0) {
			printf("%s\n", de->d_name);
			//printf("writing %s\n", buf);
			write(con, buf, strlen(buf));
			read(con, buf, MAXBUF);
		}
	}
	
	// send the signal to client that there are no more directories
	bzero(buf, MAXBUF);
	strcpy(buf, "N M D");
	//printf("writing N M D\n");
	write(con, buf, strlen(buf));
	
	
	closedir(dr);
	//printf("end of list\n");
}

// put command
void put(int con, struct User user) {
	char buf[MAXBUF];
	FILE *fp;
	char filename[MAXBUF];

	// I got tired of printf formatting and crap
	//test();
	// send to client that we are ready for put
	bzero(buf, MAXBUF);
	strcpy(buf, "ready");
	write(con, buf, strlen(buf));

	//test();
	// put does the same thing twice and I didn't want to copy paste so helper
	// cause for each put you get two files, ie 1, 2 or 4, 1
	puthelper(con, user);
	puthelper(con, user);
}

// actually does most of the work for put
void puthelper(int con, struct User user) {
	char buf[MAXBUF];
	FILE *fp;
	char filename[MAXBUF];
	int n;

	//testm("Making file");
	// get filename from client and put it in buf
	bzero(buf, MAXBUF);
	read(con, buf, MAXBUF);
	// first make sure we are putting in the right place
	// cause you gotta do the whole user thing and the DFS# thing and yea
	bzero(filename, MAXBUF);
	strcpy(filename, root);
	if(strlen(user.username) != 0) {
		strcat(filename, "users/");
		strcat(filename, user.username);
		strcat(filename, "/");
	}
	strcat(filename, buf);
	fp = fopen(filename, "wb");
	if(fp == NULL) {
		testm("null");
		bzero(buf, MAXBUF);
		strcpy(buf, "Failed to create ");
		strcat(buf, filename);
		write(con, buf, strlen(buf));
		return;
	}
	//testm("not null");
	bzero(buf, MAXBUF);
	strcpy(buf, "created ");
	strcat(buf, filename);
	write(con, buf, strlen(buf));

	
	//testm("reading file");
	bzero(buf, MAXBUF);
	strcpy(buf, "start");
	// while buf != EOF keep reading from the client
	while(strcmp(buf, "EOF\r\n") != 0) {
		bzero(buf, MAXBUF);
		n = read(con, buf, MAXBUF);
		//printf("got %ld\n", sizeof(buf));
		if(strcmp(buf, "EOF\r\n") != 0) {
			//fputs(buf, fp);
			// write to the file using fwrite so we can write files of type
			// .jpg and stuff
			fwrite(buf, 1, n, fp);
			bzero(buf, MAXBUF);
			strcpy(buf, "ACK");
			//printf("sending %s\n", buf);
			write(con, buf, strlen(buf));
		}
	}

	
	//testm("sending success");
	// let the client know we did it we're so cool
	bzero(buf, MAXBUF);
	strcpy(buf, "Successfully wrote ");
	strcat(buf, filename);
	printf("%s\n", buf);
	//printf("sending %s\n", buf);
	write(con, buf, strlen(buf));
	
	
	fclose(fp);
	//printf("put helper done\n");
}

// I got tired of writing printf
void test() {
	printf("Test\n");
}

void testm(char message[]) {
	printf("%s\n", message);

}

// make directory command
void makedir(int con, char *buf, struct User user) {
	char delim[] = " \r\n";
	char *tokked;
	//char buf[MAXBUF];
	int check;
	char ret[MAXBUF];

	// this is really similar to just the other commands
	// gotta do the whole correct place thing if logged in
	// and actually DFS# not just base folder
	bzero(ret, MAXBUF);
	strcpy(ret, root);
	if(strlen(user.username) != 0) {
		strcat(ret, "users/");
		strcat(ret, user.username);
		strcat(ret, "/");
	}
	tokked = strtok(buf, delim);
	tokked = strtok(NULL, delim);
	strcat(ret, tokked);
	// try to make the directory
	check = mkdir(ret, 0777);
	// let client know if we failed or succeeded 
	if(!check) {
		printf("Directory %s created\n", ret);
		bzero(buf, MAXBUF);
		strcpy(buf, "SUCCESS");
		write(con, buf, strlen(buf));
	} else {
		printf("Unable to make %s directory\n", ret);
		bzero(buf, MAXBUF);
		strcpy(buf, "FAILED");
		write(con, buf, strlen(buf));
	}
}

// ok, so the whole login sequence is a pain
// so do a special login that is super simple and not a pain
// so that I don't have to do the entire login sequence 
// for each server which would be awful
struct User logspc(int con, char *buf) {
	struct User user;
	char delim[] = " :\r\n";
	char *tokked;
	char dir[MAXBUF];

	// get the username and password and just login as that user
	// no search or check or anything
	tokked = strtok(buf, delim);
	tokked = strtok(NULL, delim);
	strcpy(user.username, tokked);
	tokked = strtok(NULL, delim);
	strcpy(user.password, tokked);
	printf("User %s logged in.\n", user.username);
	bzero(dir, MAXBUF);
	strcpy(dir, root);
	strcat(dir, "users/");
	strcat(dir, user.username);
	//printf("%s\n", dir);
	//printf("%d\n", mkdir(dir, 0777));
	// ok, so users are actually folders, bc that makes sense
	// each user has their own private folders in the directory users
	mkdir(dir, 0777);
	return user;
}

// the login sequence for users
struct User login(int con) {
	char buf[MAXBUF];
	struct User user;
	struct User temp;
	char filebuf[MAXBUF];
	char colondelim[] = ":\r\n";
	char *tokked;
	int fusr = 0;
	FILE *fp = fopen("dfs.conf", "r+");
	// dfs.conf stores our user data, if that doesn't exist then make a new one
	if(fp == NULL) {
		printf("Unable to open dfs.conf creating new.\n");
		fp = fopen("dfs.conf", "w+");
		if(fp == NULL) {
			printf("Oh no. That's bad....idk what to do about this\n");
			printf("Couldn't even creat dfs.conf...crash ig???\n");
			return user;
		}
	}

	//printf("Attempting login.\n");
	// read the users data and check if that user exists
	bzero(buf, strlen(buf));
	read(con, buf, MAXBUF);
	strcpy(user.username, buf);
	while(fgets(buf, MAXBUF, fp) != NULL) {
		tokked = strtok(buf, colondelim);
		strcpy(temp.username, tokked);
		tokked = strtok(NULL, colondelim);
		strcpy(temp.password, tokked);
		//printf("{%s}, {%s}\n", user.username, temp.username);
		// if the user exists then we can quit and send that the user
		// exists to the client
		if(strcmp(temp.username, user.username) == 0) {
			bzero(buf, strlen(buf));
			strcpy(buf, "FUSR");
			write(con, buf, strlen(buf));
			fusr = 1;
			break;
		}
		bzero(buf, strlen(buf));
	}
	fclose(fp);
	// user existed so try to login
	if(fusr == 1) {
		int attempts = 3;
		while(attempts > 0) {
			// client sends some passwords
			bzero(buf, MAXBUF);
			read(con, buf, MAXBUF);
			//printf("{%s}, {%s}\n", buf, temp.password);
			// we send whether it good or bad password
			if(strcmp(buf, temp.password) == 0) {
				bzero(buf, strlen(buf));
				strcpy(buf, "SUCCESS");
				write(con, buf, strlen(buf));
				return temp;
			} else {
				bzero(buf, strlen(buf));
				strcpy(buf, "FAILED");
				write(con, buf, strlen(buf));
				attempts--;
			}
		}
	// if the user didn't exist let's make a new one maybe
	} else {
		bzero(buf, strlen(buf));
		strcpy(buf, "NUSR");
		write(con, buf, strlen(buf));
		bzero(buf, MAXBUF);
		//printf("expecting read\n");
		read(con, buf, MAXBUF);
		bzero(user.password, 32);
		strcpy(user.password, buf);
		//printf("got read %s\n", buf);
		bzero(buf, strlen(buf));
		strcpy(buf, "SUCCESS");
		write(con, buf, strlen(buf));
		// user didn't exist and they want to make a new one
		if(strcmp(user.password, "n") != 0) {
			//printf("%s\n", buf);
			//strcpy(user.password, buf);
			// write this new user and their password to dfs.conf for all to have
			// and future use
			bzero(buf, strlen(buf));
			strcpy(buf, user.username);
			strcat(buf, ":");
			strcat(buf, user.password);
			strcat(buf, "\n");
			//printf("{%s}\n", buf);
			fp = fopen("dfs.conf", "a");
			fputs(buf, fp);
			//fprintf(fp, buf);
			fclose(fp);
			return user;
		}
	}

}

// main server logic ig? actually this logic is for each 
// connection so it's actually just handle the client commands
// which is mostly just call the appropriate method and pass the right stuff
void logic(int connfd) {
	size_t n;
	char buf[MAXBUF];
	int optval = 1;
	socklen_t optlen = sizeof(optval);
	struct User user; // this is actually the user we are / the client is logged in as
	bzero(user.username, strlen(user.username));
	bzero(user.password, strlen(user.password));
	char *tokked;
	char delim[] = " \r\n";

	bzero(buf, MAXBUF);
	n = read(connfd, buf, MAXBUF);
	printf("Client connected.\n");
	//printf("server received:\n{%s}\n", buf);
	// this is really just switch based on what the client sent us
	while(strcmp(buf, "END CONNECTION\r\n") != 0) {
		printf("-------------\n");
		if(strcmp(buf, "") == 0) {
			printf("Recieved empty buf assuming dead client closing connection\n");
			return;
		}
		bzero(buf, MAXBUF);
		n = read(connfd, buf, MAXBUF);
		//printf("server received:\n{%s}\n", buf);
		if(strncmp(buf, "get", 3) == 0) {
			tokked = strtok(buf, delim);
			tokked = strtok(NULL, delim);
			if(tokked == NULL) {
				printf("Invalid filename for get\n");
			} else {
				get(connfd, user, tokked);
			}
		} else if(strncmp(buf, "put", 3) == 0) {
			//printf("Uhg %s\n", buf);
			put(connfd, user);
		} else if(strncmp(buf, "list", 4) == 0) {
			tokked = strtok(buf, delim);
			tokked = strtok(NULL, delim);
			list(connfd, user, tokked);
		} else if(strncmp(buf, "mkdir", 5) == 0) {
			makedir(connfd, buf, user);
		} else if(strncmp(buf, "login", 5) == 0) {
			user = login(connfd);
		} else if(strncmp(buf, "logout", 6) == 0) {
			printf("logging out of %s\n", user.username);
			bzero(user.username, 32);
			bzero(user.password, 32);
			bzero(buf, MAXBUF);
			strcpy(buf, "1");
			write(connfd, buf, sizeof(buf));
		} else if(strncmp(buf, "logspc", 6) == 0) {
			user = logspc(connfd, buf);
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
	char users[16];

	if(argc < 3) {
		printf("Wrong number of arguments expected at least dfs DFS# port#");
		exit(0);
	}

	// oh, this is making the folders we need
	// each server needs it's folder ie DFS1 DFS2 so on
	// and they also need the users folder for when
	// someone logs in
	port = atoi(argv[2]);
	strcpy(root, argv[1]);
	bzero(users, 16);
	strcpy(users, root);
	mkdir(users, 0777);
	strcat(users, "/users");
	mkdir(users, 0777);
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