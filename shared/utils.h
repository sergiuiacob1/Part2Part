#include <stdio.h>
#include <unistd.h>
#include <cstring>
#include <string>
#define MAX_READ_SIZE 1024
#define MAX_WRITE_SIZE 1024

char *ReadMessageInChar(int);
std::string ReadMessageInString (int);

bool WriteMessage (int, const char*);