#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <syslog.h>
#include <time.h>
#include "callback.h"
#include "logger.h"
#include "netutils.h"
#include "proxy.h"
#include "net.h"
#include "http_message.h"

void accept_cb(int fd)
{
    while (1)
    {
        struct http_conn *conn = NULL;
        struct sockaddr_storage storage;
        socklen_t len = sizeof(struct sockaddr_storage);
        int clientfd;
        clientfd = accept(fd, (struct sockaddr *)&storage, &len);
        if (clientfd == -1) {
            if ((errno != EAGAIN) && (errno != EWOULDBLOCK)) {
                logger_error("accept error: [%d]\n", errno);
            }
            break;
        }
        //设置非阻塞
//        if (set_nonblocking(clientfd) < 0) {
//            logger_error("set_nonblocking: [%d]\n", errno);
//        }
        client_recv_cb(clientfd);

    }

}

void client_recv_cb(int clientfd) {

    http_request *req;
    //读取http信息
    req = http_read_header(clientfd);
    if(req == NULL)
    {
        logger_error( "Failed to parse the header\n");
        return;
    }
    int server_sockfd;
    //连接服务
    server_sockfd = http_connect(req);
    if(server_sockfd == -1)
    {
        logger_error("Failed to connect to host\n");
        //释放队列
        http_request_destroy(req);
        return;
    }
    logger_info( "Connected to host\n");
    http_request_send(server_sockfd,req);
    http_request_destroy(req);
    //检索相应头
    logger_info("Beginning to retrieve the response header\n");
    //循环接受
    char* line;
    int line_length;
    int is_bad_encoding = 0;
    int is_text_content = 0;
    while(1)
    {
        line = read_line(server_sockfd);
        line_length = strlen(line);
        send_to_client(clientfd,line,0,line_length);
        if (line[0] == '\r' && line[1] == '\n')
        {
            //相应头结尾
            logger_info( "Received the end of the HTTP response header\n");
            free(line);
            break;
        }
        else if(18 <= line_length)
        {
            line[18] = '\0'; // 销毁行中的数据，但需要检查传入的数据是否为文本格式。
            if (strcmp(line, "Content-Type: text") == 0)
                is_text_content = 1;
            else if (strcmp(line, "Content-Encoding: ") == 0)
                is_bad_encoding = 1;
        }

        free(line);

    }
    logger_info( "Beginning to retrieve content\n");
    ssize_t chunk_length;
    char * temp = getdata(server_sockfd, &chunk_length);
    if (is_text_content && !is_bad_encoding )
    {
        //send_to_client(clientfd, NULL, 0, strlen(NULL));
    }
    else{
        send_to_client(clientfd, temp, 0, chunk_length);
    }
    logger_info("recv data :%s \n",temp);
    free(temp);
    close(server_sockfd);


}

int http_request_send(int sockfd,http_request *req)
{
    logger_info( "Requesting: %s\n", req->search_path);
    char *request_buffer = http_build_request(req);
    //发送消息
    if(send(sockfd, request_buffer, strlen(request_buffer), 0) == -1)
    {
        free(request_buffer);
        perror("send");
        return 1;
    }
    free(request_buffer);

    logger_info("Sent HTTP header to web server\n");

    return 0;
}

int send_to_client(int client_sockfd, char data[], int packages_size, ssize_t length)
{
    if (packages_size <1)
    {
        //发送到客户端
        if (send(client_sockfd,data,length,0) == -1)
        {
            perror("Couldn't send data to the client.");
            return -1;
        }

    }
    else
    {
        int p;
        for(p = 0; p*packages_size + packages_size < length; p++){
            if(send(client_sockfd, (data + p*packages_size), packages_size, 0) == -1)
            {
                //发送为空
                perror("Couldn't send any or just some data to the client. (loop)\n");
                return -1;
            }
        }

        if (p*packages_size < length)
        {
            if(send(client_sockfd, (data + p*packages_size), length - p*packages_size, 0) == -1)
            {
                perror("Couldn't send any or just some data to the client.\n");
                return -1;
            }
        }
    }

    return 0;
}

//客户端获取流

char * getdata(int server_sockfd,ssize_t *length){
    if(length == NULL)
    {
        logger_info("The length pointer supplied to getData is NULL\n");
        return NULL;
    }

    if(server_sockfd == -1)
    {
        logger_info( "The socket given to http_read_chunk is invalid\n");
        return NULL;
    }

    char *buf = malloc(sizeof(char));
    memset(buf, '\0', sizeof(char));
    char c;
    int current_size = 1;

    time_t timeout = 5;
    time_t start = time(NULL);

    ssize_t total_bytes = 0;
    ssize_t num_bytes = 0;

    while(1)
    {
        // 检测超时
        if(time(NULL) - start > timeout)
        {
            logger_info( "Request timed out\n");
            break;
        }

        //接受字符串
        num_bytes = recv(server_sockfd, &c, 1, 0);

        if(num_bytes <= -1)
        {
            break;
        }
        else if(num_bytes == 0)
        {
            break;
        }

        //为新数据重新分配空间
        buf = realloc(buf, sizeof(char)*++current_size);
        buf[total_bytes] = c;

        total_bytes += num_bytes;
    }

    logger_info( "Received: %d\n", (int)total_bytes);

    *length = total_bytes;

    return buf;
}