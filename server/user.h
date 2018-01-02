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

class User
{
private:
  int usrDescriptor;
  string address, name;
  vector<File> userFiles;

public:
  User(int _descriptor)
  {
    usrDescriptor = _descriptor;
    userFiles.clear();
  }
  string GetAddress() { return address; }
  int GetUsrDescriptor() { return usrDescriptor; }
  void AddUserFile(File);
  vector<File> GetFiles() { return userFiles; }
};

#endif