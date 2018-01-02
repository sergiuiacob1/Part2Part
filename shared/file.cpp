#include "./file.h"

mutex filesBeingChanged;

FileStatus File::CreateFile(char *filePath)
{
    if (access(filePath, F_OK) == -1)
    {
        return FileStatus::NOT_EXIST;
    }

    if (stat(filePath, &fileStat) == -1)
        return FileStatus::STAT_ERROR;

    fileName.assign(strrchr(filePath, '/') + 1);
    fileSize = (int)fileStat.st_size;

    return FileStatus::SUCCESS;
}