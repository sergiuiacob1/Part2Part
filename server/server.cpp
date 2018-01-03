#include "./server.h"

void ParseRequest(string &);

mutex modifyUsersMutex;

bool Server::Create()
{
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("[server]Socket error");
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
        perror("[server]Bind error");
        return false;
    }

    BuildAvailableNames();

    return true;
}

void Server::BuildAvailableNames()
{
    string name;
    ifstream fin("./server/usernames.txt");
    while (1)
    {
        fin >> name;
        if (fin.eof())
            break;
        availableNames.push_back(name);
    }
}

void Server::Listen()
{
    if (listen(sd, MAX_CLIENTS) == -1)
    {
        perror("[server] Listen error");
        return;
    }
    printf("[server]Waiting at port %d...\n", PORT);
    while (1)
    {
        int usrDescriptor;
        socklen_t length = sizeof(from);

        fflush(stdout);

        //blocant
        if ((usrDescriptor = accept(sd, (struct sockaddr *)&from, &length)) < 0)
        {
            perror("[server] Accept error");
            continue;
        }

        ProcessNewConnection(usrDescriptor);
    }
}

void Server::ProcessNewConnection(int usrDescriptor)
{
    modifyUsersMutex.lock();
    User newUser(usrDescriptor);
    users.push_back(newUser);
    modifyUsersMutex.unlock();

    thread newThread(ListenToUser, this, &(users.back()));
    newThread.detach();
}

void Server::ListenToUser(Server *server, User *user)
{
    string request;
    char *aux;
    int lgRequest, lgRead;
    int sd = user->GetUsrDescriptor();

    if (server->SendNameToUser(user) == false)
        return;

    while (1)
    {
        request = ReadMessageInString(sd);
        if (request.size() == 0)
            break;

        printf("Received request %s from %s\n", request.data(), user->GetName().c_str());
        if (server->ProcessRequest(user, request) == false)
            break;
    }

    server->DisconnectUser(user);
}

void Server::DisconnectUser(User *user)
{
    modifyUsersMutex.lock();

    for (list<User>::iterator it = users.begin(); it != users.end(); it++)
        if (it->GetName() == user->GetName())
        {
            users.erase(it);
            break;
        }
    AddUserName(user->GetName());

    cout << "Client disconnected\n";
    modifyUsersMutex.unlock();
}

bool Server::SendNameToUser(User *user)
{
    string clientName = availableNames.back();
    availableNames.pop_back();

    if (!WriteMessage(user->GetUsrDescriptor(), clientName.c_str()))
    {
        availableNames.push_back(clientName);
        return false;
    }

    user->SetName(clientName);
    return true;
}

bool Server::ProcessRequest(User *user, string request)
{
    ParseRequest(request);
    if (request == "show files")
        SendAvailableFiles(user);

    if (request == "add file")
        AddFileToServer(user);

    if (request == "download file")
        DownloadFileRequest(user);

    if (request == "add peer")
    {
        if (AddPeer(user) == false)
            return false;
    }

    return true;
}

bool Server::AddPeer(User *user)
{
    if (GetDwnldInfo(user) == false)
    {
        printf("Could not get ip and port from peer\n");
        return false;
    }

    cout << "Peer " << user->GetName();
    cout << " connected from " << user->GetDwnldAddress() << " at port " << user->GetDwnldPort() << '\n';
    return true;
}

bool Server::GetDwnldInfo(User *user)
{
    string localIp, peerPort;
    localIp = ReadMessageInString(user->GetUsrDescriptor());
    if (localIp.size() == 0)
        return false;

    peerPort = ReadMessageInString(user->GetUsrDescriptor());
    if (peerPort.size() == 0)
        return false;

    user->SetDwnldInfo(localIp, peerPort);
    return true;
}

void Server::DownloadFileRequest(User *user)
{
    User *userOwningFile;
    int sd = user->GetUsrDescriptor();
    string fileName, userName;

    fileName = ReadMessageInString(sd);
    if (fileName.size() == 0)
    {
        cout << "Could not read file name from download request\n";
        return;
    }
    userName = ReadMessageInString(sd);
    if (userName.size() == 0)
    {
        cout << "Could not read user name from download request\n";
        return;
    }

    userOwningFile = FileExists(fileName, userName);

    if (userOwningFile != nullptr)
    {
        SendDwnldInfoToUser(user, userOwningFile->GetDwnldAddress(), userOwningFile->GetDwnldPort());
    }
    else
    {
        if (WriteMessage(sd, "NOT_EXIST") == false)
            cout << "Failed to send NON_EXIST response to client\n";
    }
}

void Server::SendDwnldInfoToUser(User *user, string address, string port)
{
    modifyUsersMutex.lock();
    if (WriteMessage(user->GetUsrDescriptor(), address.c_str()) == false)
    {
        printf("Could not send download address to user\n");
        return;
    }

    if (WriteMessage(user->GetUsrDescriptor(), port.c_str()) == false)
    {
        printf("Could not send download port to user\n");
        return;
    }

    modifyUsersMutex.unlock();
}

User *Server::FileExists(string fileName, string userName)
{
    modifyUsersMutex.lock();
    for (list<User>::iterator it = users.begin(); it != users.end(); it++)
    {
        if (it->GetName() == userName)
        {
            for (auto file : it->GetFiles())
            {
                if (file.GetFileName() == fileName)
                {
                    modifyUsersMutex.unlock();
                    return &(*it);
                }
            }
            modifyUsersMutex.unlock();
            return nullptr;
        }
    }

    modifyUsersMutex.unlock();
    return nullptr;
}

void Server::AddFileToServer(User *user)
{
    File newFile;
    string fileName, fileSize;

    fileName = ReadMessageInString(user->GetUsrDescriptor());
    if (fileName.size() == 0)
    {
        printf("Failed to get file name");
        return;
    }

    fileSize = ReadMessageInString(user->GetUsrDescriptor());
    if (fileSize.size() == 0)
    {
        printf("Failed to get file size");
        return;
    }

    newFile.SetFileName(fileName);
    newFile.SetFileSize(atoi(fileSize.c_str()));

    user->AddUserFile(newFile);
}

void Server::SendAvailableFiles(User *user)
{
    int sd = user->GetUsrDescriptor(), totalFilesSize = 0;
    char msg[MAX_PATH_SIZE + 256];
    vector<File> userFiles;

    filesBeingChanged.lock();

    for (auto it : users)
    {
        userFiles = it.GetFiles();
        for (auto file : userFiles)
        {
            strcpy(msg, "File: ");
            strcat(msg, file.GetFileName().c_str());
            if (WriteMessage(user->GetUsrDescriptor(), msg) == false)
            {
                printf("Couldn't send filename to client\n");
                filesBeingChanged.unlock();
                return;
            }

            strcpy(msg, " ---size (B)--- ");
            strcat(msg, to_string(file.GetFileSize()).c_str());
            if (WriteMessage(user->GetUsrDescriptor(), msg) == false)
            {
                printf("Couldn't send user name to client\n");
                filesBeingChanged.unlock();
                return;
            }

            strcpy(msg, " ---from user--- ");
            strcat(msg, it.GetName().c_str());
            strcat(msg, "\n");
            if (WriteMessage(user->GetUsrDescriptor(), msg) == false)
            {
                printf("Couldn't send user name to client\n");
                filesBeingChanged.unlock();
                return;
            }
        }
    }
    filesBeingChanged.unlock();

    WriteMessage(user->GetUsrDescriptor(), ""); //EOF
}

void ParseRequest(string &request)
{
    while (request.back() == ' ' || request.back() == '\t')
        request.pop_back();
}