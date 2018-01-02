#include "./utils.h"

using namespace std;

char *ReadMessageInChar(int sd)
{
    char *msg, aux[MAX_READ_SIZE];
    int lgMsg = -1, lgRead, lgAux = -1;
    if (read(sd, &lgMsg, 4) <= 0)
        return nullptr;
    if (lgMsg <= 0)
        return nullptr;

    msg = new char[lgMsg];
    lgRead = 0;
    while (lgRead < lgMsg)
    {
        lgAux = read(sd, aux, MAX_READ_SIZE);
        if (lgAux <= 0)
        {
            strcpy(msg, "Failed to read MessageInChar");
            return msg;
        }
        lgRead += lgAux;
        strcat(msg, aux);
    }
    msg[lgMsg] = 0;

    return msg;
}

string ReadMessageInString(int sd)
{
    string request = "";
    char aux[MAX_READ_SIZE];
    int lgRequest = -1, lgRead, lgAux = -1;

    if (read(sd, &lgRequest, 4) <= 0)
        return "";
    if (lgRequest <= 0)
        return "";

    request.reserve(lgRequest);
    request.clear();
    lgRead = 0;

    while (lgRead < lgRequest)
    {
        lgAux = read(sd, aux, MAX_READ_SIZE);
        if (lgAux <= 0)
        {
            return "";
        }
        lgRead += lgAux;
        request.append(aux);
    }
    request[lgRequest] = 0;

    return request;
}

bool WriteMessage(int sd, const char *msg)
{
    int lgMsg = strlen(msg);
    if (write(sd, &lgMsg, 4) == -1)
        return false;
    if (write(sd, msg, lgMsg) == -1)
        return false;
    return true;
}