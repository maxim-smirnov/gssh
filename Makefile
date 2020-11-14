all: client server

client: client.c client.h common.h config.h
	gcc -o client -pthread client.c

server: server.c server.h common.h config.h
	gcc -o server -pthread -lutil server.c

clean:
	rm -rf pty_server pty_client
