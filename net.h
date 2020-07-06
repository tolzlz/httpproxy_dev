
#ifndef HTTPPROXY_DEV_NET_H
#define HTTPPROXY_DEV_NET_H

#include "proxy.h"

http_request *http_read_header(int sockfd);
char *read_line(int sockfd);
int http_connect(http_request *req);

#endif
