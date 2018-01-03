#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>
#include <string>
#include <iostream>
#define MAX_READ_SIZE 1024
#define MAX_WRITE_SIZE 1024

extern int errno;

char *ReadMessageInChar(int);
std::string ReadMessageInString(int);
std::string ReadChunkMessageInString(int, int &, int);

bool WriteMessage(int, const char *);
bool WriteFileInChunks (int, int, int);
bool DescriptorIsValid(int);