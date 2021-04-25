all: dfc dfs

dfc: Client.c
	gcc Client.c -o dfc

dfs: Server.c
	gcc Server.c -o dfs -lpthread
