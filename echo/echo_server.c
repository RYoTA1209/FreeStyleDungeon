#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAXPENDING 5
#define RECVBUFSIZE 32

void errorHandler(char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

void HandleTcpClient(int clientSock){
    char echoBuffer[RECVBUFSIZE];
    int recvMsgSize;

    if ((recvMsgSize = recv(clientSock, echoBuffer, RECVBUFSIZE, 0)) < 0)
    {
        errorHandler("recv()");
    }
    echoBuffer[recvMsgSize] = '\0';
    while (recvMsgSize > 0)
    {
        if(send(clientSock,echoBuffer,recvMsgSize,0) != recvMsgSize){
            errorHandler("send()");            
        }
        if((recvMsgSize = recv(clientSock, echoBuffer, RECVBUFSIZE, 0)) < 0){
            errorHandler("recv()");
        }
    }
    close(clientSock);
}

int main(int argc, char const *argv[])
{
    int serverSock;
    int clientSock;
    struct sockaddr_in echoServerAddr;
    struct sockaddr_in echoClientAddr;
    unsigned short echoServerPort;
    unsigned int clientLen;

    if(argc != 2){
        fprintf(stderr,"Usage: %s <Server Port>\n",argv[0]);
        exit(EXIT_FAILURE);
    }

    echoServerPort = atoi(argv[1]);
    
    // create socket
    if ((serverSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        errorHandler("socket()");
    }
    
    //create adress struct of echo server
    memset(&echoServerAddr, 0, sizeof(echoServerAddr));
    echoServerAddr.sin_family = AF_INET;
    echoServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    echoServerAddr.sin_port = htons(echoServerPort);

    //bind socket
    if (bind(serverSock, (struct sockaddr *)&echoServerAddr, sizeof(echoServerAddr)) < 0)
    {
        errorHandler("bind()");
    }

    //listen port
    if (listen(serverSock, MAXPENDING) < 0)
    {
        errorHandler("listen()");   
    }

    //accept connectoin-request from client
    while (1)
    {
        clientLen = sizeof(echoClientAddr);
        clientSock = accept(serverSock,(struct sockaddr *)&echoClientAddr,&clientLen);
        if(clientSock < 0){
            errorHandler("accept()");
        }
        printf("Handling client %s \n",inet_ntoa(echoClientAddr.sin_addr));
        HandleTcpClient(clientSock);
    }
    
    
    return 0;
}
