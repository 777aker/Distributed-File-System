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

// this is a structure used by list to keep track of files
// and if they are complete
struct File {

	char name[MAXBUF];
	int available;
	int one;
	int two;
	int three;
	int four;

};

// a whole lot of function declarations
// more description provided above implementation of each function
struct Servers handleconf(FILE *fp); // read's the dfc.conf file and parses info
void connectservers(struct Servers servers); // connect to the 4 servers
void clientlogic(struct Servers servers); // where the client's processing and actions happen
long file_size(char *name); // get a file's size
void get(struct Servers servers, char *filename); // handle client doing get command
void put(struct Servers servers, char *filename); // handle client doing put command
void list(struct Servers servers, char *directory); // handle client doing list command
int connectserver(char *ip, int port); // helper that connects to one server
void exitcmd(struct Servers servers); // handle client doing exit command
void makedir(struct Servers servers, char *dirname); // handle mkdir command
void login(struct Servers servers); // handles a user logging in
void logout(struct Servers servers); // handles a user logging out
struct Servers reconnect(struct Servers servers); // try to reconnect to a server
void writeservers(struct Servers servers, char *message); // write message to all servers
void readservers(struct Servers servers); // reads message from each server
int md5sumhash(char *filename); // compute the md5hash of a file
// a helper for the put command so there isn't so much repeated code
void puthelper(int serv1, int serv2, FILE *fp, int size, char filename[], int filen);
int listhash(char *name); // a hasher for list so i can make a cool dir structure

void get(struct Servers servers, char *filename) {
	//printf("%s\n", filename);
	// get file from servers
	// ask each server for a specific fourth until we get one
	// then repeat for each fourth
	// if you none of them provide a fourth then say file incomplete


}

