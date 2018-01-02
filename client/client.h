#ifndef CLIENT_H
#define CLIENT_H
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
#include <sys/stat.h>
#include <ifaddrs.h>
#include <string>
#include <thread>
#include <iostream>
#include <set>
#include <list>
#include "./../shared/file.h"
#include "./../shared/utils.h"
#define MAX_COMMAND_SIZE 1024

using namespace std;

extern int errno;

class Client
{
private:
  int sd;
  struct sockaddr_in server;

  int sdPeer;
  struct sockaddr_in peerServer, peerFrom;
  string peerIp, peerPort;

  list <File> downloadableFiles;
  set<string> addedFiles;
  string name;

  void AddFile();
  void ProcessCommand(string);
  bool ShowAvailableFiles();
  void DownloadFile();
  bool SendFileToServer(File);
  bool GetClientNameFromServer();
  FileStatus GetFileFromServer(string, string);
  FileStatus DownloadFileFromClient(string, string, string);
  bool SetAddressForPeerServer();
  bool SendPeerInfoToServer();
  int ConnectToPeerClient(string, string);
  void CreatePeerListener();
  static void ListenToPeers(Client *);

public:
  bool ConnectToServer(char *, char *);
  bool CreatePeerServer();
  void ListenToCommands();
  int GetSdPeer() { return sdPeer; }
  struct sockaddr_in GetPeerFrom() { return peerFrom; }
};

#endif