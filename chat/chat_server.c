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
#include "chat_server.h"
#include "proto.h"

#define MAXPENDING 5
#define LENGTH_NAME 31
#define LENGTH_MSG 101
#define LENGTH_SEND 201

int server_sock = 0, client_sock = 0;
PlayerList *root,*now;

void sigint_handler(){
    PlayerList *tmp;
    while (root != NULL)
    {
        printf("\nClose socketfd: %d\n",root->data);
        close(root->data);
        tmp = root;
        root = root->next;
        free(tmp);
    }
    printf("chatroom closed.\n");
    exit(EXIT_SUCCESS);
}

void send_to_all_clients(PlayerList *node,char buf[]){
    PlayerList *tmp = root->next;
    while (tmp != NULL)
    {
        if(node->data != tmp->data){
            printf("Send to sockfd %d:\"%s\" \n",tmp->data,buf);
            send(tmp->data, buf, LENGTH_SEND, 0);
        }
        tmp = tmp->next;
    }
    
}

void client_handler(void *p_client){
    int leave_flag = 0;
    char nickname[LENGTH_NAME] = {};
    char recv_buffer[LENGTH_MSG] = {};
    char send_buffer[LENGTH_SEND] = {};
    PlayerList *node = (PlayerList *)p_client;

    //ニックネーム(2文字以上)
    if(recv(node->data, nickname, LENGTH_NAME, 0) <= 0 || strlen(nickname) < 2 || strlen(nickname) >= LENGTH_NAME -1){
        //不正なニックネームのときチャットルームを退出
        printf("%s has illegal nickname.\n",node->ip);
        leave_flag = 1;
    }else{
        //正当なニックネームのとき
        strncpy(node->name,nickname,LENGTH_NAME);
        printf("%s(%s)(%d) join the chatroom．.\n",node->name,node->ip,node->data);
        sprintf(send_buffer,"%s がチャットルームに参加しました.",node->name);
        send_to_all_clients(node,send_buffer);
    }
    
    while (1)
    {
        if (leave_flag)
        {
            break;
        }
        int receive = recv(node->data, recv_buffer, LENGTH_MSG, 0);
        
        if (receive > 0)
        {
            if (strlen(recv_buffer) == 0)
            {
                continue;
            }
            sprintf(send_buffer,"%s:%s",node->name,recv_buffer);
        }else if (receive == 0 || strcmp(recv_buffer,"exit") == 0){
            printf("%s left the chatroom.\n",node->name);
            sprintf(send_buffer,"%sがチャットルームを退出しました.\n",node->name);
            leave_flag = 1;
        }else
        {
            printf("Fatal Error: -1\n");
            leave_flag = 1;
        }
        send_to_all_clients(node,send_buffer);     
    }

    //退出
    close(node->data);
    if(node == now){
        now = node->prev;
        now->next = NULL;
    }else{
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }
    free(node);
}

int main(int argc, char const *argv[])
{
    
    signal(SIGINT,sigint_handler);

    //ソケットを作成
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1)
    {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    //サーバソケットの設定
    struct sockaddr_in serverAddr,clientAddr;
    int server_addrlen = sizeof(serverAddr);
    int client_addrlen = sizeof(clientAddr);
    memset(&serverAddr, 0, server_addrlen);
    memset(&clientAddr, 0, client_addrlen);
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8888);

    //ソケットの登録
    if(bind(server_sock, (struct sockaddr *)&serverAddr, server_addrlen) < 0){
        perror("bind()");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    //待機
    if(listen(server_sock,MAXPENDING) < 0){
        perror("listen()");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    //サーバ情報の表示
    getsockname(server_sock,(struct sockaddr *)&serverAddr, (socklen_t*)&server_addrlen);
    printf("Get started Chat Server on %s:%d\n",inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));

    //PlayerListの初期化
    root = newNode(server_sock, inet_ntoa(serverAddr.sin_addr));
    now = root;

    while (1)
    {
        client_sock = accept(server_sock,(struct sockaddr*)&clientAddr, (socklen_t*)&client_addrlen);

        //クライアント情報の表示
        getpeername(client_sock, (struct sockaddr*)&clientAddr, (socklen_t*)&client_addrlen);
        printf("Client %s:%d come in.\n",inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        //プレイヤーリストに追加
        PlayerList *node = newNode(client_sock,inet_ntoa(clientAddr.sin_addr));
        node->prev = now;
        now->next = node;
        now = node;

        pthread_t id;
        if(pthread_create(&id, NULL, (void *)client_handler, (void *)node) != 0){
            perror("pthread_create()");
            exit(EXIT_FAILURE);
        }

    }
    

    return 0;
}