// this handles the list command which displays subdirectories and files
// in a directory specified
void list(struct Servers servers, char *directory) {
	// get the files in servers and see if
	// have all 4 parts
	char dir[MAXBUF]; // actual directory to look into
	char buf[MAXBUF]; // buf used for reading writing and various nonsense
	char ret[MAXBUF]; // need a buffer that won't overwrite buf
	struct File files[50]; // this array stores the files and directories and their info
	//struct File testfile;
	int numfiles = 0; // how many files there are
	char ack[MAXBUF]; // ack packet
	//char *tokked;
	int i; // for for loops
	int exists; // for checking if a file already exists or not
	char tmpbuf[MAXBUF];
	int end;

	//bzero(buf, MAXBUF);
	//strcpy(buf, "Paul Johnson");
	//strcpy(files[numfiles].name, buf);
	//printf("test: %s\n", files[numfiles].name);

	// this bit is checking if they actually specified a directory
	// if they didn't then we are opening the current root so do a .
	bzero(dir, MAXBUF);
	if(directory == NULL) {
		strcpy(dir, ".");
		printf("No directory specified opening default.\n");
	} else {
		strcpy(dir, directory);
	}

	// making the command to send to the servers
	bzero(buf, MAXBUF);
	strcpy(buf, "list ");
	strcat(buf, dir);

	// sending the command to the servers "list directory"
	bzero(ret, MAXBUF);
	if(servers.dfs1sock != -1) {
		//printf("writing %s\n", buf);
		write(servers.dfs1sock, buf, strlen(buf));
		read(servers.dfs1sock, ret, MAXBUF);
		if(strcmp(ret, "opened") != 0)
			servers.dfs1sock = -1;
		//printf("%s\n", ret);
	}
	bzero(ret, MAXBUF);
	if(servers.dfs2sock != -1) {
		//printf("writing %s\n", buf);
		write(servers.dfs2sock, buf, strlen(buf));
		read(servers.dfs2sock, ret, MAXBUF);
		if(strcmp(ret, "opened") != 0)
			servers.dfs2sock = -1;
		//printf("%s\n", ret);
	}
	bzero(ret, MAXBUF);
	if(servers.dfs3sock != -1) {
		//printf("writing %s\n", buf);
		write(servers.dfs3sock, buf, strlen(buf));
		read(servers.dfs3sock, ret, MAXBUF);
		if(strcmp(ret, "opened") != 0)
			servers.dfs3sock = -1;
		//printf("%s\n", ret);
	}
	bzero(ret, MAXBUF);
	if(servers.dfs4sock != -1) {
		//printf("writing %s\n", buf);
		write(servers.dfs4sock, buf, strlen(buf));
		read(servers.dfs4sock, ret, MAXBUF);
		if(strcmp(ret, "opened") != 0)
			servers.dfs4sock = -1;
		//printf("%s\n", ret);
	}

	// letting the servers know we finished 
	// sending list directories and are ready for the next phase
	bzero(buf, MAXBUF);
	strcpy(buf, "continue");
	writeservers(servers, buf);

	// this next part is a lot and I should've written a method for it 
	// but it's 3am and I'm tired

	// now we actually get each servers dir
	bzero(buf, MAXBUF);
	strcpy(buf, "lets gooo");
	bzero(ack, MAXBUF);
	strcpy(ack, "ACK");
	if(servers.dfs1sock != -1) {
		// while we don't get N M D which means no more directories
		// keep reading from the server
		while(strcmp(buf, "N M D") != 0) {
			bzero(buf, MAXBUF);
			read(servers.dfs1sock, buf, MAXBUF);
			//printf("read %s\n", buf);
			if(strcmp(buf, "N M D") != 0) {
				//printf("writing\n");
				write(servers.dfs1sock, ack, strlen(ack));
				//break;
				end = 0;
				// ok, so here we figure out if it's a fourth of a file
				// through our amazing extension method
				// we also remove the extension for printing just the filename
				if(strcmp(buf+strlen(buf)-2, ".1") == 0) {
					bzero(tmpbuf, MAXBUF);
					strncpy(tmpbuf, buf, strlen(buf)-2);
					bzero(buf, MAXBUF);
					strcpy(buf, tmpbuf);
					end = 1;
				} else if(strcmp(buf+strlen(buf)-2, ".2") == 0) {
					bzero(tmpbuf, MAXBUF);
					strncpy(tmpbuf, buf, strlen(buf)-2);
					bzero(buf, MAXBUF);
					strcpy(buf, tmpbuf);
					end = 2;
				} else if(strcmp(buf+strlen(buf)-2, ".3") == 0) {
					bzero(tmpbuf, MAXBUF);
					strncpy(tmpbuf, buf, strlen(buf)-2);
					bzero(buf, MAXBUF);
					strcpy(buf, tmpbuf);
					end = 3;
				} else if(strcmp(buf+strlen(buf)-2, ".4") == 0) {
					bzero(tmpbuf, MAXBUF);
					strncpy(tmpbuf, buf, strlen(buf)-2);
					bzero(buf, MAXBUF);
					strcpy(buf, tmpbuf);
					end = 4;
				}
				// check to see if the file already exists
				exists = 0;
				for(i = 0; i < numfiles; i++) {
					if(strcmp(files[i].name, buf) == 0) {
						exists = 1;
						break;
					}
				}
				// if it doesn't exist add it to our list of files
				if(exists == 0) {
					// intialize everything that needs to be intialized
					strcpy(files[numfiles].name, buf);
					files[numfiles].one = 0;
					files[numfiles].two = 0;
					files[numfiles].three = 0;
					files[numfiles].four = 0;
					files[numfiles].available = 0;
					// so available actually keeps track of how many servers
					// have a directory, since this one had it increment by one
					if(end == 0)
						files[numfiles].available++;
					// increment the number of files
					numfiles++;
				// so if it did exist and it's not a special extension file
				// increment how many servers have it
				// the for loop quit when it found it and it found it at i so
				// i is actually which file we found which makes this easier
				} else if(end == 0) {
					files[i].available++;
				}
				// now depending on the end we got add to the struct which end we got
				// important later
				// we can just edit files[i] since i is where we found this file
				// entry at
				switch(end) {
					case 1:
						files[i].one = 1;
						break;
					case 2:
						files[i].two = 1;
						break;
					case 3:
						files[i].three = 1;
						break;
					case 4:
						files[i].four = 1;
						break;
					default:
						break;
				}

			}
			// we just got a directory, not we need to put it into
			// our list of directories
			// check if it's already in our list, if not
			// put it into our list
			
			
			
			//printf("%d: buf %s\n", numfiles, buf);
			//bzero(files[numfiles]->name, MAXBUF);
			//strcpy(files[numfiles].name, "Paul Johnson");
			//strcpy(files[numfiles]->name, buf);
			//files[numfiles].name = buf;
			//numfiles++;
		}
	}
	// same thing as dfs1 above but do it for each server now
	bzero(buf, MAXBUF);
	strcpy(buf, "lets gooo");
	if(servers.dfs2sock != -1) {
		while(strcmp(buf, "N M D") != 0) {
			bzero(buf, MAXBUF);
			read(servers.dfs2sock, buf, MAXBUF);
			//printf("read %s\n", buf);
			if(strcmp(buf, "N M D") != 0) {
				write(servers.dfs2sock, ack, strlen(ack));
				//break;
				end = 0;
				// ok, so here we figure out if it's a fourth of a file
				// through our amazing extension method
				// we also remove the extension for printing just the filename
				if(strcmp(buf+strlen(buf)-2, ".1") == 0) {
					bzero(tmpbuf, MAXBUF);
					strncpy(tmpbuf, buf, strlen(buf)-2);
					bzero(buf, MAXBUF);
					strcpy(buf, tmpbuf);
					end = 1;
				} else if(strcmp(buf+strlen(buf)-2, ".2") == 0) {
					bzero(tmpbuf, MAXBUF);
					strncpy(tmpbuf, buf, strlen(buf)-2);
					bzero(buf, MAXBUF);
					strcpy(buf, tmpbuf);
					end = 2;
				} else if(strcmp(buf+strlen(buf)-2, ".3") == 0) {
					bzero(tmpbuf, MAXBUF);
					strncpy(tmpbuf, buf, strlen(buf)-2);
					bzero(buf, MAXBUF);
					strcpy(buf, tmpbuf);
					end = 3;
				} else if(strcmp(buf+strlen(buf)-2, ".4") == 0) {
					bzero(tmpbuf, MAXBUF);
					strncpy(tmpbuf, buf, strlen(buf)-2);
					bzero(buf, MAXBUF);
					strcpy(buf, tmpbuf);
					end = 4;
				}
				// check to see if the file already exists
				exists = 0;
				for(i = 0; i < numfiles; i++) {
					if(strcmp(files[i].name, buf) == 0) {
						exists = 1;
						break;
					}
				}
				// if it doesn't exist add it to our list of files
				if(exists == 0) {
					// intialize everything that needs to be intialized
					strcpy(files[numfiles].name, buf);
					files[numfiles].one = 0;
					files[numfiles].two = 0;
					files[numfiles].three = 0;
					files[numfiles].four = 0;
					files[numfiles].available = 0;
					// so available actually keeps track of how many servers
					// have a directory, since this one had it increment by one
					if(end == 0)
						files[numfiles].available++;
					// increment the number of files
					numfiles++;
				// so if it did exist and it's not a special extension file
				// increment how many servers have it
				// the for loop quit when it found it and it found it at i so
				// i is actually which file we found which makes this easier
				} else if(end == 0) {
					files[i].available++;
				}
				// now depending on the end we got add to the struct which end we got
				// important later
				// we can just edit files[i] since i is where we found this file
				// entry at
				switch(end) {
					case 1:
						files[i].one = 1;
						break;
					case 2:
						files[i].two = 1;
						break;
					case 3:
						files[i].three = 1;
						break;
					case 4:
						files[i].four = 1;
						break;
					default:
						break;
				}
			}
			
			//printf("%d: buf %s\n", numfiles, buf);
			//bzero(files[numfiles]->name, MAXBUF);
			//strcpy(files[numfiles].name, "Paul Johnson");
			//strcpy(files[numfiles]->name, buf);
			//files[numfiles].name = buf;
			//numfiles++;
		}
	}
	bzero(buf, MAXBUF);
	strcpy(buf, "lets gooo");
	if(servers.dfs3sock != -1) {
		while(strcmp(buf, "N M D") != 0) {
			bzero(buf, MAXBUF);
			read(servers.dfs3sock, buf, MAXBUF);
			//printf("read %s\n", buf);
			if(strcmp(buf, "N M D") != 0) {
				write(servers.dfs3sock, ack, strlen(ack));
				//break;
				end = 0;
				// ok, so here we figure out if it's a fourth of a file
				// through our amazing extension method
				// we also remove the extension for printing just the filename
				if(strcmp(buf+strlen(buf)-2, ".1") == 0) {
					bzero(tmpbuf, MAXBUF);
					strncpy(tmpbuf, buf, strlen(buf)-2);
					bzero(buf, MAXBUF);
					strcpy(buf, tmpbuf);
					end = 1;
				} else if(strcmp(buf+strlen(buf)-2, ".2") == 0) {
					bzero(tmpbuf, MAXBUF);
					strncpy(tmpbuf, buf, strlen(buf)-2);
					bzero(buf, MAXBUF);
					strcpy(buf, tmpbuf);
					end = 2;
				} else if(strcmp(buf+strlen(buf)-2, ".3") == 0) {
					bzero(tmpbuf, MAXBUF);
					strncpy(tmpbuf, buf, strlen(buf)-2);
					bzero(buf, MAXBUF);
					strcpy(buf, tmpbuf);
					end = 3;
				} else if(strcmp(buf+strlen(buf)-2, ".4") == 0) {
					bzero(tmpbuf, MAXBUF);
					strncpy(tmpbuf, buf, strlen(buf)-2);
					bzero(buf, MAXBUF);
					strcpy(buf, tmpbuf);
					end = 4;
				}
				// check to see if the file already exists
				exists = 0;
				for(i = 0; i < numfiles; i++) {
					if(strcmp(files[i].name, buf) == 0) {
						exists = 1;
						break;
					}
				}
				// if it doesn't exist add it to our list of files
				if(exists == 0) {
					// intialize everything that needs to be intialized
					strcpy(files[numfiles].name, buf);
					files[numfiles].one = 0;
					files[numfiles].two = 0;
					files[numfiles].three = 0;
					files[numfiles].four = 0;
					files[numfiles].available = 0;
					// so available actually keeps track of how many servers
					// have a directory, since this one had it increment by one
					if(end == 0)
						files[numfiles].available++;
					// increment the number of files
					numfiles++;
				// so if it did exist and it's not a special extension file
				// increment how many servers have it
				// the for loop quit when it found it and it found it at i so
				// i is actually which file we found which makes this easier
				} else if(end == 0) {
					files[i].available++;
				}
				// now depending on the end we got add to the struct which end we got
				// important later
				// we can just edit files[i] since i is where we found this file
				// entry at
				switch(end) {
					case 1:
						files[i].one = 1;
						break;
					case 2:
						files[i].two = 1;
						break;
					case 3:
						files[i].three = 1;
						break;
					case 4:
						files[i].four = 1;
						break;
					default:
						break;
				}
			}
			
			//printf("%d: buf %s\n", numfiles, buf);
			//bzero(files[numfiles]->name, MAXBUF);
			//strcpy(files[numfiles].name, "Paul Johnson");
			//strcpy(files[numfiles]->name, buf);
			//files[numfiles].name = buf;
			//numfiles++;
		}
	}
	bzero(buf, MAXBUF);
	strcpy(buf, "lets gooo");
	if(servers.dfs4sock != -1) {
		while(strcmp(buf, "N M D") != 0) {
			bzero(buf, MAXBUF);
			read(servers.dfs4sock, buf, MAXBUF);
			//printf("read %s\n", buf);
			if(strcmp(buf, "N M D") != 0) {
				write(servers.dfs4sock, ack, strlen(ack));
				//break;
				end = 0;
				// ok, so here we figure out if it's a fourth of a file
				// through our amazing extension method
				// we also remove the extension for printing just the filename
				if(strcmp(buf+strlen(buf)-2, ".1") == 0) {
					bzero(tmpbuf, MAXBUF);
					strncpy(tmpbuf, buf, strlen(buf)-2);
					bzero(buf, MAXBUF);
					strcpy(buf, tmpbuf);
					end = 1;
				} else if(strcmp(buf+strlen(buf)-2, ".2") == 0) {
					bzero(tmpbuf, MAXBUF);
					strncpy(tmpbuf, buf, strlen(buf)-2);
					bzero(buf, MAXBUF);
					strcpy(buf, tmpbuf);
					end = 2;
				} else if(strcmp(buf+strlen(buf)-2, ".3") == 0) {
					bzero(tmpbuf, MAXBUF);
					strncpy(tmpbuf, buf, strlen(buf)-2);
					bzero(buf, MAXBUF);
					strcpy(buf, tmpbuf);
					end = 3;
				} else if(strcmp(buf+strlen(buf)-2, ".4") == 0) {
					bzero(tmpbuf, MAXBUF);
					strncpy(tmpbuf, buf, strlen(buf)-2);
					bzero(buf, MAXBUF);
					strcpy(buf, tmpbuf);
					end = 4;
				}
				// check to see if the file already exists
				exists = 0;
				for(i = 0; i < numfiles; i++) {
					if(strcmp(files[i].name, buf) == 0) {
						exists = 1;
						break;
					}
				}
				// if it doesn't exist add it to our list of files
				if(exists == 0) {
					// intialize everything that needs to be intialized
					strcpy(files[numfiles].name, buf);
					files[numfiles].one = 0;
					files[numfiles].two = 0;
					files[numfiles].three = 0;
					files[numfiles].four = 0;
					files[numfiles].available = 0;
					// so available actually keeps track of how many servers
					// have a directory, since this one had it increment by one
					if(end == 0)
						files[numfiles].available++;
					// increment the number of files
					numfiles++;
				// so if it did exist and it's not a special extension file
				// increment how many servers have it
				// the for loop quit when it found it and it found it at i so
				// i is actually which file we found which makes this easier
				} else if(end == 0) {
					files[i].available++;
				}
				// now depending on the end we got add to the struct which end we got
				// important later
				// we can just edit files[i] since i is where we found this file
				// entry at
				switch(end) {
					case 1:
						files[i].one = 1;
						break;
					case 2:
						files[i].two = 1;
						break;
					case 3:
						files[i].three = 1;
						break;
					case 4:
						files[i].four = 1;
						break;
					default:
						break;
				}
			}
			
			//printf("%d: buf %s\n", numfiles, buf);
			//bzero(files[numfiles]->name, MAXBUF);
			//strcpy(files[numfiles].name, "Paul Johnson");
			//strcpy(files[numfiles]->name, buf);
			//files[numfiles].name = buf;
			//numfiles++;
		}
	}
	//strcpy(testfile.name, "Paul Johnson");
	//printf("%s\n", testfile.name);
	// print out all the file names
	//printf("%ld\n", sizeof(files));
	for(i = 0; i < numfiles; i++) {
		files[i].available += files[i].one;
		files[i].available += files[i].two;
		files[i].available += files[i].three;
		files[i].available += files[i].four;
		printf("%d: %s (%d/4)\n", i, files[i].name, files[i].available);
	}
	
	

	//printf("end of list\n");
}

