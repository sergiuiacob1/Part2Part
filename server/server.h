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
#include <pthread.h>

#define MAX_CLIENTS 1000
#define PORT 1234

typedef struct thData
{
    int cl;       //descriptorul intors de accept
    int idThread; //id-ul thread-ului tinut in evidenta de acest program
} thData;

extern int errno;

class Server
{
  private:
    int sd;

    struct sockaddr_in server;
    struct sockaddr_in from;
    pthread_t threads[MAX_CLIENTS];

  public:
    bool Create();
    void Listen();
};

#endif