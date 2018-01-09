#include "client.h"

using namespace std;
int PEER_PORT;

void ReadCommand(string &);
void ParseCommand(string &);
string GetAvailableFiles(int);
File GetFile();
void ProcessNewConnection(Client *, int);

/**
 * @brief Connects to the server at the given address and port
 * 
 * @param address 
 * @param port 
 * @return true 
 * @return false 
 */
bool Client::ConnectToServer(char *address, char *port)
{
    struct sockaddr_in6 server6;
    struct sockaddr_in server4;
    int portVal = atoi(port);

    if (ipMode == "ipv6")
    {
        server6.sin6_family = AF_INET6;
        if (inet_pton(AF_INET6, address, &server6.sin6_addr) <= 0)
        {
            perror("inet_pton() error");
            return false;
        }
        server6.sin6_port = htons(portVal);

        if ((sd = socket(AF_INET6, SOCK_STREAM, 0)) == -1)
        {
            perror("[client]Socket error");
            return false;
        }

        if (connect(sd, (struct sockaddr *)&server6, sizeof(server6)) == -1)
        {
            perror("[client]Connect error");
            return false;
        }
    }

    if (ipMode == "ipv4")
    {
        server4.sin_family = AF_INET;
        server4.sin_addr.s_addr = inet_addr(address);
        server4.sin_port = htons(portVal);

        if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            perror("[client]Socket error");
            return false;
        }

        if (connect(sd, (struct sockaddr *)&server4, sizeof(struct sockaddr)) == -1)
        {
            perror("[client]Connect error");
            return false;
        }
    }

    if (GetClientNameFromServer() == false)
    {
        cout << "Could not acquire username from server\n\n";
        return false;
    }
    else
    {
        cout << "Connected with username: " << name << "\n";
    }

    return true;
}

/**
 * @brief Creates the server-part from this peer
 * 
 * @return true 
 * @return false 
 */
