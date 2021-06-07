all: Server.c Clients.c
	gcc Server.c -o server -lpthread
	gcc Clients.c -o clients -lpthread

clean:
	rm server
	rm clients