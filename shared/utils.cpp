#include "./utils.h"

using namespace std;

/**
 * @brief Reads a message from a given socket descriptor and returns a char pointer containing the read bytes
 * 
 * @param sd 
 * @return char* 
 */
char *ReadMessageInChar(int sd)
{
    char *msg, aux[MAX_READ_SIZE + 1];
    int lgMsg = -1, lgRead, lgAux = -1;
    if (read(sd, &lgMsg, 4) <= 0)
        return nullptr;
    if (lgMsg <= 0)
        return nullptr;

    msg = new char[lgMsg + 1];
    lgRead = 0;
    while (lgRead < lgMsg)
    {
        if (lgMsg - lgRead < MAX_READ_SIZE)
            lgAux = read(sd, aux, lgMsg - lgRead);
        else
            lgAux = read(sd, aux, MAX_READ_SIZE);
        if (lgAux <= 0)
        {
            strcpy(msg, "Failed to read MessageInChar");
            return msg;
        }
        lgRead += lgAux;
        aux[lgAux] = 0;
        strcat(msg, aux);
    }
    msg[lgMsg] = 0;

    return msg;
}

string ReadMessageInString(int sd)
{
    string request = "";
    char aux[MAX_READ_SIZE + 1];
    int lgRequest = -1, lgRead, lgAux = -1;

    if (read(sd, &lgRequest, 4) <= 0)
        return "";
    if (lgRequest <= 0)
        return "";

    request.reserve(lgRequest + 1);
    request.clear();
    lgRead = 0;

    while (lgRead < lgRequest)
    {
        if (lgRequest - lgRead < MAX_READ_SIZE)
            lgAux = read(sd, aux, lgRequest - lgRead);
        else
            lgAux = read(sd, aux, MAX_READ_SIZE);
        if (lgAux <= 0)
        {
            return "";
        }
        lgRead += lgAux;
        aux[lgAux] = 0;
        request.append(aux);
    }
    request[lgRequest] = 0;

    return request;
}

string ReadChunkMessageInString(int sd, int &lgReadTotal, int fileSize)
{
    string msg = "";
    char aux[MAX_READ_SIZE + 1];
    int lgMsg;

    if (MAX_READ_SIZE < (fileSize - lgReadTotal))
        lgMsg = MAX_READ_SIZE;
    else
        lgMsg = fileSize - lgReadTotal;

    msg.clear();
    msg.reserve(lgMsg);
    if (read(sd, aux, lgMsg) < 0)
    {
        lgReadTotal = -1;
        return "";
    }
    aux[lgMsg] = 0;
    for (int i = 0; i < lgMsg; ++i)
        msg.push_back(aux[i]);
    msg[lgMsg] = 0;
    lgReadTotal += lgMsg;

    return msg;
}

bool WriteInt(int sd, int val)
{
    if (write(sd, &val, 4) < 0)
        return false;
    return true;
}

bool WriteMessage(int sd, const char *msg)
{
    //if (DescriptorIsValid(sd) == false)
    //return false;

    int lgMsg = strlen(msg), lgWrite, lgAux = -1;
    if (write(sd, &lgMsg, 4) <= 0)
        return false;

    lgWrite = 0;
    while (lgWrite < lgMsg)
    {
        if (lgMsg - lgWrite < MAX_WRITE_SIZE)
            lgAux = write(sd, msg + lgWrite, lgMsg - lgWrite);
        else
            lgAux = write(sd, msg + lgWrite, MAX_WRITE_SIZE);
        if (lgAux <= 0)
        {
            return false;
        }
        lgWrite += lgAux;
    }
    return true;
}

bool WriteFileInChunks(int peerDescriptor, ifstream &fd, int fileSize)
{
    char auxRead[MAX_READ_SIZE + 1];
    int lgRead, lgAuxRead;

    if (write(peerDescriptor, &fileSize, 4) < 0)
        return false;

    lgRead = 0;
    while (!fd.eof())
    {
        fd.read(auxRead, MAX_WRITE_SIZE);
        lgAuxRead = ((fd) ? MAX_WRITE_SIZE : fd.gcount());
        auxRead[lgAuxRead] = 0;

        if (write(peerDescriptor, auxRead, lgAuxRead) < 0)
            return false;
        lgRead += lgAuxRead;
    }
    return true;
}

bool DescriptorIsValid(int sd)
{
    errno = 0;
    return (fcntl(sd, F_GETFD) != -1 || errno != EBADF);
}