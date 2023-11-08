all: server client
server:server.cc
	g++ server.cc -o server -lpthread
client: client.cc
	g++ client.cc -o client
clean:
	rm -f server client