bool Client::CreatePeerServer()
{
    bool connectedSuccessfully;
    struct sockaddr_in6 serv_addr;
    struct sockaddr_in peerServer;

    bzero(&serv_addr, sizeof(serv_addr));
    bzero(&peerServer, sizeof(peerServer));

    if (ipMode == "ipv6")
    {
        sdPeer = socket(AF_INET6, SOCK_STREAM, 0);
        if (sdPeer < 0)
        {
            perror("ERROR opening socket");
            return false;
        }
        int on = 1;
        setsockopt(sdPeer, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));

        serv_addr.sin6_flowinfo = 0;
        serv_addr.sin6_family = AF_INET6;
        serv_addr.sin6_addr = in6addr_any;

        PEER_PORT = 9909;
        connectedSuccessfully = false;
        for (int i = 0; i < 1000; ++i)
        {
            serv_addr.sin6_port = htons(PEER_PORT);
            if (bind(sdPeer, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != -1)
            {
                connectedSuccessfully = true;
                break;
            }
            ++PEER_PORT;
        }
    }

    if (ipMode == "ipv4")
    {
        if ((sdPeer = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            perror("[peer server] Socket error\n");
            return false;
        }
        int on = 1;
        setsockopt(sdPeer, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

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
    }

    if (!connectedSuccessfully)
    {
        perror("[peer server]Bind error");
        return false;
    }

    cout << "conn success\n";

    if (!SetAddressForPeerServer())
        return false;
    cout << "set address success\n";

    if (SendPeerInfoToServer() == false)
        return false;

    cout << "Created peer at address " << peerIp << " at port " << peerPort << "\n";

    CreatePeerListener();

    return true;
}

/**
 * @brief Creates a thread which listens to other peers
 * 
 */
void Client::CreatePeerListener()
{
    thread threadPeerListener(ListenToPeers, this);
    threadPeerListener.detach();
}

/**
 * @brief Listens to other peers who want to connect to this peer
 * 
 * @param client 
 */
void Client::ListenToPeers(Client *client)
{
    //create peer server part
    int sdPeer = client->GetSdPeer();
    struct sockaddr_in peerFrom4;
    struct sockaddr_in6 peerFrom6;
    socklen_t length;

    if (client->GetIpMode() == "ipv4")
        length = sizeof(peerFrom4);
    else
        length = sizeof(peerFrom6);

    bzero(&peerFrom4, sizeof(peerFrom4));
    bzero(&peerFrom6, sizeof(peerFrom6));

    if (listen(sdPeer, MAX_PEERS) == -1)
    {
        perror("[server] Listen error");
        return;
    }

    printf("[server]Peer started listening at port %d...\n\n", PEER_PORT);

    while (1)
    {
        int peerDescriptor;
        fflush(stdout);

        //blocant

        if (client->GetIpMode() == "ipv4")
        {
            if ((peerDescriptor = accept(sdPeer, (struct sockaddr *)&peerFrom4, &length)) < 0)
            {
                perror("[server] Accept error");
                continue;
            }
        }
        else
        {
            if ((peerDescriptor = accept(sdPeer, (struct sockaddr *)&peerFrom6, &length)) < 0)
            {
                perror("[server] Accept error");
                continue;
            }
        }

        thread threadPeerConnected(ListenToConnectedPeer, client, peerDescriptor);
        threadPeerConnected.detach();
    }
}

/**
 * @brief This function is called when a certain peer connects to me
 * 
 * @param client 
 * @param peerDescriptor 
 */
void Client::ListenToConnectedPeer(Client *client, int peerDescriptor)
{
    string request;
    while (1)
    {
        request = ReadMessageInString(peerDescriptor);
        if (request.size() == 0)
        {
            cout << "Peer disconnected from me\n\n";
            break;
        }

        cout << "Peer received " << request << " request\n";

        if (client->ProcessRequest(request, peerDescriptor) == false)
            break;
    }
}

/**
 * @brief Called when this peer receives a request
 * 
 * @param request 
 * @param peerDescriptor 
 * @return true 
 * @return false 
 */
bool Client::ProcessRequest(string request, int peerDescriptor)
{
    if (request == "download file")
    {
        if (SendFileToPeer(peerDescriptor) == false)
            return false;
    }

    return true;
}

/**
 * @brief Sends a file to a peer who wants to download from me
 * 
 * @param peerDescriptor 
 * @return true 
 * @return false 
 */
bool Client::SendFileToPeer(int peerDescriptor)
{
    ifstream fd;
    struct stat st;
    int fileSize;
    string fileRequested;
    char filePath[MAX_FILE_PATH_SIZE];

    fileRequested = ReadMessageInString(peerDescriptor);
    getcwd(filePath, MAX_PATH_SIZE);
    strcat(filePath, "/");
    strcat(filePath, fileRequested.c_str());

    if (IHaveFile(fileRequested, filePath))
    {
        if (WriteMessage(peerDescriptor, "ok, you may download from me, cutie pie") == false)
        {
            cout << "Could not send response to peer requesting download\n";
            return false;
        }

        fd.open(filePath, ifstream::binary);

        if (!fd)
        {
            fd.close();
            cout << "Can't open file " << filePath << "\n\n";
            return false;
        }

        //send file size
        stat(filePath, &st);
        fileSize = st.st_size;

        if (WriteFileInChunks(peerDescriptor, fd, fileSize) == false)
        {
            fd.close();
            cout << "Couldn't send file to peer requesting download\n";
            return false;
        }

        cout << "File sent to peer requesting download\n";

        fd.close();
    }
    else
    {
        if (WriteMessage(peerDescriptor, "I do not have that file") == false)
        {
            cout << "Could not send response to peer requesting download\n";
            return false;
        }
    }

    return true;
}

/**
 * @brief Checks if this user has a certain file
 * 
 * @param fileName 
 * @param filePath 
 * @return true 
 * @return false 
 */
bool Client::IHaveFile(string fileName, char *filePath)
{
    for (auto it : downloadableFiles)
    {
        if (it.GetFileName() == fileName)
        {
            if (access(filePath, F_OK) != -1)
                return true;
        }
    }
    return false;
}

/**
 * @brief Sends to the server the ip address and the port to which this peer listens to
 * 
 * @return true 
 * @return false 
 */
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

/**
 * @brief Assings a ip address to this peer
 * 
 * @return true 
 * @return false 
 */
bool Client::SetAddressForPeerServer()
{
    struct ifaddrs *ifAddrStruct = NULL;
    struct ifaddrs *ifa = NULL;
    void *tmpAddrPtr = NULL;
    char auxIp[INET6_ADDRSTRLEN];

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_addr)
            continue;
        if (ifa->ifa_addr->sa_family == AF_INET && ipMode == "ipv4") //ipv4
        {

            tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            inet_ntop(AF_INET, tmpAddrPtr, auxIp, INET_ADDRSTRLEN);
            peerIp.assign(auxIp);
            if (count(peerIp.begin(), peerIp.end(), '.') == 3)
                break;
        }
        if (ifa->ifa_addr->sa_family == AF_INET6 && ipMode == "ipv6") //ipv6
        {
            tmpAddrPtr = &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
            inet_ntop(AF_INET6, tmpAddrPtr, auxIp, INET6_ADDRSTRLEN);
            peerIp.assign(auxIp);
            if (count(peerIp.begin(), peerIp.end(), ':') == 7)
                break;
        }
    }
    if (ifAddrStruct != NULL)
        freeifaddrs(ifAddrStruct);

    peerPort = to_string(PEER_PORT);

    if (peerIp.size() > 0)
        return true;
    return false;
}

