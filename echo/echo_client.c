#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define RECVBUFSIZE 32
#define DEFAULT_PORT 7

void errorHandler(char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char const *argv[])
{
    int sock;   //socket descriptor
    struct sockaddr_in echoServerAddr;
    unsigned short echoServerPort;
    char * serverIP;
    char * echoString;
    char echoBuffer[RECVBUFSIZE];
    unsigned int echoStringLen;
    int recvBytes, totalRecvBytes;

    if ((argc < 3) || (argc > 4))
    {
        fprintf(stderr,"Usage:%s <Server IP> <Echo Word> [<Server Port>]\n",argv[0]);
        exit(EXIT_FAILURE);
    }

    serverIP = argv[1];
    echoString = argv[2];

    if (argc == 4)
    {
        echoServerPort = atoi(argv[3]);     
    }else
    {
        echoServerPort = DEFAULT_PORT;
    }
    
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        errorHandler("socket()");
    }

    memset(&echoServerAddr, 0,sizeof(echoServerAddr));
    echoServerAddr.sin_family = AF_INET;
    echoServerAddr.sin_addr.s_addr = inet_addr(serverIP);
    echoServerAddr.sin_port = htons(echoServerPort);
    
    if (connect(sock, (struct sockaddr *)&echoServerAddr, sizeof(echoServerAddr)) < 0)
    {
        errorHandler("connect()");
    }

    echoStringLen = strlen(echoString);

    if (send(sock, echoString, echoStringLen, 0) != echoStringLen)
    {
        errorHandler("send()");
    }
    

    totalRecvBytes = 0;
    printf("Recived:");
    while (totalRecvBytes < echoStringLen)
    {
        if ((recvBytes = recv(sock, echoBuffer, RECVBUFSIZE-1,0)) <= 0)
        {
            errorHandler("recv()");
        }
        totalRecvBytes += recvBytes;
        echoBuffer[recvBytes] = '\0';
        printf("%s",echoBuffer);
    }
    printf("\n");
    close(sock);

    return 0;
}
