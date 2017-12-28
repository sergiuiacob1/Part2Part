#include "./server.h"

void raspunde(void *);
static void *treat (void *);

bool Server::Create()
{
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("[server]Eroare la socket().\n");
        return false;
    }
    int on = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    memset(&server, 0, sizeof(server));
    memset(&from, 0, sizeof(from));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);

    if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("[server]Eroare la bind().\n");
        return false;
    }

    return true;
}

void Server::Listen()
{
    int i = 0;
    if (listen(sd, 2) == -1)
    {
        perror("[server]Eroare la listen().\n");
        return;
    }

    while (1)
    {
        int client;
        thData *td;
        socklen_t length = sizeof(from);

        printf("[server]Asteptam la portul %d...\n", PORT);
        fflush(stdout);

        //client= malloc(sizeof(int));

        if ((client = accept(sd, (struct sockaddr *)&from, &length)) < 0)
        {
            perror("[server]Eroare la accept().\n");
            continue;
        }

        int idThread;
        int cl;

        td = (struct thData *)malloc(sizeof(struct thData));
        td->idThread = i++;
        td->cl = client;

        pthread_create(&threads[i], NULL, &treat, td);
    }
}

static void *treat(void *arg)
{
    struct thData tdL;
    tdL = *((struct thData *)arg);
    printf("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
    fflush(stdout);
    pthread_detach(pthread_self());
    raspunde((struct thData *)arg);
    /* am terminat cu acest client, inchidem conexiunea */
    close((intptr_t)arg);
    return (NULL);
}

void raspunde(void *arg)
{
    int nr, i = 0;
    struct thData tdL;
    tdL = *((struct thData *)arg);
    if (read(tdL.cl, &nr, sizeof(int)) <= 0)
    {
        printf("[Thread %d]\n", tdL.idThread);
        perror("Eroare la read() de la client.\n");
    }

    printf("[Thread %d]Mesajul a fost receptionat...%d\n", tdL.idThread, nr);

    /*pregatim mesajul de raspuns */
    nr++;
    printf("[Thread %d]Trimitem mesajul inapoi...%d\n", tdL.idThread, nr);

    /* returnam mesajul clientului */
    if (write(tdL.cl, &nr, sizeof(int)) <= 0)
    {
        printf("[Thread %d] ", tdL.idThread);
        perror("[Thread]Eroare la write() catre client.\n");
    }
    else
        printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);
}