int listhash(char *name) {
	// I was going to use this to do an efficient list with hashing instead
	// of just go through the list, but the list is small enough and this
	// is a little overkill and I'm tired so not gonna do that
	unsigned char hashval[MD5_DIGEST_LENGTH];
	MD5_CTX mdContext;
	int value = 0;
	int i = 0;
	char namec[MAXBUF];

	bzero(namec, MAXBUF);
	strcpy(namec, name);

	MD5_Init(&mdContext);
	MD5_Update(&mdContext, namec, strlen(namec));
	MD5_Final(hashval, &mdContext);

	for(i = 0; i < MD5_DIGEST_LENGTH; i++)
		value += hashval[i];
	value = value % 50;
	return value;
}

// this function helps keep code down since put does the same thing just
// with different combinations
void puthelper(int serv1, int serv2, FILE *fp, int size, char filename[], int filen) {
	int fourth = 0;
	char buf[MAXBUF];
	char c = 0;
	int bufi = 0;
	char newfile[MAXBUF];
	char snum[4];
	char ack[MAXBUF];

	// create our file to write
	// have to append which segment of the file this is
	// since I store files as example.txt.1
	// or example.txt.3 depending on which section of the file it is
	bzero(newfile, MAXBUF);
	strcpy(newfile, filename);
	strcat(newfile, ".");
	//itoa(filen, snum, 10);
	snprintf(snum, sizeof(snum), "%d", filen);
	strcat(newfile, snum);
	//printf("%s\n", newfile);
	//printf("%d, %d\n", serv1, serv2);

	//printf("creating files\n");
	// create the files on the server
	if(serv1 != -1) {
		write(serv1, newfile, strlen(newfile));
		bzero(buf, MAXBUF);
		read(serv1, buf, MAXBUF);
		printf("%s\n", buf);
		if(strncmp(buf, "created", 7) != 0) {
			//printf("%s\n", buf);
			serv1 = -1;
		}
	}
	if(serv2 != -1) {
		write(serv2, newfile, strlen(newfile));
		bzero(buf, MAXBUF);
		read(serv2, buf, MAXBUF);
		printf("%s\n", buf);
		if(strncmp(buf, "created", 7) != 0) {
			//printf("%s\n", buf);
			serv2 = -1;
		}
	}

	
	//printf("writing files\n");
	// now time to write the file info to the server
	bzero(buf, MAXBUF);
	// size is actually the size we need to read for this section
	// ie 1/4 the total file size
	// fourth is how much of it we've transmitted
	// so while we haven't transmitted 1/4 of the data keep going
	while(fourth <= size-1) {
		// if we've filled up our buf than send it to the servers
		// and empty it so we can keep reading and sending data
		if(bufi >= MAXBUF) {
			//printf("sending %d\n", bufi);
			//printf("sending %s", buf);
			if(serv1 != -1) {
				write(serv1, buf, bufi);
				bzero(ack, MAXBUF);
				read(serv1, ack, MAXBUF);
				//printf("received %s\n", ack);
			}
			if(serv2 != -1) {
				write(serv2, buf, bufi);
				bzero(ack, MAXBUF);
				read(serv2, ack, MAXBUF);
				//printf("received %s\n", ack);
			}
			bufi = 0;
			bzero(buf, MAXBUF);
		}
		// get a char, increment our variables
		c = (char) fgetc(fp);
		buf[bufi] = c;
		bufi++;
		fourth++;
		//printf("size: %d, bufi: %d, fourth: %d\n", size, bufi, fourth);
	}
	
	// send the last bit of our buffer since the loop exits once we've read 1/4
	// the data and doesn't send buf. so we gotta send our last bit of the data
	//printf("sending %d\n", bufi);
	if(serv1 != -1) {
		write(serv1, buf, bufi);
		bzero(ack, MAXBUF);
		read(serv1, ack, MAXBUF);
		//printf("received final %s\n", ack);
	}
	if(serv2 != -1) {
		write(serv2, buf, bufi);
		bzero(ack, MAXBUF);
		read(serv2, ack, MAXBUF);
		//printf("received final %s\n", ack);
	}

	// that was the end of the file so send our EOF signal to servers
	//printf("Finished writing\n");
	bzero(buf, MAXBUF);
	strcpy(buf, "EOF\r\n");
	//printf("sending %s\n", buf);
	if(serv1 != -1) {
		write(serv1, buf, strlen(buf));
	}
	if(serv2 != -1) {
		write(serv2, buf, strlen(buf));
	}

	//printf("Reading success\n");
	// get the message servers sent back which should be that the file was
	// created and completed
	bzero(buf, MAXBUF);
	if(serv1 != -1) {
		read(serv1, buf, MAXBUF);
		printf("%s\n", buf);
	}
	bzero(buf, MAXBUF);
	if(serv2 != -1) {
		read(serv2, buf, MAXBUF);
		printf("%s\n", buf);
	}
	
	//printf("put helper done\n");
}