/**
 * @brief Receives a username from the server with which this client is identified
 * 
 * @return true 
 * @return false 
 */
bool Client::GetClientNameFromServer()
{
    name = ReadMessageInString(sd);
    if (name.size() == 0)
        return false;
    return true;
}

/**
 * @brief Listen to commands from the console
 * 
 */
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

/**
 * @brief Shows the available commands the user can make
 * 
 */
void Client::ShowAvailableCommands()
{
    cout << "Available commands:\n";
    cout << "\t"
         << "\033[1;31madd file\033[0m\n";
    cout << "\t"
         << "\033[1;31mshow files\033[0m\n";
    cout << "\t"
         << "\033[1;31mdownload file\033[0m\n";
    cout << "\t"
         << "\033[1;31mexit\033[0m\n";
}

/**
 * @brief Remove white characters from the back of a given string
 * 
 * @param command 
 */
void ParseCommand(string &command)
{
    while (command.back() == ' ' || command.back() == '\t')
        command.pop_back();
}

/**
 * @brief Reads a string
 * 
 * @param command 
 */
void ReadCommand(string &command)
{
    getline(cin, command);
}

/**
 * @brief Checks what kind of command the user asked for
 * 
 * @param command 
 */
void Client::ProcessCommand(string command)
{
    if (command == "show files")
    {
        if (ShowAvailableFiles() == false)
            cout << "No available files at this time\n\n";
    }
    else if (command == "add file")
        AddFile();
    else if (command == "download file")
        DownloadFile();
    else if (command == "exit")
    {
        close(sd);
        close(sdPeer);
        exit(0);
    }

    cout << "Command is not recognized\n";
}

/**
 * @brief Adds a new file available for download
 * 
 */
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

    if (addedFiles.count(newFile.GetFilePath()) > 0)
    {
        cout << "You cannot add the same file multiple times!\n";
        return;
    }

    SendFileToServer(newFile);
}

/**
 * @brief Sends a new file to the server and marks it as downladable
 * 
 * @param file 
 * @return true 
 * @return false 
 */
bool Client::SendFileToServer(File file)
{
    if (WriteMessage(sd, "add file") == false)
        return false;
    if (WriteMessage(sd, file.GetFileName().c_str()) == false)
        return false;
    if (WriteMessage(sd, to_string(file.GetFileSize()).c_str()) == false)
        return false;

    cout << "Successfully added file\n\n";
    addedFiles.insert(file.GetFilePath());
    downloadableFiles.push_back(file);

    return true;
}

/**
 * @brief Shows the available files that can be downloaded
 * 
 * @return true 
 * @return false 
 */
