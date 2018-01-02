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
#include <mutex>
#include <fstream>
#include "./user.h"
#include "./../shared/file.h"
#include "./../shared/utils.h"

#define PORT 1234

using namespace std;

extern int errno;

class Server
{
private:
  int sd;
  struct sockaddr_in server;
  struct sockaddr_in from;

  list<User> users;
  list<string> availableNames;

  void BuildAvailableNames();
  void ProcessNewConnection(int);
  static void ListenToUser(Server *, User *);
  void SendAvailableFiles(User *);
  void AddFileToServer(User *);
  void DownloadFileRequest();

public:
  ~Server() { close(sd); }
  bool Create();
  void Listen();
  int GetNrOfConnectedUsers() { return users.size(); }
  void ProcessRequest(User *, string);
  bool SendNameToUser(User *);
  void AddUserName(string name) { availableNames.push_back(name); }
};

#endif