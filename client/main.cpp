#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include "./client.h"

using namespace std;

extern int errno;

int main(int argc, char *argv[])
{
  Client client;
  string ipMode;

  if (argc != 3)
  {
    printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
    return -1;
  }

  do
  {
    cout << "Enter the ipMode you want to use: ipv4/ipv6: ";
    cin >> ipMode;
    cout << "\n";
  } while (ipMode != "ipv4" && ipMode != "ipv6");

  client.SetIpMode(ipMode);

  if (client.ConnectToServer(argv[1], argv[2]) == false)
  {
    printf("Couldn't connect to the server\n");
    return 0;
  }

  if (client.CreatePeerServer() == false)
  {
    printf("Could not create peer server\n");
    return 0;
  }

  sleep(1);
  client.ShowAvailableCommands();
  client.ListenToCommands();

  return 0;
}
