#ifndef FILE_H
#define FILE_H
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <mutex>
#include <iostream>
#define MAX_PATH_SIZE 1024
#define MAX_FILE_PATH_SIZE 256

using namespace std;

/* #ifndef FILES_BEING_CHANGED_MUTEX
#define FILES_BEING_CHANGED_MUTEX
  mutex filesBeingChanged;
#endif */

extern mutex filesBeingChanged;

enum class FileStatus
{
  SUCCESS,
  NOT_EXIST,
  STAT_ERROR,
  OTHER
};

class File
{
  string fileName;
  int fileSize;
  struct stat fileStat;

public:
  FileStatus CreateFile(char *);
  string GetFileName() { return fileName; }
  int GetFileSize() { return fileSize; }

  void SetFileName(string _name) { fileName = _name; }
  void SetFileSize(int _val) { fileSize = _val; }
};

#endif