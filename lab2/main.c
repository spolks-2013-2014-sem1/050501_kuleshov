#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>

#define bufSize 100

int createTcpSocket(char *hostName, unsigned short port,
                    struct sockaddr_in *sin);
int createServerSocket(char *hostName, unsigned short port,
                       struct sockaddr_in *sin, int qlen);

int main(int argc, char *argv[])
{
    char hostName[] = "localhost";
    unsigned int port = 5734;
    struct sockaddr_in sin;
    int socketDescriptor;

    if ((socketDescriptor =
         createServerSocket(hostName, port, &sin, 1)) == -1) {
        printf("Creation socket error\n");
        return 1;
    }

    char buf[bufSize + 1];

    while (1) {
        struct sockaddr_in remote;
        socklen_t rlen = sizeof(remote);
        int remoteSocketDescriptor =
            accept(socketDescriptor, (struct sockaddr *) &remote, &rlen);

        while (1) {
            int numberOfBytesRead =
                recv(remoteSocketDescriptor, buf, bufSize, 0);
            if (numberOfBytesRead <= 0)
                break;
            else {
                printf("%.*s", numberOfBytesRead, buf);
                fflush(stdout);
                send(remoteSocketDescriptor, buf, numberOfBytesRead, 0);
            }
        }
        close(remoteSocketDescriptor);
    }

    return 0;
}


// Create socket and fill in sockaddr_in structure
int createTcpSocket(char *hostName, unsigned short port,
                    struct sockaddr_in *sin)
{
    memset(sin, 0, sizeof(*sin));
    sin->sin_family = AF_INET;

    struct hostent *hptr = gethostbyname(hostName);
    if (hptr != NULL)
        memcpy(&sin->sin_addr, hptr->h_addr, hptr->h_length);   // sin_addr contain ip which bind to socket
    else
        return -1;

    sin->sin_port = htons(port);        // convert to network byte order

    struct protoent *pptr = getprotobyname("TCP");
    if (pptr == NULL)
        return -1;

    // Create socket
    int socketDescriptor = socket(PF_INET, SOCK_STREAM, pptr->p_proto);
    if (socketDescriptor < 0) {
        perror("Create socket error");
        return -1;
    }

    return socketDescriptor;
}

// Create server socket
int createServerSocket(char *hostName, unsigned short port,
                       struct sockaddr_in *sin, int qlen)
{
    int socketDescriptor;

    // Create socket
    if ((socketDescriptor =
         createTcpSocket(hostName, port,
                         (struct sockaddr_in *) sin)) == -1)
        return -1;

    // Bind socket
    if (bind(socketDescriptor, (struct sockaddr *) sin, sizeof(*sin)) < 0) {
        perror("Bind socket error");
        return -1;
    }
    // Switch socket into passive mode 
    if (listen(socketDescriptor, qlen) == -1) {
        perror("Socket passive mode error");
        return -1;
    }

    return socketDescriptor;
}
