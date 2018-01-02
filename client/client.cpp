#include "client.h"

using namespace std;
int PEER_PORT;

void ReadCommand(string &);
void ParseCommand(string &);
string GetAvailableFiles(int);
File GetFile();
void ProcessNewConnection(Client *, int);

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

bool Client::CreatePeerServer()
{
    bool connectedSuccessfully;
    if ((sdPeer = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("[peer server] Socket error\n");
        return false;
    }
    int on = 1;
    setsockopt(sdPeer, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    memset(&peerServer, 0, sizeof(peerServer));
    memset(&peerFrom, 0, sizeof(peerFrom));

    peerServer.sin_family = AF_INET;
    peerServer.sin_addr.s_addr = htonl(INADDR_ANY);

    PEER_PORT = 9909;
    connectedSuccessfully = false;
    for (int i = 0; i < 1000; ++i)
    {
        peerServer.sin_port = htons(PEER_PORT);
        if (bind(sdPeer, (struct sockaddr *)&peerServer, sizeof(struct sockaddr)) != -1)
        {
            connectedSuccessfully = true;
            break;
        }
        ++PEER_PORT;
    }

    if (!connectedSuccessfully)
    {
        perror("[peer server]Bind error\n");
        return false;
    }

    if (!SetAddressForPeerServer())
        return false;

    if (SendPeerInfoToServer() == false)
        return false;

    cout << "Created peer at address " << peerIp << " at port " << peerPort << '\n';

    CreatePeerListener();
    return true;
}

void Client::CreatePeerListener()
{
    thread threadPeerListener(ListenToPeers, this);
    threadPeerListener.detach();
}

void Client::ListenToPeers(Client *client)
{
    //create peer server part
    int sdPeer = client->GetSdPeer();
    struct sockaddr_in peerFrom = client->GetPeerFrom();
    if (listen(sdPeer, 2) == -1)
    {
        perror("[server] Listen error");
        return;
    }
    
    printf("[server]Peer started listening at port %d...\n", PEER_PORT);
    while (1)
    {
        int peerDescriptor;
        socklen_t length = sizeof(peerFrom);

        fflush(stdout);

        //blocant
        if ((peerDescriptor = accept(sdPeer, (struct sockaddr *)&peerFrom, &length)) < 0)
        {
            perror("[server] Accept error");
            continue;
        }

        ProcessNewConnection(client, peerDescriptor);
    }
}

void ProcessNewConnection(Client *client, int peerDescriptor)
{
    cout << "Somebody connected to me\n";
}

bool Client::SendPeerInfoToServer()
{
    if (WriteMessage(sd, "add peer") == false)
    {
        cout << "Could not add peer to server";
        return false;
    }

    if (WriteMessage(sd, peerIp.c_str()) == false)
    {
        cout << "Could not send local ip to server\n";
        return false;
    }

    if (WriteMessage(sd, peerPort.c_str()) == false)
    {
        cout << "Could not send peer port to server\n";
        return false;
    }
    return true;
}

bool Client::SetAddressForPeerServer()
{
    struct ifaddrs *ifAddrStruct = NULL;
    struct ifaddrs *ifa = NULL;
    void *tmpAddrPtr = NULL;
    char auxIp[INET_ADDRSTRLEN];

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_addr)
            continue;
        if (ifa->ifa_addr->sa_family == AF_INET)
        {
            tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            inet_ntop(AF_INET, tmpAddrPtr, auxIp, INET_ADDRSTRLEN);
            if (strstr(auxIp, "192.168.") != NULL)
            {
                peerIp.assign(auxIp);
                break;
            }
        }
    }
    if (ifAddrStruct != NULL)
        freeifaddrs(ifAddrStruct);

    peerPort = to_string(PEER_PORT);

    if (peerIp.size() > 0)
        return true;
    return false;
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
    {
        if (ShowAvailableFiles() == false)
            cout << "No available files at this time\n";
    }
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

bool Client::ShowAvailableFiles()
{
    string availableFiles;
    bool gotFiles = false;

    if (WriteMessage(sd, "show files") == false)
    {
        perror("Can't write message to server: ");
        return false;
    }

    cout << "Available files:\n";
    do
    {
        availableFiles = GetAvailableFiles(sd);
        if (availableFiles.size() > 0)
            gotFiles = true;
        cout << availableFiles;
    } while (availableFiles.size() > 0);
    cout << '\n';

    return gotFiles;
}

string GetAvailableFiles(int sd)
{
    string response = "";
    response = ReadMessageInString(sd);
    return response;
}

void Client::DownloadFile()
{
    string fileName, userName;
    //File fileDwnld;
    FileStatus result;

    if (ShowAvailableFiles() == false)
    {
        cout << "No files available for download\n";
        return;
    }
    cout << "Choose a file to download: ";
    cin >> fileName;

    //fileDwnld.SetFileName(aux);
    cout << "Choose the user who has that file: ";
    cin >> userName;

    if (userName == this->name)
    {
        cout << "You can't download files from yourself\n";
        return;
    }

    if (WriteMessage(sd, "download file") == false)
    {
        cout << "Can't download from server\n";
        return;
    }

    result = GetFileFromServer(fileName, userName);
    switch (result)
    {
    case FileStatus::NOT_EXIST:
        cout << "The file you want to download does not exist\n";
        break;
    case FileStatus::SUCCESS:
        cout << "Successfully downloaded " << fileName << " from " << userName << '\n';
        break;
    }
}

FileStatus Client::GetFileFromServer(string fileName, string userName)
{
    string ipAddress, userPort;

    if (WriteMessage(sd, fileName.c_str()) == false)
    {
        cout << "Could not send file name to server\n";
        return FileStatus::OTHER;
    }

    if (WriteMessage(sd, userName.c_str()) == false)
    {
        cout << "Could not send user name to server\n";
        return FileStatus::OTHER;
    }

    ipAddress = ReadMessageInString(sd);
    if (ipAddress.size() == 0)
    {
        cout << "Could not get owner's id address\n";
        return FileStatus::OTHER;
    }

    if (ipAddress == "NOT_EXIST")
    {
        return FileStatus::NOT_EXIST;
    }

    userPort = ReadMessageInString(sd);
    if (userPort.size() == 0)
    {
        cout << "Could not get owner's id address\n";
        return FileStatus::OTHER;
    }

    return DownloadFileFromClient(ipAddress, userPort, fileName);
}

FileStatus Client::DownloadFileFromClient(string ipAddress, string userPort, string fileName)
{
    int sdDownload;
    /* int fd = open("testzz.txt", O_RDWR | O_CREAT, 0666);
    close(fd); */

    sdDownload = ConnectToPeerClient(ipAddress, userPort);
    if (sdDownload == -1)
    {
        cout << "Can't connect to download peer\n";
        return FileStatus::OTHER;
    }

    close(sdDownload);
    return FileStatus::SUCCESS;
}

int Client::ConnectToPeerClient(string ipAddress, string userPort)
{
    struct sockaddr_in peerOwningFile;
    int sdDownload;

    peerOwningFile.sin_family = AF_INET;
    peerOwningFile.sin_addr.s_addr = inet_addr(ipAddress.c_str());
    peerOwningFile.sin_port = htons(atoi(userPort.c_str()));

    if ((sdDownload = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Eroare la socket().\n");
        return -1;
    }

    if (connect(sdDownload, (struct sockaddr *)&peerOwningFile, sizeof(struct sockaddr)) == -1)
    {
        perror("[client]Eroare la connect().\n");
        return -1;
    }

    return sdDownload;
}
