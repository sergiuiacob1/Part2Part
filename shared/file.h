#ifndef FILE_H
#define FILE_H
#include <string>

using namespace std;

class File
{
    string fileName;
    int fileSize;

  public:
    string GetFileName() { return fileName; }
    int GetFileSize() { return fileSize; }
};

#endif