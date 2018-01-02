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
#include <string>
#include <thread>
#include <vector>
#include "./../shared/file.h"

using namespace std;

extern mutex filesBeingChanged;

class User
{
private:
  int usrDescriptor;
  string name, dwnldPort, dwnldAddress;
  vector<File> userFiles;

public:
  User(int _descriptor)
  {
    usrDescriptor = _descriptor;
    userFiles.clear();
  }
  string GetName() { return name; }
  string GetDwnldAddress() { return dwnldAddress; }
  string GetDwnldPort() { return dwnldPort; }
  int GetUsrDescriptor() { return usrDescriptor; }
  void AddUserFile(File);
  vector<File> GetFiles() { return userFiles; }

  void SetName(string _newName) { name = _newName; }
  void SetDwnldInfo(string _ip, string _port)
  {
    dwnldAddress = _ip;
    dwnldPort = _port;
  }
};

#endif