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
#include <string>
#include <thread>
#include <iostream>
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

  void AddFile();
  void ProcessCommand(string);
  void ShowAvailableFiles();
  void DownloadFile();
  bool SendFileToServer(File);

public:
  bool ConnectToServer(char *, char *);
  void ListenToCommands();
};

#endif