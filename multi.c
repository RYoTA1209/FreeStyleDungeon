#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc,char *argv[])
{
    int sockfd,len,new_sockfd;
    char buf[BUFSIZ];
    struct sockaddr_in serv,clnt;
    socklen_t sin_siz;
    int port;

    //new add
    pid_t child_pid;

    if(argc != 3){
        printf("Usage: ./prog host port\n");
        exit(1);
    }

    if(sockfd=socket(PF_INET,SOCK_STREAM,0)<0){
        perror("socket file discriptor");
        exit(1);
    }
    printf("socket() called\n");

    serv.sin_family = PF_INET;
    port=strtol(argv[2],NULL,10);
    serv.sin_port=htons(port);
    inet_aton(argv[1],&serv.sin_addr);

    sin_siz =sizeof(struct sockaddr_in);

    if(bind(sockfd,(strugt sockaddr *)$serv,sizeof(serv))<0){
        perror("bind");
        exit(1);
    }
    printf("bind() called\n");

    if(listen(sockfd,SOMAXCONN)<0){
        perror("listen");
        exit(1);
    }
    printf("listen() called\n");

    while(1){

        if((new_sockfd = accept(sockfd,(struct sockaddr *)&clnt,&sin_siz))<0){
            perror("accept");
        }
        printf("connect from %s: %d\n",inet_ntoa(clnt.sin_addr),ntohs(clnt.sin_port));

        if((child_pid = fork())>0) /*parent*/
        {
            close(new_sockfd);
        }
        else if(child_pid == 0)/*child*/
        {
            //new process receives data
            printf("in child %d\n",getpid());
            close(sockfd);

            memset(buf,0,BUFSIZE);
            while(strncasecmp(buf,"exit\n",5)!=0){
                len=recv(new_sockfd,buf,BUFSIZ,0);
                buf[len]='\0';
                printf("<== %s\n",buf);
                len=send(new_sockfd,buf,len,0);
            }
            close(new_sockfd);
            exit(0);
        }
        else 
        {
            perror("fork");
            exit(1);
        }
    }
}