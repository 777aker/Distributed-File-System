all: dfc dfs

dfc: Client.c
	gcc Client.c -o dfc -L/usr/lib -lssl -lcrypto

dfs: Server.c
	gcc Server.c -o dfs -lpthread
