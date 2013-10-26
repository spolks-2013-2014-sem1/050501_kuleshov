#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

FILE *CreateReceiveFile(char *fileName, const char *folderName)
{
    char filePath[4096];

    // Create folder for received files if not exist
    struct stat st = { 0 };
    if (stat(folderName, &st) == -1) {
        if (mkdir(folderName, 0777) == -1)
            return NULL;
    }

    strcpy(filePath, folderName);
    strcat(filePath, "//");
    strcat(filePath, fileName);

    if (access(filePath, F_OK) != -1) {
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);

        char tmp[30];
        sprintf(tmp, "_%d-%d-%d_%d-%d-%d", tm.tm_year + 1900,
                tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min,
                tm.tm_sec);
        strcat(filePath, tmp);
    }

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
