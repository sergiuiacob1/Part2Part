#include "client.h"

using namespace std;

void ReadCommand(string &);
void ParseCommand(string &);
string GetAvailableFiles(int);
File GetFile();

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

    if (GetClientNameFromServer() == false)
    {
        cout << "Could not acquire username from server\n";
        return false;
    }
    else
    {
        cout << "Connected with username: " << name << '\n';
    }

    return true;
}

bool Client::GetClientNameFromServer()
{
    name = ReadMessageInString(sd);
    if (name.size() == 0)
        return false;
    return true;
}

void Client::ListenToCommands()
{
    string command;
    while (1)
    {
        ReadCommand(command);
        ParseCommand(command);
        ProcessCommand(command);
    }

    close(sd);
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

void Client::ProcessCommand(string command)
{
    if (command == "show files")
        ShowAvailableFiles();
    if (command == "add file")
        AddFile();
    if (command == "download file")
        DownloadFile();
}

void Client::AddFile()
{
    File newFile;
    char fullPath[MAX_PATH_SIZE], relFilePath[MAX_FILE_PATH_SIZE];

    cout << "Relative file path: ";
    cin >> relFilePath;
    getcwd(fullPath, MAX_PATH_SIZE);
    strcat(fullPath, "/");
    strcat(fullPath, relFilePath);

    if (newFile.CreateFile(fullPath) != FileStatus::SUCCESS)
    {
        cout << "Could not add file\n";
        return;
    }

    if (addedFiles.count(newFile.GetFileName()) > 0)
    {
        cout << "You cannot add multiple files with the same name!\n";
        return;
    }

    SendFileToServer(newFile);
}

bool Client::SendFileToServer(File file)
{
    if (WriteMessage(sd, "add file") == false)
        return false;
    if (WriteMessage(sd, file.GetFileName().c_str()) == false)
        return false;
    if (WriteMessage(sd, to_string(file.GetFileSize()).c_str()) == false)
        return false;

    cout << "Successfully added file\n";
    addedFiles.insert(file.GetFileName());

    return true;
}

void Client::ShowAvailableFiles()
{
    string availableFiles;

    if (WriteMessage(sd, "show files") == false)
    {
        perror("Can't write message to server: ");
        return;
    }

    cout << "Available files:\n";
    do
    {
        availableFiles = GetAvailableFiles(sd);
        cout << availableFiles;
    } while (availableFiles.size() > 0);
    cout << '\n';
}

string GetAvailableFiles(int sd)
{
    string response = "";
    response = ReadMessageInString(sd);
    return response;
}

void Client::DownloadFile()
{
    string aux;
    File fileDwnld;
    FileStatus result;

    ShowAvailableFiles();
    cout << "Choose a file to download: ";
    cin >> aux;

    fileDwnld.SetFileName(aux);

    if (WriteMessage(sd, "download file") == false)
    {
        cout << "Can't download from server\n";
        return;
    }

    result = DownloadFileFromServer(fileDwnld);
    switch (result)
    {
    case FileStatus::NOT_EXIST:
        cout << "The file you want to download does not exist\n";
    }
}

FileStatus Client::DownloadFileFromServer(File file)
{
}