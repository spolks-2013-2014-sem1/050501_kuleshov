#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <stdarg.h>

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

unsigned long long IpPortToNumber(unsigned long IPv4, unsigned short port)
{
    return (((unsigned long long) IPv4) << 16) + (unsigned long long) port;
}


// str is string like "fileName:fileSize\0"
char *getFileSizePTR(char *str, int size)
{
    for (int i = 0; i < size; i++) {
        if (str[i] == ':') {
            str[i] = 0;
            return &str[i + 1];
        }
    }
    return NULL;
}


void safe_print(pthread_mutex_t* mutex, const char* message, ...)
{	
    va_list args;
    va_start( args, message );

	pthread_mutex_lock(mutex);
    vprintf(message, args);
	pthread_mutex_unlock(mutex);

    va_end(args);	
}