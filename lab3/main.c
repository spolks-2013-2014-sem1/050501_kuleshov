#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <libgen.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "../spolks_lib/sockets.c"


int ReceiveToBuf(int descriptor, char *buf, size_t len);

FILE *CreateReceiveFile(char *fileName);

void receiveFile(char *hostName, unsigned int port);
void sendFile(char *serverName, unsigned int port, char *filePath);

int clientSocketDescriptor = -1;
int serverSocketDescriptor = -1;

void intHandler(int signo)
{
    if (serverSocketDescriptor != -1)
        close(serverSocketDescriptor);
    else
        shutdown(clientSocketDescriptor, SHUT_WR);
    exit(0);
}


int main(int argc, char *argv[])
{
    if (argc < 3 || argc > 4) {
        fprintf(stderr, "Wrong arguments number\n");
        return 1;
    }
    
    // Change SIGINT action
    struct sigaction intSignal;
    intSignal.sa_handler = intHandler;
    sigaction(SIGINT, &intSignal, NULL);

    if (argc == 3)
        receiveFile(argv[1], atoi(argv[2]));
    else
        sendFile(argv[1], atoi(argv[2]), argv[3]);

    return 0;
}


void receiveFile(char *hostName, unsigned int port)
{
    struct sockaddr_in sin;

    if ((serverSocketDescriptor =
         createServerSocket(hostName, port, &sin, 5)) == -1) {
        printf("Creation socket error\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in remote;
    socklen_t rlen = sizeof(remote);

    char replyBuf[256], buf[4096];

    while (1) {
        printf("\nAwaiting connections...\n");
        int remoteSocketDescriptor =
            accept(serverSocketDescriptor, (struct sockaddr *) &remote,
                   &rlen);

        // Receive file name and file size
        if (ReceiveToBuf
            (remoteSocketDescriptor, (char *) &replyBuf,
             sizeof(replyBuf)) <= 0) {
            exit(EXIT_FAILURE);
        }
        char *fileName = strtok(replyBuf, ":");
        long fileSize = atoi(strtok(NULL, ":"));

        printf("File size: %ld, file name: %s\n", fileSize, fileName);

        FILE *file = CreateReceiveFile(fileName);
        if (file == NULL) {
            perror("Create file error");
            exit(EXIT_FAILURE);
        }
        
        // Receiving file    
        long bytesWritten = 0;
        while (bytesWritten < fileSize) {
            size_t recvSize =
                recv(remoteSocketDescriptor, buf, sizeof(buf), 0);
            if (recvSize > 0)
                bytesWritten += fwrite(buf, 1, recvSize, file);
            else if (recvSize <= 0) {
                perror("receive error");
                break;
            }
        }
        fclose(file);
        printf("%ld bytes received.\n", bytesWritten);
        printf("Receiving file completed.\n");
        close(remoteSocketDescriptor);
    }
}


void sendFile(char *serverName, unsigned int serverPort, char *filePath)
{
    struct sockaddr_in serverAddress;

    if ((clientSocketDescriptor =
         createTcpSocket(serverName, serverPort, &serverAddress)) == -1) {
        fprintf(stderr, "Creation socket error\n");
        exit(EXIT_FAILURE);
    }

    if (connect
        (clientSocketDescriptor, (struct sockaddr *) &serverAddress,
         sizeof(serverAddress)) < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    FILE *file = fopen(filePath, "r+");
    if (file == NULL) {
        perror("Open file error");
        exit(EXIT_FAILURE);
    }
    // Get file size
    fseek(file, 0L, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0L, SEEK_SET);

    char replyBuf[256];
    sprintf(replyBuf, "%s:%ld", basename(filePath), fileSize);

    // Send file name and file size
    if (send(clientSocketDescriptor, replyBuf, sizeof(replyBuf), 0) == -1) {
        perror("Send error");
        exit(EXIT_FAILURE);
    }

    char buf[4096];
    long totallyReadBytes = 0;
    size_t bytesRead = 0;

    // Sending file
    while (totallyReadBytes < fileSize) {
        bytesRead = fread(buf, 1, sizeof(buf), file);
        totallyReadBytes += bytesRead;
        int sendBytes = send(clientSocketDescriptor, buf, bytesRead, 0);
        if (sendBytes < 0) {
            perror("Sending error");
            exit(EXIT_FAILURE);
        }
    }
    printf("Sending file completed\n");

    shutdown(clientSocketDescriptor, SHUT_RDWR);
    fclose(file);
}


// Receive data from socket to buffer
int ReceiveToBuf(int descriptor, char *buf, size_t len)
{
    size_t recvSize = 0;
    while (recvSize < len) {
        size_t numberOfBytesRead =
            recv(descriptor, buf + recvSize, len - recvSize, 0);
        if (numberOfBytesRead == 0)
            break;
        else if (numberOfBytesRead < 0)
            return -1;
        else
            recvSize += numberOfBytesRead;
    }
    return recvSize;
}

FILE *CreateReceiveFile(char *fileName)
{
    char filePath[256];

    // Create folder for received files if not exist
    struct stat st = { 0 };
    if (stat("Received_files", &st) == -1) {
        if (mkdir("Received_files", 0777) == -1)
            return NULL;
    }

    strcpy(filePath, "Received_files//");
    strcat(filePath, fileName);

    return fopen(filePath, "wb");
}
