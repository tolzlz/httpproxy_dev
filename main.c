#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <ev.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>


#include "logger.h"
#include "netutils.h"
#include "callback.h"

#define LISTEN_BACKLOG 128

int create_and_bind(char *port, int32_t backlog, struct addrinfo * info) {
    int sockfd = socket(info->ai_family,info->ai_socktype,info->ai_protocol);
    if (sockfd == -1)
    {
        logger_error("socket create error\n");
    }
    if (set_nonblocking(sockfd) < 0) {
        logger_error("set_nonblocking fail [%d]\n", errno);
        close(sockfd);
        return -1;
    }

    if (set_reuseaddr(sockfd) < 0) {
        logger_error("set_reuseaddr fail [%d]\n", errno);
        close(sockfd);
        return -1;
    }
    if (bind(sockfd, info->ai_addr,info->ai_addrlen) < 0) {
        logger_error("bind error [%d]\n", errno);
        close(sockfd);
        return -1;
    }

    if (listen(sockfd, backlog) < 0) {
        logger_error("listen error [%d]!\n", errno);
        close(sockfd);
        return -1;
    }
    return sockfd;

}

void start_server(char *port){
    int sockfd;
    struct addrinfo hints,*servinfo,*p;
    memset(&hints,0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;//被动的，用于bind，通常用于server socket
    int yes = 1;

    int flag;
    if ((flag = getaddrinfo(NULL,port,&hints,&servinfo)) != 0)
    {
        logger_error("getaddrinfo: %s \n",gai_strerror(flag));
    }
    for(p = servinfo; p != NULL; p = p->ai_next)
    {
        if((sockfd = socket(p->ai_family, p->ai_socktype,
                            p->ai_protocol)) == -1)
        {
            perror("server: socket");
            continue;
        }

        if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                      sizeof(int)) == -1)
        {
            perror("setsockopt");
            exit(1);
        }

        if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }
    freeaddrinfo(servinfo);
    if(listen(sockfd, 10) == -1)
    {
        perror("listen");
        exit(1);
    }

    logger_info("server: waiting for connections..\n");
    accept_cb(sockfd);


}

int main() {
    char  *port = "1090";
    printf("Hello, World!\n");
    start_server(port);
    return 0;
}