void put(struct Servers servers, char *filename) {
	//printf("%s\n", filename);
	// put files onto servers
	// split file into fourths and send each fourth
	FILE *fp;
	int size;
	int hashval = -1;
	char buf[MAXBUF];
	char delim[] = " \r\n";
	char *tokked;
	char fullfile[MAXBUF];
	int remainder;

	// open the file on our end for reading
	fp = fopen(filename, "rb");
	if(fp == NULL) {
		printf("Couldn't open file %s cancelling\n", filename);
		return;
	}

	// get the size of the file and split it into fourths for transmission
	// we actually need the remainder too so we don't miss whatever little
	// bit was left over in the file
	size = file_size(filename);
	//printf("size: %d\n", size);
	remainder = size % 4;
	//printf("remainder: %d\n", remainder);
	size = size / 4;
	//printf("newsize: %d\n", size);

	//printf("%d\n", size);
	// get the hashvalue of our file
	hashval = md5sumhash(filename);
	hashval = hashval % 4;
	//printf("%d hash\n", hashval);

	// prompt user for the subdirectory
	bzero(buf, MAXBUF);
	printf("Enter subdirectory including / at end or leave blank for root:\n");
	fgets(buf, MAXBUF, stdin);
	// need the / so check for that (I kept messing this up and crashing it lol)
	// so mostly did this bc I kept messing up input
	while(strlen(buf) != 1 && buf[strlen(buf)-2] != '/') {
		//printf("%s\n", buf);
		bzero(buf, MAXBUF);
		printf("Forgot / try again: ");
		fgets(buf, MAXBUF, stdin);
	}
	// if they didn't want a subdirectory could've left it blank so handle that too
	bzero(fullfile, MAXBUF);
	if(strlen(buf) != 1) {
		tokked = strtok(buf, delim);
		// also make sure they aren't trying to get into users like a bunch of hackers
		if(strncmp(tokked, "users/", 5) == 0) {
			printf("users protected directory cancelling\n");
			return;
		}
	} else {
		tokked = strtok(buf, delim);
	}
	if(tokked != NULL) {
		strcpy(fullfile, tokked);
		strcat(fullfile, filename);
	} else {
		strcat(fullfile, filename);
	}
	// ok, finished constructing our full filename cool
	printf("%d: Putting %s to %s\n", hashval, filename, fullfile);

	// send put command to servers
	bzero(buf, MAXBUF);
	strcpy(buf, "put");
	writeservers(servers, buf);
	
	// check and make sure each server got put command
	if(servers.dfs1sock != -1) {
		bzero(buf, MAXBUF);
		read(servers.dfs1sock, buf, MAXBUF);
		if(strcmp(buf, "ready") != 0)
			printf("Failed to send put to DFS1\n");
	} else {
		printf("Not connected to DFS1 file may be incomplete.\n");
	}
	if(servers.dfs2sock != -1) {
		bzero(buf, MAXBUF);
		read(servers.dfs2sock, buf, MAXBUF);
		if(strcmp(buf, "ready") != 0)
			printf("Failed to send put to DFS2\n");
	} else {
		printf("Not connected to DFS2 file may be incomplete.\n");
	}
	if(servers.dfs3sock != -1) {
		bzero(buf, MAXBUF);
		read(servers.dfs3sock, buf, MAXBUF);
		if(strcmp(buf, "ready") != 0)
			printf("Failed to send put to DFS3\n");
	} else {
		printf("Not connected to DFS3 file may be incomplete.\n");
	}
	if(servers.dfs4sock != -1) {
		bzero(buf, MAXBUF);
		read(servers.dfs4sock, buf, MAXBUF);
		if(strcmp(buf, "ready") != 0)
			printf("Failed to send put to DFS4\n");
	} else {
		printf("Not connected to DFS4 file may be incomplete.\n");
	}

	// now depending on the hash value transmit and differnt portion of the file
	// to each server
	switch(hashval) {
		case 0:
			// puthelper params:
			//server1 to transmit to, 
			// server 2 to transmit to,
			// file pointer: shared which means we can read easily from section to section
			// size to read
			// name of file 
			// which section of the file we are transmitting
			puthelper(servers.dfs4sock, servers.dfs1sock, fp, size, fullfile, 1);
			puthelper(servers.dfs1sock, servers.dfs2sock, fp, size, fullfile, 2);
			puthelper(servers.dfs2sock, servers.dfs3sock, fp, size, fullfile, 3);
			size = size+remainder;
			puthelper(servers.dfs3sock, servers.dfs4sock, fp, size, fullfile, 4);
			break;
		case 1:
			puthelper(servers.dfs1sock, servers.dfs2sock, fp, size, fullfile, 1);
			puthelper(servers.dfs2sock, servers.dfs3sock, fp, size, fullfile, 2);
			puthelper(servers.dfs3sock, servers.dfs4sock, fp, size, fullfile, 3);
			size = size+remainder;
			puthelper(servers.dfs4sock, servers.dfs1sock, fp, size, fullfile, 4);
			break;
		case 2:
			puthelper(servers.dfs2sock, servers.dfs3sock, fp, size, fullfile, 1);
			puthelper(servers.dfs3sock, servers.dfs4sock, fp, size, fullfile, 2);
			puthelper(servers.dfs4sock, servers.dfs1sock, fp, size, fullfile, 3);
			size = size+remainder;
			puthelper(servers.dfs1sock, servers.dfs2sock, fp, size, fullfile, 4);
			break;
		case 3:
			puthelper(servers.dfs3sock, servers.dfs4sock, fp, size, fullfile, 1);
			puthelper(servers.dfs4sock, servers.dfs1sock, fp, size, fullfile, 2);
			puthelper(servers.dfs1sock, servers.dfs2sock, fp, size, fullfile, 3);
			size = size+remainder;
			puthelper(servers.dfs2sock, servers.dfs3sock, fp, size, fullfile, 4);
			break;
		default:
			printf("Couldn't get hashvalue cancelling\n");
	}

	fclose(fp);
}

