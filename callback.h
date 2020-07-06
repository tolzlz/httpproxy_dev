#ifndef HTTPPROXY_DEV_CALLBACK_H
#define HTTPPROXY_DEV_CALLBACK_H

#include <ev.h>
#include "proxy.h"

void accept_cb(int fd);
void client_recv_cb(int fd);
int http_request_send(int sockfd,http_request *req);
int send_to_client(int client_sockfd, char data[], int packages_size, ssize_t length);
char * getdata(int server_sockfd,ssize_t *length);


#endif
