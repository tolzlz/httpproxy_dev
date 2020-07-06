#include <unistd.h>
#include <asm/errno.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include "net.h"
#include "proxy.h"
#include "logger.h"
#include "http_message.h"
#include "list.h"

/**
 * 从给定的套接字读取HTTP头，然后返回一个http请求*。
 * @param sockfd
 * @return
 */
http_request *http_read_header(int sockfd)
{

    logger_info("read heading \n");
    http_request *req;
    http_request_init(&req);
    char *line;
    line = read_line(sockfd);
    http_parse_method(req,line);
    while (1)
    {
        line = read_line(sockfd);
        if (line[0] == '\r' && line[1] == '\n'){
            logger_info("recvHeadr \n");
            break;
        }
        http_parse_metadata(req,line);
        free(line);
    }
    return req;
}

/**
 * 读取
 * @param sockfd
 * @param conn
 * @return
 */
char *read_line(int sockfd)
{
    int buffer_size = 2;
    char *line = (char*)malloc(sizeof(char)*buffer_size+1);
    char c;
    int length = 0;
    int counter = 0;

    while(1)
    {
        length = recv(sockfd, &c, 1, 0);

        line[counter++] = c;

        if(c == '\n')
        {
            line[counter] = '\0';
            return line;
        }

        // 重新分配空间
        if(counter == buffer_size)
        {
            buffer_size *= 2;
            line = (char*)realloc(line, sizeof(char)*buffer_size);
        }


    }
    return NULL;

}

//连接
int http_connect(http_request *req)
{
    char *host = (char *)list_get_key(&req->metadata_head, "Host");
    logger_info("http connect host %s\n",host);
    char  *port = strstr(host,":");
    if(port == NULL)
    {
        // 将端口设置为默认值
        port = calloc(3, sizeof(char));
        strncat(port, "80", 2);

        logger_info( "Using default port\n");
    }
    else
    {
        // 从主机上删除端口号
        host = strtok(host, ":");

        // 跳过":"字符
        port++;

        logger_info("Using port: %s\n", port);
    }
    logger_info( "Connecting to HTTP server: %s\n", host);
    if(host == NULL)
    {
        logger_error("Could not find the Host property in the metadata\n");
        return -1;
    }
    struct addrinfo hints, *servinfo, *p;
    int sockfd, rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    logger_info(" server info host :[%s], port: [%s] \n",host,port);
    if((rv = getaddrinfo(host, port, &hints, &servinfo)) != 0)
    {
        logger_error("Failed to lookup hostname\n");
        return -1;
    }

    // 循环查看所有结果并连接到第一个
    for(p = servinfo; p != NULL; p = p->ai_next) {
        //创建socket
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        //连接
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        logger_info("Failed to connect to HTTP server\n");
        return -1;
    }

    return sockfd;
}