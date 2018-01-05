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
#include <net/if.h>
#include <netdb.h>
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
#define MAX_CLIENTS 100

using namespace std;

extern int errno;
extern mutex modifyUsersMutex;

class Server
{
private:
  int sd;

  string ipMode;
  list<User> users;
  list<string> availableNames;

  void BuildAvailableNames();
  void ProcessNewConnection(int);
  static void ListenToUser(Server *, User *);
  void SendAvailableFiles(User *);
  void AddFileToServer(User *);
  void DownloadFileRequest(User *);
  void SendDwnldInfoToUser(User *, string, string);
  User *FileExists(string, string);
  bool AddPeer(User *);

public:
  ~Server() { close(sd); }
  void SetIpMode(string _mode) { ipMode = _mode; }
  bool Create();
  void Listen();
  int GetNrOfConnectedUsers() { return users.size(); }
  bool ProcessRequest(User *, string);
  bool SendNameToUser(User *);
  void AddUserName(string name) { availableNames.push_back(name); }
  bool GetDwnldInfo(User *);
  void DisconnectUser(User *);
};

#endif