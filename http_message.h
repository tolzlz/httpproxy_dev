

#ifndef HTTPPROXY_DEV_HTTP_MESSAGE_H
#define HTTPPROXY_DEV_HTTP_MESSAGE_H

#include "proxy.h"



void http_request_init(http_request**);
void http_parse_method(http_request*, char*);
void http_parse_metadata(http_request *result, char *line);
void http_request_destroy(http_request *req);
char *http_build_request(http_request *req);

#endif
