#include "./user.h"

using namespace std;

void User::AddUserFile(File file)
{
    filesBeingChanged.lock();
    userFiles.push_back(file);
    filesBeingChanged.unlock();
}