bool Client::ShowAvailableFiles()
{
    string availableFiles;
    bool gotFiles = false;

    if (WriteMessage(sd, "show files") == false)
    {
        perror("Can't write message to server: ");
        return false;
    }

    cout << "Available files for download:\n";
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
/**
 * @brief Reads the available files for download from the server
 * 
 * @param sd 
 * @return string 
 */
string GetAvailableFiles(int sd)
{
    string response = "";
    response = ReadMessageInString(sd);
    return response;
}

/**
 * @brief Downloads a file
 * 
 */
void Client::DownloadFile()
{
    string fileName, userName;
    //File fileDwnld;
    FileStatus result;

    if (ShowAvailableFiles() == false)
    {
        cout << "No available files at this time\n\n";
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
        cout << "The file you want to download does not exist\n\n";
        break;
    case FileStatus::SUCCESS:
        cout << "Successfully downloaded " << fileName << " from " << userName << "\n\n";
        break;
    case FileStatus::BAD_FILE_SIZE:
        cout << "Received bad file size\n\n";
        break;
    }
}

/**
 * @brief Gets from the server the ip and port from which a file can be downloaded
 * 
 * @param fileName 
 * @param userName 
 * @return FileStatus 
 */
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

/**
 * @brief Downloads a file from the peer which has that file
 * 
 * @param ipAddress 
 * @param userPort 
 * @param fileName 
 * @return FileStatus 
 */
FileStatus Client::DownloadFileFromClient(string ipAddress, string userPort, string fileName)
{
    string response;
    int sdDownload;
    FileStatus downloadResult;

    sdDownload = ConnectToPeerClient(ipAddress, userPort);
    if (sdDownload == -1)
    {
        cout << "Can't connect to download peer\n";
        return FileStatus::OTHER;
    }

    if (WriteMessage(sdDownload, "download file") == false)
    {
        cout << "Could not sent request to peer\n";
        return FileStatus::OTHER;
    }

    if (WriteMessage(sdDownload, fileName.c_str()) == false)
    {
        cout << "Could not send file name to peer\n";
        return FileStatus::OTHER;
    }

    response = ReadMessageInString(sdDownload);
    cout << "Received response from peer: " << response << '\n';
    if (response.size() == 0)
    {
        cout << "Peer disconnected\n";
        FileStatus::OTHER;
    }

    downloadResult = FileStatus::OTHER;
    if (response == "ok, you may download from me, cutie pie")
    {
        downloadResult = SaveFile(fileName, sdDownload);
    }

    close(sdDownload);
    return downloadResult;
}

/**
 * @brief Downloads and saves the file locally
 * 
 * @param fileName 
 * @param sdDownload 
 * @return FileStatus 
 */
FileStatus Client::SaveFile(string fileName, int sdDownload)
{
    string downloadedFileName, fileSize, auxStr;
    int fd, lgRead, fileSizeInt, lastProcent, oldLgRead;

    downloadedFileName = "part2part-" + fileName;
    fd = open(downloadedFileName.c_str(), O_RDWR | O_CREAT, 0666);

    cout << "Started downloading " << downloadedFileName << "\n\n";

    if (read(sdDownload, &fileSizeInt, 4) < 0)
        return FileStatus::DWNLD_ERROR;

    oldLgRead = lgRead = 0;
    lastProcent = -1;
    while (lgRead < fileSizeInt)
    {
        oldLgRead = lgRead;
        auxStr = ReadChunkMessageInString(sdDownload, lgRead, fileSizeInt);
        if (lgRead == -1)
        {
            close(fd);
            cout << "Download failed\n\n";
            return FileStatus::DWNLD_ERROR;
        }

        for (int i = 0; i < lgRead - oldLgRead; ++i)
            write(fd, &auxStr[i], 1);
        if ((int)(lgRead / 1.0f / fileSizeInt * 100) != lastProcent)
        {
            cout << "Downloaded " << (int)(lgRead / 1.0f / fileSizeInt * 100) << "%\n";
            lastProcent = (int)(lgRead / 1.0f / fileSizeInt * 100);
        }
    }

    close(fd);
    return FileStatus::SUCCESS;
}

/**
 * @brief Connect to another peer
 * 
 * @param ipAddress 
 * @param userPort 
 * @return int 
 */
int Client::ConnectToPeerClient(string ipAddress, string userPort)
{
    struct sockaddr_in peerOwningFile4;
    struct sockaddr_in6 peerOwningFile6;
    int sdDownload;

    bzero(&peerOwningFile4, sizeof(peerOwningFile4));
    bzero(&peerOwningFile6, sizeof(peerOwningFile6));

    if (ipMode == "ipv6")
    {
        peerOwningFile6.sin6_family = AF_INET6;
        if (inet_pton(AF_INET6, ipAddress.c_str(), &peerOwningFile6.sin6_addr) < 0)
        {
            perror("inet_pton() error");
            return false;
        }
        peerOwningFile6.sin6_port = htons(atoi(userPort.c_str()));

        if ((sdDownload = socket(AF_INET6, SOCK_STREAM, 0)) == -1)
        {
            perror("Socket error");
            return -1;
        }

        if (connect(sdDownload, (struct sockaddr *)&peerOwningFile6, sizeof(peerOwningFile6)) == -1)
        {
            perror("[client]Connect error");
            return -1;
        }
    }

    if (ipMode == "ipv4")
    {
        peerOwningFile4.sin_family = AF_INET;
        peerOwningFile4.sin_addr.s_addr = inet_addr(ipAddress.c_str());
        peerOwningFile4.sin_port = htons(atoi(userPort.c_str()));

        if ((sdDownload = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            perror("Socket error");
            return -1;
        }

        if (connect(sdDownload, (struct sockaddr *)&peerOwningFile4, sizeof(peerOwningFile4)) == -1)
        {
            perror("[client]Connect error");
            return -1;
        }
    }

    return sdDownload;
}