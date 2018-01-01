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
int port;

int main(int argc, char *argv[])
{
  int sd;
  struct sockaddr_in server;

  int nr = 0;
  char buf[10];

  if (argc != 3)
  {
    printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
    return -1;
  }

  port = atoi(argv[2]);

  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("Eroare la socket().\n");
    return errno;
  }

  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons(port);

  /* ne conectam la server */
  if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    perror("[client]Eroare la connect().\n");
    return errno;
  }

  /* citirea mesajului */
  printf("[client]Introduceti un numar: ");
  fflush(stdout);
  read(0, buf, sizeof(buf));
  nr = atoi(buf);

  printf("[client] Am citit %d\n", nr);

  /* trimiterea mesajului la server */
  if (write(sd, &nr, sizeof(int)) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    return errno;
  }

  /* citirea raspunsului dat de server 
     (apel blocant pina cind serverul raspunde) */
  if (read(sd, &nr, sizeof(int)) < 0)
  {
    perror("[client]Eroare la read() de la server.\n");
    return errno;
  }
  /* afisam mesajul primit */
  printf("[client]Mesajul primit este: %d\n", nr);

  /* inchidem conexiunea, am terminat */
  close(sd);
}
