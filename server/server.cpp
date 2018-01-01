#include "./server.h"

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
    if (listen(sd, 2) == -1)
    {
        perror("[server]Eroare la listen().\n");
        return;
    }

    while (1)
    {
        int clDescriptor;
        socklen_t length = sizeof(from);

        printf("[server]Asteptam la portul %d...\n", PORT);
        fflush(stdout);

        //blocant
        if ((clDescriptor = accept(sd, (struct sockaddr *)&from, &length)) < 0)
        {
            perror("[server]Eroare la accept().\n");
            continue;
        }

        ProcessNewConnection(clDescriptor);
        printf("Server: %d\n", clients.back().clDescriptor);
    }
}

void Server::ProcessNewConnection(int clDescriptor)
{
    Client newClient(clDescriptor);
    clients.push_back(newClient);

    thread newThread(ListenToClient, &(clients.back()));
    newThread.detach();
}

void Server::ListenToClient(Client *client)
{
    
}

//void *Server::treat(void *arg)
//{
/*struct thData tdL;
    tdL = *((struct thData *)arg);

    printf("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
    fflush(stdout);
    pthread_detach(pthread_self());

    if (AddNewClient(tdL.cl) == false)
    {
        close(tdL.cl);
        return (NULL);
    }

    ListenToClient((struct thData *)arg);

    close(tdL.cl); //(intptr_t)arg
    return (NULL);
    */
//}