#include "./file.h"

mutex filesBeingChanged;

FileStatus File::CreateFile(char *_filePath)
{
    if (access(_filePath, F_OK) == -1)
    {
        return FileStatus::NOT_EXIST;
    }

    if (stat(_filePath, &fileStat) == -1)
        return FileStatus::STAT_ERROR;

    fileName.assign(strrchr(_filePath, '/') + 1);
    filePath.assign (_filePath);
    fileSize = (int)fileStat.st_size;

    return FileStatus::SUCCESS;
}