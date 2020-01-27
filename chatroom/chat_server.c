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
PlayerList *playerRoot, *playerNow;
RoomList *roomRoot, *roomNow;

void sigint_handler()
{
    PlayerList *tmp;
    while (playerRoot != NULL)
    {
        printf("\nClose socketfd: %d\n", playerRoot->data);
        close(playerRoot->data);
        tmp = playerRoot;
        playerRoot = playerRoot->next;
        free(tmp);
    }
    printf("chatroom closed.\n");
    exit(EXIT_SUCCESS);
}

void send_to_all_clients(PlayerList *node, char buf[])
{
    PlayerList *tmp = playerRoot->next;
    while (tmp != NULL)
    {
        if (node->data != tmp->data)
        {
            printf("Send to sockfd %d:\"%s\" \n", tmp->data, buf);
            send(tmp->data, buf, LENGTH_SEND, 0);
        }
        tmp = tmp->next;
    }
}

void send_to_room_clients(PlayerList *node, char buf[])
{
    PlayerList *tmp = playerRoot->next;
    while (tmp != NULL)
    {
        if (node->room->id == tmp->room->id && node->data != tmp->data)
        {
            printf("Send to sockfd %d:\"%s\" \n", tmp->data, buf);
            send(tmp->data, buf, LENGTH_SEND, 0);
        }
        tmp = tmp->next;
    }
}

void client_handler(void *p_client)
{
    int leave_flag = 0;
    char firstRecvData[LENGTH_FIRSTDATA] = {};
    char roomID[LENGTH_ROOM] = {};
    int i_roomId;
    char nickname[LENGTH_NAME] = {};
    char recv_buffer[LENGTH_MSG] = {};
    char send_buffer[LENGTH_SEND] = {};
    char *ptr;
    PlayerList *node = (PlayerList *)p_client;
    RoomList *room = (RoomList *)node->room;

    if (recv(node->data, firstRecvData, LENGTH_FIRSTDATA, 0) > 0)
    {
        printf("%s\n",firstRecvData);
        ptr = strtok(firstRecvData, ",");
        // nickname = ptr;
        strcpy(nickname,ptr);
        while (ptr != NULL)
        {
            ptr = strtok(NULL, ",");

            if (ptr != NULL)
            {
                // roomID = ptr;
                strcpy(roomID,ptr);
            }
        }

        //ニックネーム(2文字以上)
        if (strlen(nickname) < 2 || strlen(nickname) >= LENGTH_NAME - 1)
        {
            //不正なニックネームのときチャットルームを退出
            printf("%s has illegal nickname.\n", node->ip);
            leave_flag = 1;
        }
        else
        {
            //正当なニックネームのとき
            strncpy(node->name, nickname, LENGTH_NAME);
            // printf("%s(%s)(%d) join the chatroom．.\n", node->name, node->ip, node->data);
            // sprintf(send_buffer, "%s がチャットルームに参加しました.", node->name);
            // send_to_all_clients(node, send_buffer);
        }

        i_roomId = atoi(roomID);

        //ルーム(0~100)
        if (i_roomId > 100 || i_roomId < 0)
        {
            //不正なニックネームのときチャットルームを退出
            printf("%s has illegal roomId.\n", node->ip);
            leave_flag = 1;
        }
        else
        {
            //正当なニックネームのとき
            node->room->id = i_roomId;
            printf("%s(%s)(%d) join the chatroom (%d)．.\n", node->name, node->ip, node->data, node->room->id);
            sprintf(send_buffer, "%s がチャットルーム(%d)に参加しました.", node->name, node->room->id);
            // send_to_all_clients(node, send_buffer);
            send_to_room_clients(node,send_buffer);
        }
    }
    else
    {
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
            sprintf(send_buffer, "%s(%d):%s", node->name,node->room->id, recv_buffer);
        }
        else if (receive == 0 || strcmp(recv_buffer, "exit") == 0)
        {
            printf("%s left the chatroom.\n", node->name);
            sprintf(send_buffer, "%sがチャットルームを退出しました.\n", node->name);
            leave_flag = 1;
        }
        else
        {
            printf("Fatal Error: -1\n");
            leave_flag = 1;
        }
        send_to_room_clients(node, send_buffer);
    }

    //退出
    close(node->data);
    if (node == playerNow)
    {
        playerNow = node->prev;
        playerNow->next = NULL;
    }
    else
    {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }
    free(node);
}

int main(int argc, char const *argv[])
{

    signal(SIGINT, sigint_handler);

    //ソケットを作成
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1)
    {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    //サーバソケットの設定
    struct sockaddr_in serverAddr, clientAddr;
    int server_addrlen = sizeof(serverAddr);
    int client_addrlen = sizeof(clientAddr);
    memset(&serverAddr, 0, server_addrlen);
    memset(&clientAddr, 0, client_addrlen);
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8888);

    //ソケットの登録
    if (bind(server_sock, (struct sockaddr *)&serverAddr, server_addrlen) < 0)
    {
        perror("bind()");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    //待機
    if (listen(server_sock, MAXPENDING) < 0)
    {
        perror("listen()");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    //サーバ情報の表示
    getsockname(server_sock, (struct sockaddr *)&serverAddr, (socklen_t *)&server_addrlen);
    printf("Get started Chat Server on %s:%d\n", inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));

    //PlayerListの初期化
    playerRoot = newNode(server_sock, inet_ntoa(serverAddr.sin_addr));
    playerNow = playerRoot;

    //RoomListの初期化
    roomRoot = newRoomNode();
    roomNow = roomRoot;

    while (1)
    {
        client_sock = accept(server_sock, (struct sockaddr *)&clientAddr, (socklen_t *)&client_addrlen);

        //クライアント情報の表示
        getpeername(client_sock, (struct sockaddr *)&clientAddr, (socklen_t *)&client_addrlen);
        printf("Client %s:%d come in.\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        //RoomListに追加
        RoomList *roomNode = newRoomNode();
        roomNode->prev = roomNow;
        roomNow->next = roomNode;
        roomNow = roomNode;

        //プレイヤーリストに追加
        PlayerList *node = newNode(client_sock, inet_ntoa(clientAddr.sin_addr));
        node->prev = playerNow;
        node->room = roomNow;
        playerNow->next = node;
        playerNow = node;

        pthread_t id;
        if (pthread_create(&id, NULL, (void *)client_handler, (void *)node) != 0)
        {
            perror("pthread_create()");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}
