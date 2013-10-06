#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

FILE *CreateReceiveFile(char *fileName, char *folderName)
{
    char filePath[256];

    // Create folder for received files if not exist
    struct stat st = { 0 };
    if (stat(folderName, &st) == -1) {
        if (mkdir(folderName, 0777) == -1)
            return NULL;
    }

    strcpy(filePath, folderName);
    strcat(filePath, "//");
    strcat(filePath, fileName);

    return fopen(filePath, "wb");
}


long GetFileSize(FILE * file)
{
    long pos = ftell(file);
    fseek(file, 0L, SEEK_END);

    long fileSize = ftell(file);
    fseek(file, pos, SEEK_SET);
    return fileSize;
}
