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
#include <fstream>
#include <set>
#include <list>
#include "./../shared/file.h"
#include "./../shared/utils.h"
#define MAX_COMMAND_SIZE 1024
#define MAX_PEERS 100

using namespace std;

extern int errno;

class Client
{
private:
  int sd;

  int sdPeer;
  string peerIp, peerPort, ipMode;

  list<File> downloadableFiles;
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
  FileStatus SaveFile(string, int);
  bool SendFileToPeer(int);
  bool IHaveFile(string, char *);

public:
  bool ConnectToServer(char *, char *);
  bool CreatePeerServer();
  void ShowAvailableCommands();
  void ListenToCommands();
  int GetSdPeer() { return sdPeer; }
  static void ListenToConnectedPeer(Client *, int);
  bool ProcessRequest(string, int);
  void SetIpMode(string _mode) { ipMode = _mode; }
  string GetIpMode() { return ipMode; }
};

#endif