#include "client.h"

using namespace std;

void ReadCommand(string &);
void ParseCommand(string &);
char *GetAvailableFiles(int);

bool Client::ConnectToServer(char *address, char *port)
{
    int portVal = atoi(port);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(address);
    server.sin_port = htons(portVal);

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Eroare la socket().\n");
        return false;
    }

    if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("[client]Eroare la connect().\n");
        return false;
    }

    return true;
}

void Client::ListenToCommands()
{
    string command;
    while (1)
    {
        ReadCommand(command);
        cout << "Am citit: " << command << '\n';
        ParseCommand(command);
        ProcessCommand(command);
    }

    close(sd);
}

void Client::ProcessCommand(string command)
{
    if (command == "show files")
        ShowAvailableFiles();
}

void Client::ShowAvailableFiles()
{
    char *availableFiles;
    availableFiles = GetAvailableFiles(sd);
    if (availableFiles == nullptr)
    {
        cout << "No files available\n";
        return;
    }

    cout << "Available files:\n";
    cout << availableFiles << '\n';
    delete availableFiles; //clean up
}

char *GetAvailableFiles(int sd)
{
    char *response;

    if (WriteMessage(sd, "show files") == false)
    {
        perror("Can't write message to server: ");
        return nullptr;
    }

    response = ReadMessageInChar(sd);
    return response;
}

void ParseCommand(string &command)
{
    while (command.back() == ' ' || command.back() == '\t')
        command.pop_back();
}

void ReadCommand(string &command)
{
    getline(cin, command);
}