// this gets the md5 hash of a file
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
	// get that md5 hash
	MD5_Init(&mdContext);
	while((bytes = fread(data, 1, 1024, fp)) != 0)
		MD5_Update(&mdContext, data, bytes);
	MD5_Final(c, &mdContext);
	// turn it into an int bc that's what I want
	for(i = 0; i < MD5_DIGEST_LENGTH; i++)
		value += c[i];
	return value;
}

// this sends the exit command to each server to gracefully
// close the client, you can actually just ctrl c the client tho
// which is what I usually do so not very useful honestly
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

// this is the mkdir command which will make a subfolder on the servers
void makedir(struct Servers servers, char *dirname) {
	//printf("%s\n", dirname);
	// make a subdirectory in current directory
	char buf[MAXBUF];

	// pretty straighforword command actually
	// just, send command mkdir directory to servers, and read what they said bakc
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

// this is the login command for users
void login(struct Servers servers) {
	// login as a user for private drive
	// username and password
	char buf[MAXBUF];
	int sock = -1;
	char username[32];
	char password[32];

	// ok, so servers share dfs.conf file which is where
	// username and passwords are, which means we just need
	// to connect to one server for the login password part
	// so find first available server and start login process
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
	// we found someone to login too cool start sending stuff to them
	write(sock, buf, strlen(buf));

	// get the user they want to login too
	printf("Username: ");
	fgets(buf, MAXBUF, stdin);
	// strtok is destructive heheheeeee
	strtok(buf, "\n");
	// check to make sure they didn't do a bad
	// we are actually in the users folder so now they can make a user called
	// users, but that doesn't let them hack or anything since they'll be in
	// users/users/ and only users/ protected
	while(strlen(buf) > 32 || strlen(buf) < 3 || strchr(buf, ':') != NULL) {
		printf("Username too long or too short or contains :.\n Must be 3-32 characters no :. Try agagin\n");
		printf("Username: ");
		bzero(buf, strlen(buf));
		fgets(buf, MAXBUF, stdin);
		strtok(buf, "\n");
	}
	
	// write the username to the server
	strcpy(username, buf);
	write(sock, buf, strlen(buf));
	// server will send whether the username exists already or not
	bzero(buf, strlen(buf));
	read(sock, buf, MAXBUF);
	// if the user exists already
	if(strcmp(buf, "FUSR") == 0) {
		// you only get 3 guesses at the password
		int attempts = 3;
		while(attempts > 0) {
			printf("Enter password: ");
			bzero(buf, strlen(buf));
			fgets(buf, MAXBUF, stdin);
			strtok(buf, "\n");
			strcpy(password, buf);
			//printf("%s\n", buf);
			// write the password to the server and see if
			// server said it was good or not
			write(sock, buf, strlen(buf));
			bzero(buf, MAXBUF);
			read(sock, buf, MAXBUF);
			// oh, good job you got the password right
			if(strcmp(buf, "SUCCESS") == 0) {
				// there's actually a special login
				// bc we've confirmed the user exists and 
				// the password was right
				// so there's a special command that bypasses
				// the login sequence for simplicity
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
		// the user doesn't exist so start the process to make a user
		printf("User doesn't exist. Create user? (y/n): ");
		bzero(buf, strlen(buf));
		fgets(buf, MAXBUF, stdin);
		strtok(buf, "\n");
		if(strcmp(buf, "y") == 0) {
			printf("Enter password: ");
			bzero(buf, strlen(buf));
			fgets(buf, MAXBUF, stdin);
			strtok(buf, "\n");
			// make sure not bad password
			// the : exclusion is because that's how we seperate users:password
			// in dfs.conf so if they put that in the password it'd be a pain
			// to split user:password and come out wrong
			while(strlen(buf) > 32 || strlen(buf) < 3 || strchr(buf, ':') != NULL) {
				printf("Password must be 3-32 characters long and not contain :.\n Try again: ");
				bzero(buf, strlen(buf));
				fgets(buf, MAXBUF, stdin);
				strtok(buf, "\n");
			}
			// make are cool new user
			write(sock, buf, strlen(buf));
			strcpy(password, buf);
			bzero(buf, strlen(buf));
			read(sock, buf, MAXBUF);
			bzero(buf, strlen(buf));
			// do our special login 
			strcpy(buf, "logspc ");
			strcat(buf, username);
			strcat(buf, ":");
			strcat(buf, password);
			writeservers(servers, buf);
		} else {
			// they said they didn't wanna make a user
			// so send that to the server we connected to
			// and stop the command process
			bzero(buf, strlen(buf));
			strcpy(buf, "n");
			//printf("%s\n", buf);
			write(sock, buf, strlen(buf));
			bzero(buf, strlen(buf));
			read(sock, buf, MAXBUF);
		}
	}

}

// logout of the user we are logged in as
void logout(struct Servers servers) {
	// logout of the user so they can access shared drive again
	char buf[] = "logout";
	char ret[MAXBUF];

	// this is actually just a really straightforward command on client end
	// just send logout and see if successful or not
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
	//printf("write msg: %s\n", message);
	if(servers.dfs1sock != -1)
		write(servers.dfs1sock, message, strlen(message));
	if(servers.dfs2sock != -1)
		write(servers.dfs2sock, message, strlen(message));
	if(servers.dfs3sock != -1)
		write(servers.dfs3sock, message, strlen(message));
	if(servers.dfs4sock != -1)
		write(servers.dfs4sock, message, strlen(message));
}

// reads a message from each server
// I don't actually end up using this even though
// probably would've been a good idea for acks I don't 
// particularly care about
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

// this command allows the user to try to reconnect to the servers
// maybe they were put up and you wanna check
struct Servers reconnect(struct Servers servers) {
	// check if still connected to each server
	// for each server not connected to try to reconnect
	char buf[] = "Connected";

	// pretty straightforward really
	// if just calls the method connect for each server
	// and sends the preliminary we connected message to the server
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


// this is mostly just handling user input in a while loop
void clientlogic(struct Servers servers) {
	char buf[MAXBUF];
	char spacedelim[] = " \r\n"; // for splitting on space
	char *tokked; //  result of strtok
	int optval = 1;
	socklen_t optlen = sizeof(optval);

	// preliminary connected message that needs to be sent to the servers
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
		// exit command
		if(strncmp(buf, "exit", 4) == 0) {
			exitcmd(servers);
			break;
		// get command
		} else if(strncmp(buf, "get", 3) == 0) {
			// get the filename we are getting
			tokked = strtok(buf, spacedelim);
			tokked = strtok(NULL, spacedelim);
			if(tokked == NULL) {
				printf("Invalid filename after get\n");
			} else if(strncmp(tokked, "users/", 6) == 0) {
				printf("Users is protected directory.\n");
			} else {
				get(servers, tokked);
			}
		// put command
		} else if(strncmp(buf, "put", 3) == 0) {
			// get filename for the put command
			tokked = strtok(buf, spacedelim);
			tokked = strtok(NULL, spacedelim);
			if(tokked == NULL) {
				printf("Invalid filename after get\n");
			} else if(strncmp(tokked, "users/", 6) == 0) {
				printf("Users is protected directory.\n");
			} else {
				put(servers, tokked);
			}
		// list command cool
		} else if(strncmp(buf, "list", 4) == 0) {
			tokked = strtok(buf, spacedelim);
			tokked = strtok(NULL, spacedelim);
			if(tokked != NULL && strncmp(tokked, "users", 5) == 0) {
				printf("Users is protected directory.\n");
			} else {
				list(servers, tokked);
			}
		// these explanations suck but I mean, these
		// are just checking user input and calling the method
		// which is pretty straightforward and simple
		} else if(strncmp(buf, "mkdir", 5) == 0) {
			tokked = strtok(buf, spacedelim);
			tokked = strtok(NULL, spacedelim);
			if(tokked == NULL)
				printf("Invalid directory cancelling.\n");
			else if(strcmp(tokked, "users") == 0)
				printf("Protected directory can't make.\n");
			else
				makedir(servers, tokked);
		// this one literally just calls the method 
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

// this connects to each of the servers
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
		size = ftell(fp)-1;
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