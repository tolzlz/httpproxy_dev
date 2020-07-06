
#include <stdlib.h>
#include <string.h>
#include "http_message.h"
#include "logger.h"

int http_methods_len = 9;
const char *http_methods[] =
        {
                "OPTIONS",
                "GET",
                "HEAD",
                "POST",
                "PUT",
                "DELETE",
                "TRACE",
                "CONNECT",
                "INVALID"
        };

void http_request_init(http_request **req)
{
    *req = malloc(sizeof(http_request));
    http_request *request = *req;
    request->method = 0;
    request->search_path = NULL;
    //初始化队列
    TAILQ_INIT(&request->metadata_head);
}

void http_parse_method(http_request* request, char* line)
{
    enum parser_states {
        METHOD, //方式
        URL,    //URL
        VERSION,//版本
        DONE
    };
    int s = METHOD;
    char * token;
    //strsep函数用于分解字符串为一组字符串
    while ((token = strsep(&line," \r\n") )!= NULL)
    {
        switch (s) {
            case METHOD: {
                int found = 0;
                for (int i = 0; i < http_methods_len; i++) {
                    if (strcmp(token, http_methods[i]) == 0) {
                        found = 1;
                        request->method = i;
                        logger_info("method %d \n",i);
                        break;
                    }
                }
                if (found == 0) {
                    request->method = http_methods_len - 1;
                    free(line);
                    return;
                }
                s++;
                break;
            }
            case URL: {
                request->search_path = strdup(token);
                logger_info("search_path %s \n",request->search_path);
                s++;
                break;
            }
            case VERSION: {
                if (strcmp(token, "HTTP/1.0") == 0) {
                    request->version = HTTP_VERSION_1_0;
                } else if (strcmp(token, "HTTP/1.1") == 0) {
                    request->version = HTTP_VERSION_1_1;
                } else {
                    request->version = HTTP_VERSION_INVALID;
                }
                s++;
                break;
            }
            case DONE:
                break;
        }
    }

}
void http_parse_metadata(http_request *result, char *line)
{
    char *line_copy = strdup(line);
    char  *key = strdup(strtok(line_copy,":"));
    char *value = strtok(NULL, "\r");
    char *p = value;
    while(*p == ' ') p++;
    value = strdup(p);
    free(line_copy);
    http_metadata_item *item = malloc(sizeof(*item));
    item->key = key;
    item->value = value;
    //尾部插入元素
    TAILQ_INSERT_TAIL(&result->metadata_head, item, entries);
}

//释放队列
void http_request_destroy(http_request *req)
{
    free((char*)req->search_path);

    struct http_metadata_item *item;
    TAILQ_FOREACH(item, &req->metadata_head, entries) {
        free((char*)item->key);
        free((char*)item->value);
        free(item);
    }
}

//构造请求
char *http_build_request(http_request *req)
{
    //构造http请求
    const  char  *search_path = req->search_path;
    int size = strlen("GET ") + 1;
    char *request_buffer = calloc(size, sizeof(char));
    strncat(request_buffer,"GET ",4);

    size +=strlen(search_path) +1;
    //重新分配缓冲区空间
    request_buffer = realloc(request_buffer,size);
    strncat(request_buffer, search_path, strlen(search_path));
    //验证版本
    switch(req->version)
    {
        case HTTP_VERSION_1_0:
            size += strlen(" HTTP/1.0\r\n\r\n");
            request_buffer = realloc(request_buffer, size);
            strncat(request_buffer, " HTTP/1.0\r\n", strlen(" HTTP/1.0\r\n"));
            break;
        case HTTP_VERSION_1_1:
            size += strlen(" HTTP/1.1\r\n\r\n");
            request_buffer = realloc(request_buffer, size);
            strncat(request_buffer, " HTTP/1.1\r\n", strlen(" HTTP/1.1\r\n"));
            break;
        default:
            logger_error("Failed to retrieve the http version\n");
            return NULL;
    }
    http_metadata_item *item;
    //遍历队列中
    TAILQ_FOREACH(item, &req->metadata_head, entries) {
        //删除头中的连接属性
        if(strcmp(item->key, "Connection") == 0 ||
           strcmp(item->key, "Proxy-Connection") == 0)
        {
            continue;
        }

        //连接总长度
        size += strlen(item->key) + strlen(": ") + strlen(item->value) + strlen("\r\n");
        request_buffer = realloc(request_buffer, size);
        strncat(request_buffer, item->key, strlen(item->key));
        strncat(request_buffer, ": ", 2);
        strncat(request_buffer, item->value, strlen(item->value));
        strncat(request_buffer, "\r\n", 2);
    }
    if(req->version == HTTP_VERSION_1_1)
    {
        ///加入拼接Connection: close
        size += strlen("Connection: close\r\n");
        request_buffer = realloc(request_buffer, size);
        strncat(request_buffer, "Connection: close\r\n", strlen("Connection: close\r\n"));
    }
    size += strlen("\r\n");
    request_buffer = realloc(request_buffer, size);
    strncat(request_buffer, "\r\n", 2);
    logger_info("request_buffer %s",request_buffer);
    return request_buffer;


}

