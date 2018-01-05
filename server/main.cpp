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
#include "./server.h"

using namespace std;

extern int errno;

int main()
{
    string ipMode;
    Server server;
    do
    {
        cout << "Enter the ipMode you want to use: ipv4/ipv6: ";
        cin >> ipMode;
        cout << "\n";
    } while (ipMode != "ipv4" && ipMode != "ipv6");

    server.SetIpMode(ipMode);
    if (!server.Create())
        return -1;

    server.Listen();
    return 0;
}
