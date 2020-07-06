#ifndef HTTPPROXY_DEV_PROXY_H
#define HTTPPROXY_DEV_PROXY_H

#include <sys/queue.h>
#define DEFAULT_BUFFER_SIZE 128          /*http proxy5缓冲区大小*/
enum http_methods_enum {
    OPTIONS,
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
    TRACE,
    CONNECT,
    UNKNOWN
};
enum http_versions_enum {
    HTTP_VERSION_1_0,
    HTTP_VERSION_1_1,
    HTTP_VERSION_INVALID
};

typedef struct http_request
{
    enum http_methods_enum method;
    enum http_versions_enum version;
    const char *search_path;

    //定义队列的头部
    TAILQ_HEAD(METADATA_HEAD, http_metadata_item) metadata_head;
} http_request;

typedef struct http_metadata_item
{
    const char *key;
    const char *value;
    //必须包含一个TAILQ_ENTRY来指向上一个和下一个元素
    TAILQ_ENTRY(http_metadata_item) entries;
} http_metadata_item;



#endif
