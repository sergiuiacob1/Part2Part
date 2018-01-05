#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>
#include <string>
#include <iostream>
#include <fstream>
#define MAX_READ_SIZE 1024
#define MAX_WRITE_SIZE 1024

extern int errno;

using namespace std;

char *ReadMessageInChar(int);
string ReadMessageInString(int);
string ReadChunkMessageInString(int, int &, int);

bool WriteInt (int, int);
bool WriteMessage(int, const char *);
bool WriteFileInChunks (int, ifstream &, int);
bool DescriptorIsValid(int);