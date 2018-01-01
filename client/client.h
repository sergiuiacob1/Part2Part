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

using namespace std;

class Client
{
private:
  string address;

public:
  int clDescriptor;
  Client(int _descriptor) { clDescriptor = _descriptor; }
  string GetAddress() { return address; }
  int GetClDescriptor() { return clDescriptor; }
};

#endif