#ifndef SERVER_H
#define SERVER_H
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <arpa/inet.h>
#include <string>
#include <thread>
#include <vector>
#include <list>
#include "./../client/client.h"

#define PORT 1234

using namespace std;

extern int errno;

class Server
{
private:
  int sd;
  struct sockaddr_in server;
  struct sockaddr_in from;

  list<Client> clients;

  void ProcessNewConnection(int);
  void CreateClientThread();
  static void ListenToClient(Client *);

public:
  bool Create();
  void Listen();
  int GetNrOfConnectedClients() { return clients.size(); }
};

#endif