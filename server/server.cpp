#include "./server.h"

void ParseRequest(string &);

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
        int usrDescriptor;
        socklen_t length = sizeof(from);

        printf("[server]Asteptam la portul %d...\n", PORT);
        fflush(stdout);

        //blocant
        if ((usrDescriptor = accept(sd, (struct sockaddr *)&from, &length)) < 0)
        {
            perror("[server]Eroare la accept().\n");
            continue;
        }

        ProcessNewConnection(usrDescriptor);
    }
}

void Server::ProcessNewConnection(int usrDescriptor)
{
    User newUser(usrDescriptor);
    users.push_back(newUser);

    thread newThread(ListenToUser, this, &(users.back()));
    newThread.detach();
}

void Server::ListenToUser(Server *server, User *user)
{
    string request;
    char *aux;
    int lgRequest, lgRead;
    int sd = user->GetUsrDescriptor();

    while (1)
    {
        request = ReadMessageInString(sd);

        printf("Received request %s\n", request.data());
        server->ProcessRequest(user, request);
    }
}

void Server::ProcessRequest(User *user, string request)
{
    ParseRequest(request);
    if (request == "show files")
        SendAvailableFiles(user);
}

void Server::SendAvailableFiles(User *user)
{
    //SendMessage (user->GetUsrDescriptor(), )
}

void ParseRequest(string &request)
{
    while (request.back() == ' ' || request.back() == '\t')
        request.pop_back();
}