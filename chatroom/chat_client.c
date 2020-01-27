#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "proto.h"
#include "string.h"

volatile sig_atomic_t flag = 0;
int sockfd = 0;
char *ip = "127.0.0.1";
char nickname[LENGTH_NAME] = {};
char roomId[LENGTH_ROOM] = {};
char firstData[LENGTH_FIRSTDATA] = {};

void sigint_handler(int sig){
    flag = 1;
}

void recv_msg_handler(){
    char receiveMessage[LENGTH_MSG];
    while (1)
    {
        int receive = recv(sockfd, receiveMessage, LENGTH_SEND, 0);
        if (receive > 0)
        {
            printf("\r%s             \n",receiveMessage);
            str_overwrite_stdout(nickname,roomId);
        }else if (receive == 0)
        {
            break;
        }else{
            //
        }
    }  
}

void send_msg_handler(){
    char message[LENGTH_MSG] = {};
    while (1)
    {
        str_overwrite_stdout(nickname,roomId);
        while (fgets(message, LENGTH_MSG, stdin) != NULL)
        {
            str_trim_lf(message, LENGTH_MSG);
            if (strlen(message) == 0)
            {
                str_overwrite_stdout(nickname,roomId);
            }else{
                break;
            }
        }
        send(sockfd, message, LENGTH_MSG,0);
        if (strcmp(message, "exit") == 0)
        {
            break;
        }        
    }
    sigint_handler(2);
}

int main(int argc, char const *argv[])
{
    signal(SIGINT, sigint_handler);

    if(argc == 2){
        ip = argv[1];
    }
    printSplash();
    printf("名前を入れてください：");
    if(fgets(nickname,LENGTH_NAME,stdin) != NULL){
        str_trim_lf(nickname, LENGTH_NAME);
    }

    if (strlen(nickname) < 2 || strlen(nickname) >= LENGTH_NAME - 1){
        printf("\n名前は2文字以上かつ30文字以内で入力してください\n");
        exit(EXIT_FAILURE);
    }

    printf("ルームIDを入れてください：");
    if(fgets(roomId,LENGTH_ROOM,stdin) != NULL){
        str_trim_lf(roomId, LENGTH_ROOM);
    }

    sprintf(firstData,"%s,%s",nickname,roomId);


    //ソケットを作成
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1){
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    //サーバーソケットの設定
    struct sockaddr_in serverAddr,clientAddr;
    int server_addrlen = sizeof(serverAddr);
    int client_addrlen = sizeof(clientAddr);
    memset(&serverAddr, 0, server_addrlen);
    memset(&clientAddr, 0, client_addrlen);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(8888);

    //サーバに接続
    int err = connect(sockfd, (struct sockaddr *)&serverAddr, server_addrlen);
    if (err == -1)
    {
        printf("サーバーへの接続に失敗しました．");
        exit(EXIT_FAILURE);
    }
    
    getsockname(sockfd, (struct sockaddr*)&clientAddr, (socklen_t*)&client_addrlen);
    getpeername(sockfd, (struct sockaddr*)&serverAddr, (socklen_t*)&server_addrlen);
    //printf("サーバーに接続しました:%s(%d)\n",inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));
    printf("サーバーに接続しました(%s)【ルーム：%s】\n", nickname, roomId);

    // send(sockfd, nickname, LENGTH_NAME, 0);
    send(sockfd, firstData, LENGTH_FIRSTDATA, 0);

    pthread_t send_msg_thread;
    if (pthread_create(&send_msg_thread, NULL, (void *)send_msg_handler, NULL) != 0)
    {
        printf("エラーが発生しました\n");
        exit(EXIT_FAILURE);
    }

    pthread_t recv_msg_thread;
    if (pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0)
    {
        printf("エラーが発生しました\n");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        if(flag){
            printf("\nByb\n");
            break;
        }
    }
    
    close(sockfd);
    return 0;
}
