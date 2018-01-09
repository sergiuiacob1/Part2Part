#compile the client and server
all:
	g++ -std=c++14 -pthread server/*.cpp shared/*.cpp -o server.out
	g++ -std=c++14 -pthread client/*.cpp shared/*.cpp -o client.out
clean:
	rm -f *~server client